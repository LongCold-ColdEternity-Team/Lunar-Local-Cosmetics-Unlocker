#define UNICODE
#define _UNICODE
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>

#include <algorithm>
#include <cwctype>
#include <iostream>
#include <string>
#include <vector>

namespace {

std::wstring lower(std::wstring value) {
    std::transform(value.begin(), value.end(), value.begin(), [](wchar_t value) {
        return static_cast<wchar_t>(std::towlower(value));
    });
    return value;
}

bool containsInsensitive(const std::wstring& value, const std::wstring& needle) {
    return lower(value).find(lower(needle)) != std::wstring::npos;
}

std::wstring processPath(DWORD pid) {
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!process) return {};
    std::vector<wchar_t> buffer(32768);
    DWORD length = static_cast<DWORD>(buffer.size());
    std::wstring result;
    if (QueryFullProcessImageNameW(process, 0, buffer.data(), &length)) {
        result.assign(buffer.data(), length);
    }
    CloseHandle(process);
    return result;
}

std::wstring lunarWindowTitle(DWORD pid) {
    struct Context {
        DWORD pid;
        std::wstring title;
    } context{pid, {}};

    EnumWindows([](HWND window, LPARAM parameter) -> BOOL {
        auto* context = reinterpret_cast<Context*>(parameter);
        DWORD owner = 0;
        GetWindowThreadProcessId(window, &owner);
        if (owner != context->pid || !IsWindowVisible(window)) return TRUE;

        wchar_t title[512]{};
        GetWindowTextW(window, title, static_cast<int>(_countof(title)));
        const std::wstring value(title);
        if (containsInsensitive(value, L"Lunar Client") &&
            containsInsensitive(value, L"1.8.9")) {
            context->title = value;
            return FALSE;
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&context));
    return context.title;
}

bool isLunarJava(DWORD pid, std::wstring* pathOut = nullptr,
                 std::wstring* titleOut = nullptr) {
    const std::wstring path = processPath(pid);
    const std::wstring title = lunarWindowTitle(pid);
    const bool matches = containsInsensitive(path, L".lunarclient\\jre\\") &&
        containsInsensitive(path, L"\\bin\\java") && !title.empty();
    if (pathOut) *pathOut = path;
    if (titleOut) *titleOut = title;
    return matches;
}

DWORD findTarget() {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32W entry{sizeof(entry)};
    DWORD pid = 0;
    if (Process32FirstW(snapshot, &entry)) {
        do {
            if (_wcsicmp(entry.szExeFile, L"javaw.exe") != 0 &&
                _wcsicmp(entry.szExeFile, L"java.exe") != 0) continue;
            if (isLunarJava(entry.th32ProcessID)) {
                pid = entry.th32ProcessID;
                break;
            }
        } while (Process32NextW(snapshot, &entry));
    }
    CloseHandle(snapshot);
    return pid;
}

bool hasAgentLoaded(DWORD pid, const std::wstring& expectedPath) {
    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!process) return false;

    std::vector<HMODULE> modules(2048);
    DWORD required = 0;
    bool found = false;
    if (EnumProcessModulesEx(process, modules.data(),
                             static_cast<DWORD>(modules.size() * sizeof(HMODULE)),
                             &required, LIST_MODULES_ALL)) {
        const std::wstring expected = lower(
            expectedPath.substr(expectedPath.find_last_of(L"\\/") + 1));
        const DWORD count = std::min<DWORD>(
            required / sizeof(HMODULE), static_cast<DWORD>(modules.size()));
        for (DWORD i = 0; i < count; ++i) {
            wchar_t name[MAX_PATH]{};
            if (GetModuleBaseNameW(process, modules[i], name, MAX_PATH) == 0) continue;
            const std::wstring loaded = lower(name);
            if (loaded == expected ||
                (loaded.rfind(L"lunar_unlock_agent_", 0) == 0 &&
                 loaded.size() > 23 && loaded.ends_with(L".dll"))) {
                found = true;
                break;
            }
        }
    }
    CloseHandle(process);
    return found;
}

std::wstring formatError(DWORD error) {
    wchar_t* message = nullptr;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                       FORMAT_MESSAGE_IGNORE_INSERTS,
                   nullptr, error, 0, reinterpret_cast<wchar_t*>(&message), 0, nullptr);
    std::wstring result = message ? message : L"unknown error";
    if (message) LocalFree(message);
    while (!result.empty() && (result.back() == L'\r' || result.back() == L'\n' ||
                               result.back() == L' ')) {
        result.pop_back();
    }
    return result;
}

bool inject(DWORD pid, const std::wstring& dllPath, DWORD timeoutMs) {
    HANDLE process = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
                                     PROCESS_VM_OPERATION | PROCESS_VM_WRITE |
                                     PROCESS_VM_READ,
                                 FALSE, pid);
    if (!process) {
        std::wcerr << L"OpenProcess failed: " << formatError(GetLastError()) << L'\n';
        return false;
    }

    const SIZE_T pathBytes = (dllPath.size() + 1) * sizeof(wchar_t);
    void* remotePath = VirtualAllocEx(
        process, nullptr, pathBytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remotePath) {
        std::wcerr << L"VirtualAllocEx failed: " << formatError(GetLastError()) << L'\n';
        CloseHandle(process);
        return false;
    }

    SIZE_T written = 0;
    if (!WriteProcessMemory(process, remotePath, dllPath.c_str(), pathBytes, &written) ||
        written != pathBytes) {
        std::wcerr << L"WriteProcessMemory failed: " << formatError(GetLastError()) << L'\n';
        VirtualFreeEx(process, remotePath, 0, MEM_RELEASE);
        CloseHandle(process);
        return false;
    }

    auto loadLibrary = reinterpret_cast<LPTHREAD_START_ROUTINE>(
        GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW"));
    HANDLE thread = loadLibrary
        ? CreateRemoteThread(process, nullptr, 0, loadLibrary, remotePath, 0, nullptr)
        : nullptr;
    if (!thread) {
        std::wcerr << L"CreateRemoteThread failed: " << formatError(GetLastError()) << L'\n';
        VirtualFreeEx(process, remotePath, 0, MEM_RELEASE);
        CloseHandle(process);
        return false;
    }

    const DWORD wait = WaitForSingleObject(thread, timeoutMs);
    DWORD exitCode = 0;
    GetExitCodeThread(thread, &exitCode);
    CloseHandle(thread);
    VirtualFreeEx(process, remotePath, 0, MEM_RELEASE);
    CloseHandle(process);

    if (wait != WAIT_OBJECT_0 || exitCode == 0) {
        std::wcerr << (wait == WAIT_TIMEOUT ? L"LoadLibrary timed out.\n"
                                              : L"LoadLibrary returned null.\n");
        return false;
    }
    return true;
}

void printUsage() {
    std::wcout << L"LunarUnlockInjector.exe [--pid PID] [--dll PATH] [--timeout-ms N] [--force]\n";
}

}  // namespace

int wmain(int argc, wchar_t** argv) {
    DWORD pid = 0;
    DWORD timeoutMs = 15000;
    std::wstring dllPath;
    bool force = false;

    for (int i = 1; i < argc; ++i) {
        const std::wstring argument = argv[i];
        if (argument == L"--pid" && i + 1 < argc) {
            pid = static_cast<DWORD>(std::stoul(argv[++i]));
        } else if (argument == L"--dll" && i + 1 < argc) {
            dllPath = argv[++i];
        } else if (argument == L"--timeout-ms" && i + 1 < argc) {
            timeoutMs = static_cast<DWORD>(std::stoul(argv[++i]));
        } else if (argument == L"--force") {
            force = true;
        } else if (argument == L"--help" || argument == L"-h") {
            printUsage();
            return 0;
        } else {
            printUsage();
            return 2;
        }
    }

    if (dllPath.empty()) {
        wchar_t executable[32768]{};
        GetModuleFileNameW(nullptr, executable, static_cast<DWORD>(_countof(executable)));
        std::wstring directory(executable);
        directory.resize(directory.find_last_of(L"\\/") + 1);
        dllPath = directory + L"lunar_unlock_agent.dll";
    }
    wchar_t absolute[32768]{};
    if (GetFullPathNameW(dllPath.c_str(), static_cast<DWORD>(_countof(absolute)),
                         absolute, nullptr)) {
        dllPath = absolute;
    }
    if (GetFileAttributesW(dllPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
        std::wcerr << L"DLL not found: " << dllPath << L'\n';
        return 3;
    }

    if (!pid) pid = findTarget();
    if (!pid) {
        std::wcerr << L"No active Lunar Client 1.8.9 JVM found.\n";
        return 4;
    }

    std::wstring path;
    std::wstring title;
    if (!isLunarJava(pid, &path, &title)) {
        std::wcerr << L"PID is not an active Lunar Client 1.8.9 Java process.\n";
        return 5;
    }
    std::wcout << L"Target PID: " << pid << L"\nWindow: " << title
               << L"\nJava: " << path << L'\n';

    if (!force && hasAgentLoaded(pid, dllPath)) {
        std::wcout << L"Agent is already loaded.\n";
        return 0;
    }
    if (!inject(pid, dllPath, timeoutMs)) return 6;
    std::wcout << L"DLL loaded successfully. Inspect lunar_unlock_agent.log for verification.\n";
    return 0;
}
