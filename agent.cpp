#include <windows.h>
#include <jni.h>
#include <jvmti.h>

#include <atomic>
#include <climits>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace {

constexpr char kManagerSignature[] =
    "Lcom/moonsworth/lunar/client/OIOIRCOOICRCIIOIRHOIOORCIOHORI/"
    "RCIORCRRIROROHROCCOIIOHCHIICRC/HIIOHCCICCHRCHOROORICCRHIHOCOO;";
constexpr char kLoginResponseSignature[] =
    "Lcom/lunarclient/websocket/cosmetic/v2/LoginResponse;";
constexpr char kLoginResponseDescriptor[] =
    "Lcom/lunarclient/websocket/cosmetic/v2/LoginResponse;";
constexpr char kLoginResponseBuilderDescriptor[] =
    "Lcom/lunarclient/websocket/cosmetic/v2/LoginResponse$Builder;";
constexpr char kOwnedCosmeticDescriptor[] =
    "Lcom/moonsworth/lunar/client/ICORCHCORIOIIIRCIRRHIHORICOIHH/"
    "HRCORCCCHOCRCICCRHOHHICOIIICII/RCRROIORHICCOHOIIIRROHIORIIIHC;";
constexpr char kCatalogCosmeticDescriptor[] =
    "Lcom/moonsworth/lunar/client/ICORCHCORIOIIIRCIRRHIHORICOIHH/"
    "HRCORCCCHOCRCICCRHOHHICOIIICII/HRCORCCCHOCRCICCRHOHHICOIIICII;";
constexpr char kCosmeticTypeDescriptor[] =
    "Lcom/moonsworth/lunar/client/ICORCHCORIOIIIRCIRRHIHORICOIHH/"
    "HRCORCCCHOCRCICCRHOHHICOIIICII/RCRHHRIOICOORCIRCHOHHRHHIRRCRI;";
constexpr char kItemMaterialDescriptor[] =
    "Lcom/moonsworth/lunar/client/ORIHOIHRRIORROHIHHIIHOIHCCRHRI/"
    "RCRROIORHICCOHOIIIRROHIORIIIHC/RCRROIORHICCOHOIIIRROHIORIIIHC/"
    "HIIOHCCICCHRCHOROORICCRHIHOCOO;";
constexpr char kOwnedMetadataDescriptor[] =
    "Lcom/moonsworth/lunar/client/ICORCHCORIOIIIRCIRRHIHORICOIHH/"
    "HRCORCCCHOCRCICCRHOHHICOIIICII/CIOCOHHRIIRCIROCHHICICRHICOHIH;";
constexpr char kLocalCosmeticDescriptor[] =
    "Lcom/moonsworth/lunar/client/ICORCHCORIOIIIRCIRRHIHORICOIHH/"
    "HRCORCCCHOCRCICCRHOHHICOIIICII/COROIRIRHICROHCIOORRIOHCROCIHC;";
constexpr char kEquippedCosmeticsGetter[] =
    "IHCHCHIRHCHIRHRCHOICIRHHCROCOH";
constexpr char kEquippedCosmeticsSetter[] =
    "RRIORIHHCRRHCRCOCIRCOIIRICOCHH";
constexpr char kClientUtilitySignature[] =
    "Lcom/moonsworth/lunar/client/util/CHOCOICROCICIHCCRIIICHRCICORIH;";
constexpr char kClientBridgeGetter[] =
    "ROHRIICHRCORIHOICHICHIROORIIOI";
constexpr char kPlayerCosmeticsGetter[] =
    "RHCCHHCIHIRICHOICOHCCHCCICRCOO";
constexpr char kPlayerCosmeticsMutation[] =
    "HRCORCCCHOCRCICCRHOHHICOIIICII";
constexpr char kLoadoutPreviewId[] = "TEST_PLAYER_DUMMY";

constexpr char kEmoteManagerSignature[] =
    "Lcom/moonsworth/lunar/client/OIOIRCOOICRCIIOIRHOIOORCIOHORI/"
    "RCIORCRRIROROHROCCOIIOHCHIICRC/OORCHRCCHICCRHRORHIIOCIRHIIRII;";
constexpr char kJamManagerSignature[] =
    "Lcom/moonsworth/lunar/client/OIOIRCOOICRCIIOIRHOIOORCIOHORI/"
    "OORCHRCCHICCRHRORHIIOCIRHIIRII/HRCORCCCHOCRCICCRHOHHICOIIICII;";
constexpr char kSprayManagerSignature[] =
    "Lcom/moonsworth/lunar/client/OIOIRCOOICRCIIOIRHOIOORCIOHORI/"
    "RCIORCRRIROROHROCCOIIOHCHIICRC/IHICCORIHRHOHHHCHCICICCHIOIHCH;";
constexpr char kBadgeManagerSignature[] =
    "Lcom/moonsworth/lunar/client/OIOIRCOOICRCIIOIRHOIOORCIOHORI/"
    "RCIORCRRIROROHROCCOIIOHCHIICRC/CIOCOHHRIIRCIROCHHICICRHICOHIH;";
constexpr char kLunarPlusManagerSignature[] =
    "Lcom/moonsworth/lunar/client/OIOIRCOOICRCIIOIRHOIOORCIOHORI/"
    "RCIORCRRIROROHROCCOIIOHCHIICRC/OIOIRCOOICRCIIOIRHOIOORCIOHORI;";

constexpr char kEmoteOwnedWrapperSignature[] =
    "Lcom/moonsworth/lunar/client/ICORCHCORIOIIIRCIRRHIHORICOIHH/"
    "RCRHHRIOICOORCIRCHOHHRHHIRRCRI/RCRHHRIOICOORCIRCHOHHRHHIRRCRI;";
constexpr char kEmoteOwnedWrapperDescriptor[] =
    "Lcom/moonsworth/lunar/client/ICORCHCORIOIIIRCIRRHIHORICOIHH/"
    "RCRHHRIOICOORCIRCHOHHRHHIRRCRI/RCRHHRIOICOORCIRCHOHHRHHIRRCRI;";
constexpr char kEquippedEmoteWrapperSignature[] =
    "Lcom/moonsworth/lunar/client/RCIORCRRIROROHROCCOIIOHCHIICRC/"
    "IOHCCHIRRRIIIHORIROIORORIHIICO;";
constexpr char kEmoteMetadataDescriptor[] =
    "Lcom/moonsworth/lunar/client/ICORCHCORIOIIIRCIRRHIHORICOIHH/"
    "HRCORCCCHOCRCICCRHOHHICOIIICII/CIOCOHHRIIRCIROCHHICICRHICOHIH;";

constexpr char kOwnedJamSignature[] = "Lcom/lunarclient/websocket/jam/v1/OwnedJam;";
constexpr char kOwnedJamDescriptor[] = "Lcom/lunarclient/websocket/jam/v1/OwnedJam;";
constexpr char kOwnedJamBuilderDescriptor[] =
    "Lcom/lunarclient/websocket/jam/v1/OwnedJam$Builder;";

constexpr char kBadgeWrapperSignature[] =
    "Lcom/moonsworth/lunar/client/RCRHHRIOICOORCIRCHOHHRHHIRRCRI/"
    "HRCORCCCHOCRCICCRHOHHICOIIICII;";
constexpr char kBadgeWrapperDescriptor[] =
    "Lcom/moonsworth/lunar/client/RCRHHRIOICOORCIRCHOHHRHHIRRCRI/"
    "HRCORCCCHOCRCICCRHOHHICOIIICII;";
constexpr char kBadgeMetadataDescriptor[] =
    "Lcom/moonsworth/lunar/client/RCRHHRIOICOORCIRCHOHHRHHIRRCRI/"
    "RCRROIORHICCOHOIIIRROHIORIIIHC;";

constexpr char kColorSignature[] = "Lcom/lunarclient/common/v1/Color;";
constexpr char kColorDescriptor[] = "Lcom/lunarclient/common/v1/Color;";
constexpr char kColorBuilderDescriptor[] = "Lcom/lunarclient/common/v1/Color$Builder;";
constexpr char kEquippedSpraySignature[] = "Lcom/lunarclient/websocket/spray/v1/EquippedSpray;";
constexpr char kEquippedSprayDescriptor[] = "Lcom/lunarclient/websocket/spray/v1/EquippedSpray;";
constexpr char kEquippedSprayBuilderDescriptor[] =
    "Lcom/lunarclient/websocket/spray/v1/EquippedSpray$Builder;";

HMODULE g_module = nullptr;
std::filesystem::path g_logPath;
std::filesystem::path g_statusPath;
std::filesystem::path g_selectionPath;
std::mutex g_logMutex;
std::mutex g_resolutionLogMutex;
std::set<std::string> g_loggedResolutions;
std::atomic<unsigned long> g_tagSequence{1};

struct EmoteSelection {
    jint id = 0;
    jint slot = 0;
    jint jam = 0;
    bool layoutKnown = true;

    bool operator==(const EmoteSelection& other) const {
        return id == other.id && slot == other.slot && jam == other.jam &&
            layoutKnown == other.layoutKnown;
    }

    bool operator<(const EmoteSelection& other) const {
        if (slot != other.slot) return slot < other.slot;
        if (id != other.id) return id < other.id;
        if (jam != other.jam) return jam < other.jam;
        return layoutKnown < other.layoutKnown;
    }
};

struct SelectionState {
    std::set<jlong> cosmetics;
    std::set<EmoteSelection> emotes;
    std::map<jint, jint> sprays;
    std::optional<jint> lunarPlus;

    bool operator==(const SelectionState& other) const {
        return cosmetics == other.cosmetics && emotes == other.emotes &&
            sprays == other.sprays && lunarPlus == other.lunarPlus;
    }
};

SelectionState g_savedSelection;
bool g_hasSavedSelection = false;
jobject g_persistentCosmeticManager = nullptr;
jobject g_persistentEmoteManager = nullptr;
jclass g_persistentEquippedEmoteClass = nullptr;
jobject g_persistentSprayManager = nullptr;
jobject g_persistentLoginResponse = nullptr;
jobject g_persistentPlayerCosmeticState = nullptr;
jobject g_persistentLocalPlayerUuid = nullptr;
bool g_localCosmeticSelectionInitialized = false;
int g_playerStateWaitReason = 0;

void logLine(const std::string& line) {
    std::lock_guard<std::mutex> guard(g_logMutex);
    std::ofstream output(g_logPath, std::ios::app);
    if (output) output << line << '\n';
    OutputDebugStringA(("[lunar_unlock_agent] " + line + "\n").c_str());
}

void writeStatus(const std::string& status) {
    if (g_statusPath.empty()) return;
    const std::filesystem::path temporary = g_statusPath.wstring() + L".tmp";
    {
        std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
        if (!output) return;
        output << status << '\n';
        output.flush();
        if (!output.good()) return;
    }
    MoveFileExW(temporary.c_str(), g_statusPath.c_str(),
                MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
}

void initializeSelectionPath() {
    std::vector<wchar_t> buffer(32768);
    const DWORD length = GetEnvironmentVariableW(
        L"LOCALAPPDATA", buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length == 0 || length >= buffer.size()) {
        logLine("SELECTION_PATH_DISABLED reason=LOCALAPPDATA_UNAVAILABLE");
        return;
    }

    const std::filesystem::path directory = std::filesystem::path(buffer.data()) /
        L"ColdEternityTeam" / L"LunarLocalCosmetics";
    std::error_code error;
    std::filesystem::create_directories(directory, error);
    if (error) {
        logLine("SELECTION_PATH_FAILED error=" + error.message());
        return;
    }
    g_selectionPath = directory / L"selection-v1.txt";
    logLine("SELECTION_PATH_READY path=" + g_selectionPath.string());
}

bool loadSelection(SelectionState& selection) {
    selection = {};
    if (g_selectionPath.empty()) return false;
    std::ifstream input(g_selectionPath);
    if (!input) return false;

    std::string line;
    if (!std::getline(input, line)) return false;
    if (!line.empty() && line.back() == '\r') line.pop_back();
    if (line != "LUNAR_LOCAL_COSMETICS_V1") {
        logLine("SELECTION_LOAD_FAILED reason=UNSUPPORTED_FORMAT");
        return false;
    }

    size_t loaded = 0;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line.front() == '#') continue;
        const size_t separator = line.find('=');
        if (separator == std::string::npos || separator == 0 ||
            separator + 1 >= line.size()) continue;
        const std::string key = line.substr(0, separator);
        const std::string value = line.substr(separator + 1);
        try {
            if (key == "cosmetic") {
                size_t consumed = 0;
                const long long id = std::stoll(value, &consumed);
                if (consumed == value.size() && id >= 0) {
                    selection.cosmetics.insert(static_cast<jlong>(id));
                    ++loaded;
                }
            } else if (key == "emote") {
                const size_t firstColon = value.find(':');
                if (firstColon == std::string::npos) {
                    size_t consumed = 0;
                    const long id = std::stol(value, &consumed);
                    if (consumed == value.size() && id >= 0 && id <= INT_MAX) {
                        selection.emotes.insert({static_cast<jint>(id), 0, 0, false});
                        ++loaded;
                    }
                } else {
                    const size_t secondColon = value.find(':', firstColon + 1);
                    if (secondColon == std::string::npos || firstColon == 0 ||
                        secondColon == firstColon + 1 || secondColon + 1 >= value.size()) continue;
                    size_t idConsumed = 0;
                    size_t slotConsumed = 0;
                    size_t jamConsumed = 0;
                    const long id = std::stol(value.substr(0, firstColon), &idConsumed);
                    const long slot = std::stol(
                        value.substr(firstColon + 1, secondColon - firstColon - 1), &slotConsumed);
                    const long jam = std::stol(value.substr(secondColon + 1), &jamConsumed);
                    if (idConsumed == firstColon &&
                        slotConsumed == secondColon - firstColon - 1 &&
                        jamConsumed == value.size() - secondColon - 1 &&
                        id >= 0 && id <= INT_MAX && slot >= 0 && slot <= INT_MAX &&
                        jam >= 0 && jam <= INT_MAX) {
                        selection.emotes.insert({
                            static_cast<jint>(id), static_cast<jint>(slot),
                            static_cast<jint>(jam), true});
                        ++loaded;
                    }
                }
            } else if (key == "spray") {
                const size_t colon = value.find(':');
                if (colon == std::string::npos || colon == 0 || colon + 1 >= value.size()) continue;
                size_t slotConsumed = 0;
                size_t idConsumed = 0;
                const long slot = std::stol(value.substr(0, colon), &slotConsumed);
                const long id = std::stol(value.substr(colon + 1), &idConsumed);
                if (slotConsumed == colon && idConsumed == value.size() - colon - 1 &&
                    slot >= 0 && slot <= INT_MAX && id >= 0 && id <= INT_MAX) {
                    selection.sprays[static_cast<jint>(slot)] = static_cast<jint>(id);
                    ++loaded;
                }
            } else if (key == "lunarplus") {
                size_t consumed = 0;
                const long color = std::stol(value, &consumed, 0);
                if (consumed == value.size() && color >= 0 && color <= 0xFFFFFF) {
                    selection.lunarPlus = static_cast<jint>(color);
                    ++loaded;
                }
            }
        } catch (...) {
            continue;
        }
    }
    logLine("SELECTION_LOAD entries=" + std::to_string(loaded) +
            " cosmetics=" + std::to_string(selection.cosmetics.size()) +
            " emotes=" + std::to_string(selection.emotes.size()) +
            " sprays=" + std::to_string(selection.sprays.size()) +
            " lunarplus=" + std::to_string(selection.lunarPlus.has_value()));
    return true;
}

bool saveSelection(const SelectionState& selection) {
    if (g_selectionPath.empty()) return false;
    const std::filesystem::path temporary = g_selectionPath.wstring() + L".tmp";
    {
        std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
        if (!output) return false;
        output << "LUNAR_LOCAL_COSMETICS_V1\n";
        output << "# Local equipped state only; owned catalogs are not persisted.\n";
        for (const jlong id : selection.cosmetics) output << "cosmetic=" << id << '\n';
        for (const EmoteSelection& emote : selection.emotes) {
            output << "emote=" << emote.id << ':' << emote.slot << ':' << emote.jam << '\n';
        }
        for (const auto& item : selection.sprays) {
            output << "spray=" << item.first << ':' << item.second << '\n';
        }
        if (selection.lunarPlus.has_value()) {
            output << "lunarplus=0x" << std::hex << selection.lunarPlus.value() << std::dec << '\n';
        }
        output.flush();
        if (!output.good()) return false;
    }
    if (!MoveFileExW(temporary.c_str(), g_selectionPath.c_str(),
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        DeleteFileW(temporary.c_str());
        return false;
    }
    return true;
}

bool clearException(JNIEnv* env, const char* location);

void replaceGlobalRef(JNIEnv* env, jobject& target, jobject value) {
    if (target) {
        env->DeleteGlobalRef(target);
        target = nullptr;
    }
    if (value) target = env->NewGlobalRef(value);
    clearException(env, "global reference");
}

void replaceGlobalClassRef(JNIEnv* env, jclass& target, jclass value) {
    if (target) {
        env->DeleteGlobalRef(target);
        target = nullptr;
    }
    if (value) target = static_cast<jclass>(env->NewGlobalRef(value));
    clearException(env, "global class reference");
}

std::string jstringToUtf8(JNIEnv* env, jstring value) {
    if (!value) return {};
    const char* chars = env->GetStringUTFChars(value, nullptr);
    std::string result = chars ? chars : "";
    if (chars) env->ReleaseStringUTFChars(value, chars);
    return result;
}

bool clearException(JNIEnv* env, const char* location) {
    if (!env->ExceptionCheck()) return false;

    jthrowable throwable = env->ExceptionOccurred();
    env->ExceptionClear();
    std::string detail;
    if (throwable) {
        jclass throwableClass = env->GetObjectClass(throwable);
        jmethodID toString = throwableClass
            ? env->GetMethodID(throwableClass, "toString", "()Ljava/lang/String;")
            : nullptr;
        if (env->ExceptionCheck()) env->ExceptionClear();
        jstring text = toString
            ? static_cast<jstring>(env->CallObjectMethod(throwable, toString))
            : nullptr;
        if (!env->ExceptionCheck() && text) detail = jstringToUtf8(env, text);
        if (env->ExceptionCheck()) env->ExceptionClear();
        if (text) env->DeleteLocalRef(text);
        if (throwableClass) env->DeleteLocalRef(throwableClass);
        env->DeleteLocalRef(throwable);
    }

    logLine(std::string("JNI_EXCEPTION location=") + location +
            (detail.empty() ? "" : " detail=" + detail));
    return true;
}

// Obfuscated client builds occasionally expose a method through a descriptor
// that differs from the one reported by the running JVM.  Reflection matches
// the public method by name and arity, then lets the JVM perform boxing for
// primitive return values.
jobject invokeNoArgReflective(JNIEnv* env, jobject target, const char* wantedName,
                              const char* location) {
    if (!target || !wantedName) return nullptr;

    jclass classClass = env->FindClass("java/lang/Class");
    jclass methodClass = env->FindClass("java/lang/reflect/Method");
    jclass arrayClass = env->FindClass("java/lang/reflect/Array");
    if (clearException(env, "reflect helper classes") || !classClass ||
        !methodClass || !arrayClass) {
        if (classClass) env->DeleteLocalRef(classClass);
        if (methodClass) env->DeleteLocalRef(methodClass);
        if (arrayClass) env->DeleteLocalRef(arrayClass);
        return nullptr;
    }

    jmethodID getMethods = env->GetMethodID(
        classClass, "getMethods", "()[Ljava/lang/reflect/Method;");
    jmethodID methodName = env->GetMethodID(
        methodClass, "getName", "()Ljava/lang/String;");
    jmethodID getParameterTypes = env->GetMethodID(
        methodClass, "getParameterTypes", "()[Ljava/lang/Class;");
    jmethodID invoke = env->GetMethodID(
        methodClass, "invoke", "(Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");
    if (clearException(env, "reflect helper methods") || !getMethods ||
        !methodName || !getParameterTypes || !invoke) {
        env->DeleteLocalRef(classClass);
        env->DeleteLocalRef(methodClass);
        env->DeleteLocalRef(arrayClass);
        return nullptr;
    }

    jclass targetClass = env->GetObjectClass(target);
    jobject methods = targetClass
        ? env->CallObjectMethod(targetClass, getMethods) : nullptr;
    if (clearException(env, location) || !methods) {
        if (methods) env->DeleteLocalRef(methods);
        if (targetClass) env->DeleteLocalRef(targetClass);
        env->DeleteLocalRef(classClass);
        env->DeleteLocalRef(methodClass);
        env->DeleteLocalRef(arrayClass);
        return nullptr;
    }

    jobject result = nullptr;
    const jsize methodCount = env->GetArrayLength(static_cast<jarray>(methods));
    for (jsize i = 0; i < methodCount && !result; ++i) {
        if (env->PushLocalFrame(12) != JNI_OK) break;
        jobject method = env->GetObjectArrayElement(
            static_cast<jobjectArray>(methods), i);
        jstring name = method
            ? static_cast<jstring>(env->CallObjectMethod(method, methodName)) : nullptr;
        jobject parameterTypes = method
            ? env->CallObjectMethod(method, getParameterTypes) : nullptr;
        const std::string nameUtf8 = jstringToUtf8(env, name);
        const jsize parameterCount = parameterTypes
            ? env->GetArrayLength(static_cast<jarray>(parameterTypes)) : -1;
        jobject frameResult = nullptr;
        if (!clearException(env, location) && nameUtf8 == wantedName &&
            parameterCount == 0) {
            jobject value = env->CallObjectMethod(method, invoke, target, nullptr);
            if (!clearException(env, location) && value) {
                frameResult = value;
            }
        }
        result = env->PopLocalFrame(frameResult);
    }

    env->DeleteLocalRef(methods);
    env->DeleteLocalRef(targetClass);
    env->DeleteLocalRef(classClass);
    env->DeleteLocalRef(methodClass);
    env->DeleteLocalRef(arrayClass);
    return result;
}

jobject invokeStaticNoArgReflective(JNIEnv* env, jclass targetClass,
                                    const char* wantedName,
                                    const char* location) {
    if (!targetClass || !wantedName) return nullptr;

    jclass classClass = env->FindClass("java/lang/Class");
    jclass methodClass = env->FindClass("java/lang/reflect/Method");
    if (clearException(env, "static reflect helper classes") ||
        !classClass || !methodClass) {
        if (classClass) env->DeleteLocalRef(classClass);
        if (methodClass) env->DeleteLocalRef(methodClass);
        return nullptr;
    }

    jmethodID getMethods = env->GetMethodID(
        classClass, "getMethods", "()[Ljava/lang/reflect/Method;");
    jmethodID methodName = env->GetMethodID(
        methodClass, "getName", "()Ljava/lang/String;");
    jmethodID getParameterTypes = env->GetMethodID(
        methodClass, "getParameterTypes", "()[Ljava/lang/Class;");
    jmethodID invoke = env->GetMethodID(
        methodClass, "invoke", "(Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");
    if (clearException(env, "static reflect helper methods") ||
        !getMethods || !methodName || !getParameterTypes || !invoke) {
        env->DeleteLocalRef(classClass);
        env->DeleteLocalRef(methodClass);
        return nullptr;
    }

    jobject methods = env->CallObjectMethod(targetClass, getMethods);
    if (clearException(env, location) || !methods) {
        if (methods) env->DeleteLocalRef(methods);
        env->DeleteLocalRef(classClass);
        env->DeleteLocalRef(methodClass);
        return nullptr;
    }

    jobject result = nullptr;
    const jsize methodCount = env->GetArrayLength(static_cast<jarray>(methods));
    for (jsize i = 0; i < methodCount && !result; ++i) {
        if (env->PushLocalFrame(12) != JNI_OK) break;
        jobject method = env->GetObjectArrayElement(
            static_cast<jobjectArray>(methods), i);
        jstring name = method
            ? static_cast<jstring>(env->CallObjectMethod(method, methodName)) : nullptr;
        jobject parameterTypes = method
            ? env->CallObjectMethod(method, getParameterTypes) : nullptr;
        const std::string nameUtf8 = jstringToUtf8(env, name);
        const jsize parameterCount = parameterTypes
            ? env->GetArrayLength(static_cast<jarray>(parameterTypes)) : -1;
        jobject frameResult = nullptr;
        if (!clearException(env, location) && nameUtf8 == wantedName &&
            parameterCount == 0) {
            jobject value = env->CallObjectMethod(
                method, invoke, nullptr, nullptr);
            if (!clearException(env, location) && value) frameResult = value;
        }
        result = env->PopLocalFrame(frameResult);
    }

    env->DeleteLocalRef(methods);
    env->DeleteLocalRef(classClass);
    env->DeleteLocalRef(methodClass);
    return result;
}

std::optional<jlong> invokeLongNoArgReflective(JNIEnv* env, jobject target,
                                                const char* wantedName,
                                                const char* location) {
    jobject boxed = invokeNoArgReflective(env, target, wantedName, location);
    if (!boxed) return std::nullopt;
    jclass longClass = env->FindClass("java/lang/Long");
    jmethodID longValue = longClass
        ? env->GetMethodID(longClass, "longValue", "()J") : nullptr;
    const jlong value = (longClass && longValue)
        ? env->CallLongMethod(boxed, longValue) : 0;
    const bool failed = clearException(env, location) || !longClass || !longValue;
    if (longClass) env->DeleteLocalRef(longClass);
    env->DeleteLocalRef(boxed);
    return failed ? std::nullopt : std::optional<jlong>(value);
}

std::string jvmtiErrorName(jvmtiEnv* jvmti, jvmtiError error) {
    char* name = nullptr;
    if (jvmti && jvmti->GetErrorName(error, &name) == JVMTI_ERROR_NONE && name) {
        std::string result(name);
        jvmti->Deallocate(reinterpret_cast<unsigned char*>(name));
        return result;
    }
    return std::to_string(static_cast<int>(error));
}

void logResolutionOnce(const std::string& kind, const std::string& name,
                       const std::string& descriptor) {
    const std::string key = kind + "\n" + name + "\n" + descriptor;
    {
        std::lock_guard<std::mutex> guard(g_resolutionLogMutex);
        if (!g_loggedResolutions.insert(key).second) return;
    }
    logLine("RUNTIME_MEMBER_RESOLVED kind=" + kind + " name=" + name +
            " descriptor=" + descriptor);
}

jmethodID tryMethodByName(JNIEnv* env, jclass klass, const char* name,
                          const char* descriptor, bool wantStatic) {
    if (!klass || !name || !descriptor) return nullptr;
    jmethodID method = wantStatic
        ? env->GetStaticMethodID(klass, name, descriptor)
        : env->GetMethodID(klass, name, descriptor);
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        return nullptr;
    }
    return method;
}

jmethodID resolveMethod(JNIEnv* env, jvmtiEnv* jvmti, jclass klass,
                        const char* preferredName, const std::string& descriptor,
                        bool wantStatic, const char* kind,
                        jmethodID excluded = nullptr) {
    if (!klass || !jvmti) return nullptr;
    if (preferredName) {
        jmethodID preferred = tryMethodByName(
            env, klass, preferredName, descriptor.c_str(), wantStatic);
        if (preferred && preferred != excluded) return preferred;
    }

    jclass current = static_cast<jclass>(env->NewLocalRef(klass));
    while (current) {
        jint count = 0;
        jmethodID* methods = nullptr;
        if (jvmti->GetClassMethods(current, &count, &methods) == JVMTI_ERROR_NONE) {
            for (jint i = 0; i < count; ++i) {
                char* name = nullptr;
                char* signature = nullptr;
                char* generic = nullptr;
                jint modifiers = 0;
                const bool named = jvmti->GetMethodName(
                    methods[i], &name, &signature, &generic) == JVMTI_ERROR_NONE;
                const bool modified = jvmti->GetMethodModifiers(
                    methods[i], &modifiers) == JVMTI_ERROR_NONE;
                const bool isStatic = modified && (modifiers & 0x0008) != 0;
                const bool match = named && signature &&
                    descriptor == signature && isStatic == wantStatic &&
                    methods[i] != excluded && name && std::string(name) != "<init>" &&
                    std::string(name) != "<clinit>";
                if (match) {
                    const jmethodID result = methods[i];
                    logResolutionOnce(kind ? kind : "method", name, descriptor);
                    if (name) jvmti->Deallocate(reinterpret_cast<unsigned char*>(name));
                    if (signature) {
                        jvmti->Deallocate(reinterpret_cast<unsigned char*>(signature));
                    }
                    if (generic) {
                        jvmti->Deallocate(reinterpret_cast<unsigned char*>(generic));
                    }
                    if (methods) {
                        jvmti->Deallocate(reinterpret_cast<unsigned char*>(methods));
                    }
                    env->DeleteLocalRef(current);
                    return result;
                }
                if (name) jvmti->Deallocate(reinterpret_cast<unsigned char*>(name));
                if (signature) {
                    jvmti->Deallocate(reinterpret_cast<unsigned char*>(signature));
                }
                if (generic) {
                    jvmti->Deallocate(reinterpret_cast<unsigned char*>(generic));
                }
            }
        }
        if (methods) jvmti->Deallocate(reinterpret_cast<unsigned char*>(methods));
        jclass parent = env->GetSuperclass(current);
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
            parent = nullptr;
        }
        env->DeleteLocalRef(current);
        current = parent;
    }
    return nullptr;
}

jfieldID tryFieldByName(JNIEnv* env, jclass klass, const char* name,
                        const char* descriptor, bool wantStatic) {
    if (!klass || !name || !descriptor) return nullptr;
    jfieldID field = wantStatic
        ? env->GetStaticFieldID(klass, name, descriptor)
        : env->GetFieldID(klass, name, descriptor);
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        return nullptr;
    }
    return field;
}

jfieldID resolveField(JNIEnv* env, jvmtiEnv* jvmti, jclass klass,
                      const char* preferredName, const std::string& descriptor,
                      bool wantStatic, const char* kind,
                      std::initializer_list<const char*> genericNeedles = {}) {
    if (!klass || !jvmti) return nullptr;
    if (preferredName) {
        jfieldID preferred = tryFieldByName(
            env, klass, preferredName, descriptor.c_str(), wantStatic);
        if (preferred) return preferred;
    }

    jclass current = static_cast<jclass>(env->NewLocalRef(klass));
    while (current) {
        jint count = 0;
        jfieldID* fields = nullptr;
        if (jvmti->GetClassFields(current, &count, &fields) == JVMTI_ERROR_NONE) {
            for (jint i = 0; i < count; ++i) {
                char* name = nullptr;
                char* signature = nullptr;
                char* generic = nullptr;
                jint modifiers = 0;
                const bool named = jvmti->GetFieldName(
                    current, fields[i], &name, &signature, &generic) == JVMTI_ERROR_NONE;
                const bool modified = jvmti->GetFieldModifiers(
                    current, fields[i], &modifiers) == JVMTI_ERROR_NONE;
                const bool isStatic = modified && (modifiers & 0x0008) != 0;
                bool genericMatch = true;
                for (const char* needle : genericNeedles) {
                    if (!needle || !*needle) continue;
                    if (!generic || std::string(generic).find(needle) == std::string::npos) {
                        genericMatch = false;
                        break;
                    }
                }
                const bool match = named && signature && descriptor == signature &&
                    isStatic == wantStatic && genericMatch;
                if (match) {
                    const jfieldID result = fields[i];
                    logResolutionOnce(kind ? kind : "field", name ? name : "?", descriptor);
                    if (name) jvmti->Deallocate(reinterpret_cast<unsigned char*>(name));
                    if (signature) {
                        jvmti->Deallocate(reinterpret_cast<unsigned char*>(signature));
                    }
                    if (generic) {
                        jvmti->Deallocate(reinterpret_cast<unsigned char*>(generic));
                    }
                    if (fields) {
                        jvmti->Deallocate(reinterpret_cast<unsigned char*>(fields));
                    }
                    env->DeleteLocalRef(current);
                    return result;
                }
                if (name) jvmti->Deallocate(reinterpret_cast<unsigned char*>(name));
                if (signature) {
                    jvmti->Deallocate(reinterpret_cast<unsigned char*>(signature));
                }
                if (generic) {
                    jvmti->Deallocate(reinterpret_cast<unsigned char*>(generic));
                }
            }
        }
        if (fields) jvmti->Deallocate(reinterpret_cast<unsigned char*>(fields));
        jclass parent = env->GetSuperclass(current);
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
            parent = nullptr;
        }
        env->DeleteLocalRef(current);
        current = parent;
    }
    return nullptr;
}

jclass findLoadedClass(JNIEnv* env, jvmtiEnv* jvmti, const char* wantedSignature) {
    jint count = 0;
    jclass* classes = nullptr;
    const jvmtiError error = jvmti->GetLoadedClasses(&count, &classes);
    if (error != JVMTI_ERROR_NONE) {
        logLine("CLASS_SCAN_FAILED error=" + jvmtiErrorName(jvmti, error));
        return nullptr;
    }

    jclass result = nullptr;
    for (jint i = 0; i < count; ++i) {
        char* signature = nullptr;
        char* generic = nullptr;
        if (jvmti->GetClassSignature(classes[i], &signature, &generic) == JVMTI_ERROR_NONE) {
            if (!result && signature && std::string(signature) == wantedSignature) {
                result = static_cast<jclass>(env->NewLocalRef(classes[i]));
            }
            if (signature) jvmti->Deallocate(reinterpret_cast<unsigned char*>(signature));
            if (generic) jvmti->Deallocate(reinterpret_cast<unsigned char*>(generic));
        }
        env->DeleteLocalRef(classes[i]);
    }
    jvmti->Deallocate(reinterpret_cast<unsigned char*>(classes));
    return result;
}

struct HeapProbe {
    jlong marker = 0;
};

jvmtiIterationControl JNICALL tagInstance(jlong, jlong, jlong* tag, void* userData) {
    if (tag) *tag = static_cast<HeapProbe*>(userData)->marker;
    return JVMTI_ITERATION_CONTINUE;
}

std::vector<jobject> findInstances(JNIEnv* env, jvmtiEnv* jvmti, jclass klass) {
    std::vector<jobject> result;
    if (!klass) return result;

    HeapProbe probe;
    probe.marker = 0x4c554e4100000000LL |
        (static_cast<jlong>(GetCurrentProcessId() & 0xffff) << 16) |
        static_cast<jlong>(g_tagSequence.fetch_add(1) & 0xffff);

    const jvmtiError iterate = jvmti->IterateOverInstancesOfClass(
        klass, JVMTI_HEAP_OBJECT_EITHER, tagInstance, &probe);
    if (iterate != JVMTI_ERROR_NONE) {
        logLine("HEAP_SCAN_FAILED error=" + jvmtiErrorName(jvmti, iterate));
        return result;
    }

    jint count = 0;
    jobject* objects = nullptr;
    jlong* tags = nullptr;
    const jvmtiError get = jvmti->GetObjectsWithTags(
        1, &probe.marker, &count, &objects, &tags);
    if (get != JVMTI_ERROR_NONE) {
        logLine("HEAP_FETCH_FAILED error=" + jvmtiErrorName(jvmti, get));
        return result;
    }

    result.reserve(count);
    for (jint i = 0; i < count; ++i) {
        result.push_back(objects[i]);
        jvmti->SetTag(objects[i], 0);
    }
    if (objects) jvmti->Deallocate(reinterpret_cast<unsigned char*>(objects));
    if (tags) jvmti->Deallocate(reinterpret_cast<unsigned char*>(tags));
    return result;
}

jint collectionSize(JNIEnv* env, jobject collection) {
    if (!collection) return -1;
    jclass klass = env->GetObjectClass(collection);
    jmethodID size = klass ? env->GetMethodID(klass, "size", "()I") : nullptr;
    if (clearException(env, "collection.size lookup") || !size) {
        if (klass) env->DeleteLocalRef(klass);
        return -1;
    }
    const jint value = env->CallIntMethod(collection, size);
    const bool failed = clearException(env, "collection.size call");
    env->DeleteLocalRef(klass);
    return failed ? -1 : value;
}

struct ManagerState {
    jobject manager = nullptr;
    jint catalog = -1;
    jint owned = -1;
    jint ownedBySerial = -1;
};

struct ManagerCollections {
    jobject catalog = nullptr;
    jobject owned = nullptr;
    jobject ownedBySerial = nullptr;
};

struct CachedObjectMember {
    bool resolved = false;
    jmethodID method = nullptr;
    jfieldID field = nullptr;
};

jobject readObjectMember(JNIEnv* env, jvmtiEnv* jvmti, jobject object,
                         jclass klass, const char* preferredMethod,
                         const std::string& methodDescriptor,
                         const char* preferredField,
                         const std::string& fieldDescriptor,
                         const char* kind,
                         std::initializer_list<const char*> genericNeedles,
                         CachedObjectMember& cached) {
    if (!cached.resolved) {
        cached.method = preferredMethod
            ? tryMethodByName(env, klass, preferredMethod,
                              methodDescriptor.c_str(), false)
            : nullptr;
        if (!cached.method) {
            cached.field = resolveField(
                env, jvmti, klass, preferredField, fieldDescriptor, false, kind,
                genericNeedles);
        }
        cached.resolved = cached.method || cached.field;
    }

    if (cached.method) {
        jobject value = env->CallObjectMethod(object, cached.method);
        if (!env->ExceptionCheck() && value) return value;
        if (env->ExceptionCheck()) env->ExceptionClear();
        if (value) env->DeleteLocalRef(value);
    }

    if (!cached.field) return nullptr;
    jobject value = env->GetObjectField(object, cached.field);
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        if (value) env->DeleteLocalRef(value);
        return nullptr;
    }
    return value;
}

ManagerCollections resolveManagerCollections(JNIEnv* env, jvmtiEnv* jvmti,
                                             jobject manager) {
    static CachedObjectMember catalogMember;
    static CachedObjectMember ownedMember;
    static CachedObjectMember serialMember;
    ManagerCollections collections;
    if (!manager || !jvmti) return collections;
    jclass managerClass = env->GetObjectClass(manager);
    if (!managerClass || env->ExceptionCheck()) {
        if (env->ExceptionCheck()) env->ExceptionClear();
        return collections;
    }

    collections.catalog = readObjectMember(
        env, jvmti, manager, managerClass,
        "HCOCCIOOROHIHHRHCCHHRROIOOOHRO", "()Ljava/util/Map;",
        nullptr, "Ljava/util/Map;", "cosmetic_catalog_field",
        {kCatalogCosmeticDescriptor}, catalogMember);
    collections.owned = readObjectMember(
        env, jvmti, manager, managerClass,
        "IOOCHIHIHRRIRORHHRHOIORORCIHCR", "()Ljava/util/Set;",
        nullptr, "Ljava/util/Set;", "cosmetic_owned_field", {"TT;"},
        ownedMember);
    collections.ownedBySerial = readObjectMember(
        env, jvmti, manager, managerClass,
        "OCORRRRRHRHIIORIRCIORIIOOHRORR", "()Ljava/util/Map;",
        nullptr, "Ljava/util/Map;", "cosmetic_serial_field",
        {"Ljava/lang/Long;", kOwnedCosmeticDescriptor}, serialMember);

    env->DeleteLocalRef(managerClass);
    return collections;
}

void releaseManagerCollections(JNIEnv* env, ManagerCollections& collections) {
    if (collections.catalog) env->DeleteLocalRef(collections.catalog);
    if (collections.owned) env->DeleteLocalRef(collections.owned);
    if (collections.ownedBySerial) env->DeleteLocalRef(collections.ownedBySerial);
    collections = {};
}

ManagerState inspectManager(JNIEnv* env, jvmtiEnv* jvmti, jobject manager) {
    ManagerState state;
    state.manager = manager;
    ManagerCollections collections = resolveManagerCollections(env, jvmti, manager);
    state.catalog = collectionSize(env, collections.catalog);
    state.owned = collectionSize(env, collections.owned);
    state.ownedBySerial = collectionSize(env, collections.ownedBySerial);
    releaseManagerCollections(env, collections);
    return state;
}

// The client can recreate its cosmetic manager after a profile refresh or
// when the Locker screen is opened. Keep the persistent reference attached to
// the live manager instead of polling an object that has gone stale.
bool refreshPersistentCosmeticManager(JNIEnv* env, jvmtiEnv* jvmti) {
    if (!jvmti) return false;
    jclass managerClass = findLoadedClass(env, jvmti, kManagerSignature);
    if (!managerClass) return false;
    std::vector<jobject> managers = findInstances(env, jvmti, managerClass);
    ManagerState selected;
    for (jobject manager : managers) {
        ManagerState current = inspectManager(env, jvmti, manager);
        if (current.catalog > selected.catalog) selected = current;
    }

    bool rebound = false;
    if (selected.manager && selected.catalog > 0 &&
        (!g_persistentCosmeticManager ||
         env->IsSameObject(g_persistentCosmeticManager, selected.manager) != JNI_TRUE)) {
        replaceGlobalRef(env, g_persistentCosmeticManager, selected.manager);
        rebound = g_persistentCosmeticManager != nullptr;
        if (rebound) {
            logLine("COSMETIC_MANAGER_REBOUND catalog=" +
                    std::to_string(selected.catalog) +
                    " owned=" + std::to_string(selected.owned));
        }
    }
    for (jobject manager : managers) env->DeleteLocalRef(manager);
    env->DeleteLocalRef(managerClass);
    return rebound;
}

struct ResponseCandidate {
    jobject object = nullptr;
    jint owned = 0;
    jint outfits = 0;
    jboolean hasOutfitTree = JNI_FALSE;
    jboolean hasAll = JNI_FALSE;
    int score = -1;
};

ResponseCandidate selectResponse(JNIEnv* env, jclass responseClass,
                                 const std::vector<jobject>& responses) {
    ResponseCandidate best;
    jmethodID getOwned = env->GetMethodID(responseClass, "getOwnedCosmeticsCount", "()I");
    jmethodID getOutfits = env->GetMethodID(responseClass, "getOutfitsCount", "()I");
    jmethodID hasTree = env->GetMethodID(responseClass, "hasOutfitTree", "()Z");
    jmethodID getAll = env->GetMethodID(responseClass, "getHasAllCosmeticsFlag", "()Z");
    if (clearException(env, "LoginResponse inspectors") ||
        !getOwned || !getOutfits || !hasTree || !getAll) return best;

    for (jobject response : responses) {
        const jint owned = env->CallIntMethod(response, getOwned);
        const jint outfits = env->CallIntMethod(response, getOutfits);
        const jboolean tree = env->CallBooleanMethod(response, hasTree);
        const jboolean all = env->CallBooleanMethod(response, getAll);
        if (clearException(env, "LoginResponse inspect")) continue;

        const int score = (tree == JNI_TRUE ? 100000 : 0) +
            static_cast<int>(outfits) * 100 + static_cast<int>(owned);
        if (score > best.score) {
            best = {response, owned, outfits, tree, all, score};
        }
    }
    return best;
}

bool applyUnlock(JNIEnv* env, jvmtiEnv* jvmti, jobject manager,
                 jclass responseClass, jobject response) {
    jmethodID toBuilder = env->GetMethodID(
        responseClass, "toBuilder", (std::string("()") + kLoginResponseBuilderDescriptor).c_str());
    if (clearException(env, "LoginResponse.toBuilder lookup") || !toBuilder) return false;

    jobject builder = env->CallObjectMethod(response, toBuilder);
    if (clearException(env, "LoginResponse.toBuilder call") || !builder) return false;

    jclass builderClass = env->GetObjectClass(builder);
    const std::string setDescriptor =
        std::string("(Z)") + kLoginResponseBuilderDescriptor;
    jmethodID setAll = builderClass
        ? env->GetMethodID(builderClass, "setHasAllCosmeticsFlag", setDescriptor.c_str())
        : nullptr;
    jmethodID build = builderClass
        ? env->GetMethodID(builderClass, "build", (std::string("()") + kLoginResponseDescriptor).c_str())
        : nullptr;
    if (clearException(env, "LoginResponse.Builder methods") || !setAll || !build) {
        if (builderClass) env->DeleteLocalRef(builderClass);
        env->DeleteLocalRef(builder);
        return false;
    }

    jobject chained = env->CallObjectMethod(builder, setAll, JNI_TRUE);
    jobject patched = env->CallObjectMethod(builder, build);
    if (chained) env->DeleteLocalRef(chained);
    if (clearException(env, "LoginResponse build") || !patched) {
        if (patched) env->DeleteLocalRef(patched);
        env->DeleteLocalRef(builderClass);
        env->DeleteLocalRef(builder);
        return false;
    }

    jclass managerClass = env->GetObjectClass(manager);
    const std::string handlerDescriptor =
        std::string("(") + kLoginResponseDescriptor + ")V";
    jmethodID handler = managerClass
        ? resolveMethod(env, jvmti, managerClass,
                        "HRCORCCCHOCRCICCRHOHHICOIIICII",
                        handlerDescriptor, false, "cosmetic_login_handler")
        : nullptr;
    if (clearException(env, "manager login handler") || !handler) {
        if (managerClass) env->DeleteLocalRef(managerClass);
        env->DeleteLocalRef(patched);
        env->DeleteLocalRef(builderClass);
        env->DeleteLocalRef(builder);
        return false;
    }

    env->CallVoidMethod(manager, handler, patched);
    const bool failed = clearException(env, "manager login handler call");
    env->DeleteLocalRef(managerClass);
    env->DeleteLocalRef(patched);
    env->DeleteLocalRef(builderClass);
    env->DeleteLocalRef(builder);
    return !failed;
}

bool populateOwnedFromCatalog(JNIEnv* env, jvmtiEnv* jvmti, jobject manager) {
    jclass managerClass = env->GetObjectClass(manager);
    if (!managerClass) {
        clearException(env, "direct manager class");
        return false;
    }

    const std::string factoryDescriptor =
        std::string("(ILjava/lang/String;") + kCosmeticTypeDescriptor +
        "Ljava/lang/String;ZJLjava/time/Instant;Ljava/util/List;Ljava/util/List;Z" +
        kItemMaterialDescriptor + kOwnedMetadataDescriptor + ")" + kOwnedCosmeticDescriptor;
    jmethodID factory = resolveMethod(
        env, jvmti, managerClass, "RCRROIORHICCOHOIIIRROHIORIIIHC",
        factoryDescriptor, false, "cosmetic_owned_factory");
    if (clearException(env, "direct manager methods") || !factory) {
        env->DeleteLocalRef(managerClass);
        return false;
    }

    ManagerCollections managerCollections = resolveManagerCollections(env, jvmti, manager);
    jobject catalog = managerCollections.catalog;
    jobject targetOwned = managerCollections.owned;
    jobject targetSerials = managerCollections.ownedBySerial;
    if (clearException(env, "direct manager collections") ||
        !catalog || !targetOwned || !targetSerials) {
        releaseManagerCollections(env, managerCollections);
        env->DeleteLocalRef(managerClass);
        return false;
    }

    jclass mapClass = env->FindClass("java/util/Map");
    jclass collectionClass = env->FindClass("java/util/Collection");
    jclass iteratorClass = env->FindClass("java/util/Iterator");
    jclass optionalClass = env->FindClass("java/util/Optional");
    jclass hashSetClass = env->FindClass("java/util/HashSet");
    jclass hashMapClass = env->FindClass("java/util/HashMap");
    jclass collectionsClass = env->FindClass("java/util/Collections");
    jclass longClass = env->FindClass("java/lang/Long");
    if (clearException(env, "direct Java collection classes") ||
        !mapClass || !collectionClass || !iteratorClass || !optionalClass ||
        !hashSetClass || !hashMapClass || !collectionsClass || !longClass) {
        env->DeleteLocalRef(catalog);
        env->DeleteLocalRef(targetOwned);
        env->DeleteLocalRef(targetSerials);
        env->DeleteLocalRef(managerClass);
        return false;
    }

    jmethodID mapValues = env->GetMethodID(mapClass, "values", "()Ljava/util/Collection;");
    jmethodID mapClear = env->GetMethodID(mapClass, "clear", "()V");
    jmethodID mapPutAll = env->GetMethodID(mapClass, "putAll", "(Ljava/util/Map;)V");
    jmethodID mapPut = env->GetMethodID(
        mapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    jmethodID collectionIterator = env->GetMethodID(
        collectionClass, "iterator", "()Ljava/util/Iterator;");
    jmethodID collectionClear = env->GetMethodID(collectionClass, "clear", "()V");
    jmethodID collectionAdd = env->GetMethodID(
        collectionClass, "add", "(Ljava/lang/Object;)Z");
    jmethodID collectionAddAll = env->GetMethodID(
        collectionClass, "addAll", "(Ljava/util/Collection;)Z");
    jmethodID iteratorHasNext = env->GetMethodID(iteratorClass, "hasNext", "()Z");
    jmethodID iteratorNext = env->GetMethodID(
        iteratorClass, "next", "()Ljava/lang/Object;");
    jmethodID optionalIsPresent = env->GetMethodID(optionalClass, "isPresent", "()Z");
    jmethodID optionalGet = env->GetMethodID(optionalClass, "get", "()Ljava/lang/Object;");
    jmethodID hashSetCtor = env->GetMethodID(hashSetClass, "<init>", "()V");
    jmethodID hashMapCtor = env->GetMethodID(hashMapClass, "<init>", "()V");
    jmethodID emptyListMethod = env->GetStaticMethodID(
        collectionsClass, "emptyList", "()Ljava/util/List;");
    jmethodID longValueOf = env->GetStaticMethodID(
        longClass, "valueOf", "(J)Ljava/lang/Long;");
    if (clearException(env, "direct collection methods") ||
        !mapValues || !mapClear || !mapPutAll || !mapPut || !collectionIterator ||
        !collectionClear || !collectionAdd || !collectionAddAll || !iteratorHasNext ||
        !iteratorNext || !optionalIsPresent || !optionalGet || !hashSetCtor ||
        !hashMapCtor || !emptyListMethod || !longValueOf) {
        env->DeleteLocalRef(catalog);
        env->DeleteLocalRef(targetOwned);
        env->DeleteLocalRef(targetSerials);
        env->DeleteLocalRef(managerClass);
        return false;
    }

    jobject stagedOwned = env->NewObject(hashSetClass, hashSetCtor);
    jobject stagedSerials = env->NewObject(hashMapClass, hashMapCtor);
    jobject emptyList = env->CallStaticObjectMethod(collectionsClass, emptyListMethod);
    jobject values = env->CallObjectMethod(catalog, mapValues);
    jobject iterator = values ? env->CallObjectMethod(values, collectionIterator) : nullptr;
    if (clearException(env, "direct staging setup") ||
        !stagedOwned || !stagedSerials || !emptyList || !values || !iterator) {
        if (iterator) env->DeleteLocalRef(iterator);
        if (values) env->DeleteLocalRef(values);
        if (stagedOwned) env->DeleteLocalRef(stagedOwned);
        if (stagedSerials) env->DeleteLocalRef(stagedSerials);
        env->DeleteLocalRef(catalog);
        env->DeleteLocalRef(targetOwned);
        env->DeleteLocalRef(targetSerials);
        env->DeleteLocalRef(managerClass);
        return false;
    }

    jint built = 0;
    jint skipped = 0;
    bool failed = false;
    jclass catalogClass = nullptr;
    jmethodID getId = nullptr;
    jmethodID getName = nullptr;
    jmethodID getType = nullptr;
    jmethodID getResource = nullptr;
    jfieldID resourceField = nullptr;
    jmethodID getHidden = nullptr;
    jmethodID getTags = nullptr;
    jmethodID getColors = nullptr;
    jmethodID getAnimated = nullptr;
    jmethodID getMaterial = nullptr;

    while (env->CallBooleanMethod(iterator, iteratorHasNext) == JNI_TRUE) {
        if (clearException(env, "catalog iterator hasNext")) {
            failed = true;
            break;
        }
        if (env->PushLocalFrame(48) != JNI_OK) {
            failed = true;
            break;
        }
        jobject item = env->CallObjectMethod(iterator, iteratorNext);
        if (clearException(env, "catalog iterator next") || !item) {
            env->PopLocalFrame(nullptr);
            failed = true;
            break;
        }

        if (!catalogClass) {
            catalogClass = static_cast<jclass>(env->NewGlobalRef(env->GetObjectClass(item)));
            getId = resolveMethod(env, jvmti, catalogClass, "getId", "()I", false,
                                  "catalog_id_getter");
            getName = resolveMethod(env, jvmti, catalogClass, "getName",
                                    "()Ljava/lang/String;", false,
                                    "catalog_name_getter");
            getType = resolveMethod(env, jvmti, catalogClass,
                                    "IHCIHHCIOROIHRIORCIRHRHICHRORO",
                                    "()Ljava/util/Optional;", false,
                                    "catalog_type_getter");
            getResource = tryMethodByName(
                env, catalogClass, "COOOCICROIRHRCHCCHHHRHCRHOHHOI",
                "()Ljava/lang/String;", false);
            if (!getResource) {
                resourceField = tryFieldByName(
                    env, catalogClass, "resource", "Ljava/lang/String;", false);
                if (resourceField) {
                    logResolutionOnce("catalog_resource_field", "resource",
                                      "Ljava/lang/String;");
                }
            }
            getHidden = tryMethodByName(env, catalogClass,
                                        "CIOROHCHHHHICORRHRIOCCCRCRHRIR", "()Z", false);
            getColors = tryMethodByName(
                env, catalogClass, "getColors", "()Ljava/util/List;", false);
            getTags = tryMethodByName(env, catalogClass,
                                      "RRHCHCCIRHRRROHOROHIRHRHIOHCHH",
                                      "()Ljava/util/List;", false);
            getAnimated = tryMethodByName(env, catalogClass,
                                          "OCROCHROOCCOCOHOICHIHHORIIHROR",
                                          "()Z", false);
            const std::string materialGetter = std::string("()") + kItemMaterialDescriptor;
            getMaterial = resolveMethod(env, jvmti, catalogClass,
                                        "COOOHHHHHOOCCIIOOICHIOIHOOHRIR",
                                        materialGetter, false,
                                        "catalog_material_getter");
            if (clearException(env, "catalog item methods") ||
                !getId || !getName || !getType ||
                (!getResource && !resourceField) || !getMaterial) {
                env->PopLocalFrame(nullptr);
                failed = true;
                break;
            }
        }

        jobject typeOptional = env->CallObjectMethod(item, getType);
        const jboolean hasType = typeOptional
            ? env->CallBooleanMethod(typeOptional, optionalIsPresent)
            : JNI_FALSE;
        if (clearException(env, "catalog type optional")) {
            env->PopLocalFrame(nullptr);
            failed = true;
            break;
        }
        if (hasType != JNI_TRUE) {
            ++skipped;
            env->PopLocalFrame(nullptr);
            continue;
        }

        const jint id = env->CallIntMethod(item, getId);
        jobject name = env->CallObjectMethod(item, getName);
        jobject type = env->CallObjectMethod(typeOptional, optionalGet);
        jobject resource = getResource
            ? env->CallObjectMethod(item, getResource)
            : env->GetObjectField(item, resourceField);
        const jboolean hidden = getHidden
            ? env->CallBooleanMethod(item, getHidden) : JNI_FALSE;
        jobject tags = getTags
            ? env->CallObjectMethod(item, getTags) : env->NewLocalRef(emptyList);
        jobject colors = getColors
            ? env->CallObjectMethod(item, getColors) : env->NewLocalRef(emptyList);
        const jboolean animated = getAnimated
            ? env->CallBooleanMethod(item, getAnimated) : JNI_FALSE;
        jobject material = env->CallObjectMethod(item, getMaterial);
        if (clearException(env, "catalog item values")) {
            env->PopLocalFrame(nullptr);
            failed = true;
            break;
        }

        jobject owned = env->CallObjectMethod(
            manager, factory, id, name, type, resource, hidden, static_cast<jlong>(-1),
            nullptr, tags, colors, animated, material, nullptr);
        if (clearException(env, "owned cosmetic factory") || !owned) {
            env->PopLocalFrame(nullptr);
            failed = true;
            break;
        }

        // Lunar indexes this collection by the cosmetic id (the first factory
        // argument). Owned objects also expose an expiry long, so selecting a
        // ()J method by shape can collapse every entry onto the shared -1 key.
        jobject boxedSerial = env->CallStaticObjectMethod(
            longClass, longValueOf, static_cast<jlong>(id));
        env->CallBooleanMethod(stagedOwned, collectionAdd, owned);
        jobject previous = env->CallObjectMethod(stagedSerials, mapPut, boxedSerial, owned);
        if (previous) env->DeleteLocalRef(previous);
        if (clearException(env, "owned staging add")) {
            env->PopLocalFrame(nullptr);
            failed = true;
            break;
        }
        ++built;
        env->PopLocalFrame(nullptr);
    }
    if (env->ExceptionCheck()) {
        clearException(env, "catalog iterator completion");
        failed = true;
    }

    const jint catalogCount = collectionSize(env, catalog);
    const jint stagedOwnedCount = collectionSize(env, stagedOwned);
    const jint stagedSerialCount = collectionSize(env, stagedSerials);
    const bool stagingValid = !failed && built > 0 && stagedOwnedCount == built &&
        stagedSerialCount == built && built >= (catalogCount * 3) / 4;
    logLine("DIRECT_STAGE catalog=" + std::to_string(catalogCount) +
            " built=" + std::to_string(built) +
            " skipped=" + std::to_string(skipped) +
            " set=" + std::to_string(stagedOwnedCount) +
            " map=" + std::to_string(stagedSerialCount) +
            " valid=" + std::to_string(stagingValid));

    if (stagingValid) {
        env->MonitorEnter(manager);
        env->CallVoidMethod(targetOwned, collectionClear);
        env->CallBooleanMethod(targetOwned, collectionAddAll, stagedOwned);
        env->CallVoidMethod(targetSerials, mapClear);
        env->CallVoidMethod(targetSerials, mapPutAll, stagedSerials);
        env->MonitorExit(manager);
        if (clearException(env, "owned collection commit")) failed = true;
    }

    if (catalogClass) env->DeleteGlobalRef(catalogClass);
    env->DeleteLocalRef(iterator);
    env->DeleteLocalRef(values);
    env->DeleteLocalRef(stagedOwned);
    env->DeleteLocalRef(stagedSerials);
    env->DeleteLocalRef(emptyList);
    env->DeleteLocalRef(catalog);
    env->DeleteLocalRef(targetOwned);
    env->DeleteLocalRef(targetSerials);
    env->DeleteLocalRef(mapClass);
    env->DeleteLocalRef(collectionClass);
    env->DeleteLocalRef(iteratorClass);
    env->DeleteLocalRef(optionalClass);
    env->DeleteLocalRef(hashSetClass);
    env->DeleteLocalRef(hashMapClass);
    env->DeleteLocalRef(collectionsClass);
    env->DeleteLocalRef(longClass);
    env->DeleteLocalRef(managerClass);
    return stagingValid && !failed;
}

jobject firstInstance(JNIEnv* env, jvmtiEnv* jvmti, jclass klass) {
    std::vector<jobject> instances = findInstances(env, jvmti, klass);
    return instances.empty() ? nullptr : instances.front();
}

bool bindLocalPlayerUuid(JNIEnv* env, jvmtiEnv* jvmti, bool force = false) {
    if (g_persistentLocalPlayerUuid && !force) return true;

    jclass utilityClass = findLoadedClass(env, jvmti, kClientUtilitySignature);
    if (!utilityClass) return false;
    jobject bridge = invokeStaticNoArgReflective(
        env, utilityClass, kClientBridgeGetter, "local player bridge");
    jobject session = bridge
        ? invokeNoArgReflective(env, bridge, "bridge$getSession", "local player session")
        : nullptr;
    jobject profile = session
        ? invokeNoArgReflective(env, session, "bridge$getProfile", "local player profile")
        : nullptr;
    jobject uuid = profile
        ? invokeNoArgReflective(env, profile, "getId", "local player UUID")
        : nullptr;

    bool changed = false;
    if (uuid) {
        if (!g_persistentLocalPlayerUuid) {
            changed = true;
        } else {
            jclass uuidClass = env->GetObjectClass(uuid);
            jmethodID equals = uuidClass
                ? env->GetMethodID(uuidClass, "equals", "(Ljava/lang/Object;)Z") : nullptr;
            const jboolean same = equals
                ? env->CallBooleanMethod(uuid, equals, g_persistentLocalPlayerUuid)
                : JNI_FALSE;
            if (clearException(env, "local player UUID compare") || same != JNI_TRUE) {
                changed = true;
            }
            if (uuidClass) env->DeleteLocalRef(uuidClass);
        }
        if (changed) {
            replaceGlobalRef(env, g_persistentLocalPlayerUuid, uuid);
            jobject text = invokeNoArgReflective(
                env, uuid, "toString", "local player UUID text");
            logLine("LOCAL_PLAYER_UUID_BOUND value=" +
                    jstringToUtf8(env, static_cast<jstring>(text)));
            if (text) env->DeleteLocalRef(text);
            g_localCosmeticSelectionInitialized = false;
        }
    }

    if (uuid) env->DeleteLocalRef(uuid);
    if (profile) env->DeleteLocalRef(profile);
    if (session) env->DeleteLocalRef(session);
    if (bridge) env->DeleteLocalRef(bridge);
    env->DeleteLocalRef(utilityClass);
    return g_persistentLocalPlayerUuid != nullptr;
}


bool bindPlayerCosmeticState(JNIEnv* env, jvmtiEnv* jvmti, bool force = false) {
    if (g_persistentPlayerCosmeticState && !force) return true;

    jobject previousState = g_persistentPlayerCosmeticState;
    constexpr char kPlayerStateSignature[] =
        "Lcom/moonsworth/lunar/client/OORRRCCHICOCHRIIHHCCHIOHHRICCO/"
        "HRCORCCCHOCRCICCRHOHHICOIIICII/RCIORCRRIROROHROCCOIIOHCHIICRC/"
        "HRCORCCCHOCRCICCRHOHHICOIIICII/RCRROIORHICCOHOIIIRROHIORIIIHC;";
    jclass stateClass = findLoadedClass(env, jvmti, kPlayerStateSignature);
    if (!stateClass) {
        if (g_playerStateWaitReason != 1) {
            logLine("PLAYER_COSMETIC_STATE_WAITING reason=CLASS_NOT_LOADED");
            g_playerStateWaitReason = 1;
        }
        return false;
    }
    std::vector<jobject> states = findInstances(env, jvmti, stateClass);
    if (states.empty()) {
        if (g_playerStateWaitReason != 2) {
            logLine("PLAYER_COSMETIC_STATE_WAITING reason=INSTANCE_NOT_FOUND");
            g_playerStateWaitReason = 2;
        }
        env->DeleteLocalRef(stateClass);
        return false;
    }

    jmethodID getId = env->GetMethodID(
        stateClass, "getId", "()Ljava/lang/String;");
    jmethodID isInitialized = env->GetMethodID(
        stateClass, "isInitialized", "()Z");
    if (clearException(env, "loadout preview identity methods") ||
        !getId || !isInitialized) {
        for (jobject state : states) env->DeleteLocalRef(state);
        env->DeleteLocalRef(stateClass);
        return false;
    }

    jobject selected = nullptr;
    jint selectedSize = -1;
    size_t matchingIds = 0;
    for (jobject state : states) {
        jstring id = static_cast<jstring>(env->CallObjectMethod(state, getId));
        const jboolean initialized = env->CallBooleanMethod(state, isInitialized);
        if (clearException(env, "loadout preview identity")) {
            if (id) env->DeleteLocalRef(id);
            continue;
        }
        const bool idMatches = jstringToUtf8(env, id) == kLoadoutPreviewId;
        if (id) env->DeleteLocalRef(id);
        if (!idMatches) continue;
        ++matchingIds;
        if (initialized != JNI_TRUE) continue;

        jobject cosmetics = invokeNoArgReflective(
            env, state, "RIROHRRHOHOCRHRRICIHIIROCCHHIH",
            "loadout preview bind list");
        selected = state;
        selectedSize = collectionSize(env, cosmetics);
        if (cosmetics) env->DeleteLocalRef(cosmetics);
    }

    if (!selected) {
        if (g_playerStateWaitReason != 3) {
            logLine("LOADOUT_PREVIEW_WAITING reason=ACTIVE_MODEL_NOT_FOUND" +
                    std::string(" candidates=") + std::to_string(states.size()) +
                    " matching_ids=" + std::to_string(matchingIds));
            g_playerStateWaitReason = 3;
        }
        for (jobject state : states) env->DeleteLocalRef(state);
        env->DeleteLocalRef(stateClass);
        return false;
    }

    const bool instanceChanged = !previousState ||
        !env->IsSameObject(previousState, selected);

    replaceGlobalRef(env, g_persistentPlayerCosmeticState, selected);
    g_playerStateWaitReason = 0;

    if (instanceChanged) {
        logLine("LOADOUT_PREVIEW_BOUND id=" + std::string(kLoadoutPreviewId) +
                " size=" + std::to_string(selectedSize) +
                " candidates=" + std::to_string(states.size()) +
                " matching_ids=" + std::to_string(matchingIds));
    }

    for (jobject state : states) env->DeleteLocalRef(state);
    env->DeleteLocalRef(stateClass);
    return force ? instanceChanged : true;
}

bool captureCosmeticListSelection(JNIEnv* env, jobject equipped,
                                  std::set<jlong>& selection,
                                  const char* location) {
    selection.clear();
    if (!equipped) return false;
    jclass collectionClass = env->FindClass("java/util/Collection");
    jclass iteratorClass = env->FindClass("java/util/Iterator");
    jmethodID iteratorMethod = collectionClass
        ? env->GetMethodID(collectionClass, "iterator", "()Ljava/util/Iterator;") : nullptr;
    jmethodID hasNext = iteratorClass
        ? env->GetMethodID(iteratorClass, "hasNext", "()Z") : nullptr;
    jmethodID next = iteratorClass
        ? env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;") : nullptr;
    if (clearException(env, location) || !collectionClass || !iteratorClass ||
        !iteratorMethod || !hasNext || !next) {
        if (collectionClass) env->DeleteLocalRef(collectionClass);
        if (iteratorClass) env->DeleteLocalRef(iteratorClass);
        return false;
    }
    jobject cursor = env->CallObjectMethod(equipped, iteratorMethod);
    bool failed = clearException(env, location) || !cursor;
    while (!failed && env->CallBooleanMethod(cursor, hasNext) == JNI_TRUE) {
        if (env->PushLocalFrame(16) != JNI_OK) {
            failed = true;
            break;
        }
        jobject wrapper = env->CallObjectMethod(cursor, next);
        jobject owned = wrapper
            ? invokeNoArgReflective(env, wrapper, "HCOIRRCCICHHCOOIIIRHHIRCRCHCHR",
                                    location) : nullptr;
        const auto id = invokeLongNoArgReflective(
            env, owned ? owned : wrapper,
            "HRHCIHHIIIRCICOICIICHIHCIRRHII", location);
        if (clearException(env, location)) {
            failed = true;
        } else if (id.has_value() && id.value() >= 0) {
            selection.insert(id.value());
        }
        env->PopLocalFrame(nullptr);
    }
    if (!failed && clearException(env, location)) failed = true;
    if (cursor) env->DeleteLocalRef(cursor);
    env->DeleteLocalRef(collectionClass);
    env->DeleteLocalRef(iteratorClass);
    return !failed;
}

bool capturePlayerCosmeticSelection(JNIEnv* env, jobject state,
                                    std::set<jlong>& selection) {
    if (!state) return false;
    jobject equipped = invokeNoArgReflective(
        env, state, "RIROHRRHOHOCRHRRICIHIIROCCHHIH", "preview selection list");
    const bool captured = captureCosmeticListSelection(
        env, equipped, selection, "preview selection capture");
    if (equipped) env->DeleteLocalRef(equipped);
    return captured;
}

bool captureLocalPlayerCosmeticSelection(JNIEnv* env, jobject manager,
                                         std::set<jlong>& selection) {
    selection.clear();
    if (!manager || !g_persistentLocalPlayerUuid) return false;
    jclass managerClass = env->GetObjectClass(manager);
    jmethodID getPlayerCosmetics = managerClass ? env->GetMethodID(
        managerClass, kPlayerCosmeticsGetter,
        "(Ljava/util/UUID;)Ljava/util/List;") : nullptr;
    jobject equipped = getPlayerCosmetics
        ? env->CallObjectMethod(manager, getPlayerCosmetics,
                                g_persistentLocalPlayerUuid)
        : nullptr;
    if (clearException(env, "local player cosmetic list") ||
        !managerClass || !getPlayerCosmetics || !equipped) {
        if (equipped) env->DeleteLocalRef(equipped);
        if (managerClass) env->DeleteLocalRef(managerClass);
        return false;
    }
    const bool captured = captureCosmeticListSelection(
        env, equipped, selection, "local player selection capture");
    env->DeleteLocalRef(equipped);
    env->DeleteLocalRef(managerClass);
    return captured;
}

bool restorePlayerCosmeticSelection(JNIEnv* env, jobject manager, jobject state,
                                    const std::set<jlong>& selection) {
    if (!manager || !state) return false;
    jobject equipped = invokeNoArgReflective(
        env, state, "RIROHRRHOHOCRHRRICIHIIROCCHHIH", "player restore list");
    jclass collectionClass = env->FindClass("java/util/Collection");
    jclass managerClass = env->GetObjectClass(manager);
    jmethodID clear = collectionClass
        ? env->GetMethodID(collectionClass, "clear", "()V") : nullptr;
    jmethodID add = collectionClass
        ? env->GetMethodID(collectionClass, "add", "(Ljava/lang/Object;)Z") : nullptr;
    jmethodID factory = managerClass
        ? env->GetMethodID(managerClass, "HCIICHICRRHHOHHICRRCCOHHHROHCI",
                           (std::string("(J)") + kLocalCosmeticDescriptor).c_str()) : nullptr;
    if (clearException(env, "player restore roots") || !equipped || !collectionClass ||
        !managerClass || !clear || !add || !factory) {
        if (equipped) env->DeleteLocalRef(equipped);
        if (collectionClass) env->DeleteLocalRef(collectionClass);
        if (managerClass) env->DeleteLocalRef(managerClass);
        return false;
    }
    env->CallVoidMethod(equipped, clear);
    if (clearException(env, "player restore clear")) {
        env->DeleteLocalRef(equipped);
        env->DeleteLocalRef(collectionClass);
        env->DeleteLocalRef(managerClass);
        return false;
    }
    size_t matched = 0;
    for (const jlong id : selection) {
        jobject wrapper = env->CallObjectMethod(manager, factory, id);
        if (clearException(env, "player restore factory")) continue;
        if (wrapper) {
            env->CallBooleanMethod(equipped, add, wrapper);
            if (!clearException(env, "player restore add")) ++matched;
            env->DeleteLocalRef(wrapper);
        }
    }
    // Rebuild the manager's derived render JSON after replacing the list.
    jmethodID rebuild = env->GetMethodID(
        managerClass, "OHORROHOIHICOICRROCHOOCCIOCOHI", "()V");
    if (rebuild) env->CallVoidMethod(manager, rebuild);
    const bool failed = clearException(env, "player restore rebuild");
    logLine("PLAYER_COSMETIC_SELECTION_RESTORE requested=" + std::to_string(selection.size()) +
            " matched=" + std::to_string(matched) +
            " valid=" + std::to_string(!failed && matched == selection.size()));
    env->DeleteLocalRef(equipped);
    env->DeleteLocalRef(collectionClass);
    env->DeleteLocalRef(managerClass);
    return !failed && matched == selection.size();
}

bool restoreLocalPlayerCosmeticSelection(JNIEnv* env, jobject manager,
                                         const std::set<jlong>& selection) {
    if (!manager || !g_persistentLocalPlayerUuid) return false;

    std::set<jlong> current;
    if (!captureLocalPlayerCosmeticSelection(env, manager, current)) return false;

    jclass managerClass = env->GetObjectClass(manager);
    jclass arrayListClass = env->FindClass("java/util/ArrayList");
    jclass collectionClass = env->FindClass("java/util/Collection");
    jclass integerClass = env->FindClass("java/lang/Integer");
    jclass hashMapClass = env->FindClass("java/util/HashMap");
    jmethodID listConstructor = arrayListClass
        ? env->GetMethodID(arrayListClass, "<init>", "()V") : nullptr;
    jmethodID mapConstructor = hashMapClass
        ? env->GetMethodID(hashMapClass, "<init>", "()V") : nullptr;
    jmethodID addToList = collectionClass
        ? env->GetMethodID(collectionClass, "add", "(Ljava/lang/Object;)Z") : nullptr;
    jmethodID valueOf = integerClass
        ? env->GetStaticMethodID(integerClass, "valueOf", "(I)Ljava/lang/Integer;") : nullptr;
    jmethodID addCosmetics = managerClass ? env->GetMethodID(
        managerClass, kPlayerCosmeticsMutation,
        "(Ljava/util/UUID;Ljava/util/List;Ljava/util/Map;)V") : nullptr;
    jmethodID removeCosmetics = managerClass ? env->GetMethodID(
        managerClass, kPlayerCosmeticsMutation,
        "(Ljava/util/UUID;Ljava/util/List;)V") : nullptr;
    if (clearException(env, "local player restore roots") || !managerClass ||
        !arrayListClass || !collectionClass || !integerClass || !hashMapClass ||
        !listConstructor || !mapConstructor || !addToList || !valueOf ||
        !addCosmetics || !removeCosmetics) {
        if (managerClass) env->DeleteLocalRef(managerClass);
        if (arrayListClass) env->DeleteLocalRef(arrayListClass);
        if (collectionClass) env->DeleteLocalRef(collectionClass);
        if (integerClass) env->DeleteLocalRef(integerClass);
        if (hashMapClass) env->DeleteLocalRef(hashMapClass);
        return false;
    }

    jobject removeIds = env->NewObject(arrayListClass, listConstructor);
    jobject addIds = env->NewObject(arrayListClass, listConstructor);
    jobject metadata = env->NewObject(hashMapClass, mapConstructor);
    bool failed = clearException(env, "local player restore staging") ||
        !removeIds || !addIds || !metadata;

    auto appendIds = [&](jobject target, const std::set<jlong>& ids) {
        for (const jlong id : ids) {
            if (id < 0 || id > INT_MAX) {
                failed = true;
                continue;
            }
            jobject boxed = env->CallStaticObjectMethod(
                integerClass, valueOf, static_cast<jint>(id));
            if (!boxed || clearException(env, "local player restore ID")) {
                failed = true;
            } else {
                env->CallBooleanMethod(target, addToList, boxed);
                if (clearException(env, "local player restore list add")) failed = true;
            }
            if (boxed) env->DeleteLocalRef(boxed);
        }
    };
    if (!failed) {
        appendIds(removeIds, current);
        appendIds(addIds, selection);
    }

    if (!failed && !current.empty()) {
        env->CallVoidMethod(manager, removeCosmetics,
                            g_persistentLocalPlayerUuid, removeIds);
        if (clearException(env, "local player remove cosmetics")) failed = true;
    }
    if (!failed && !selection.empty()) {
        env->CallVoidMethod(manager, addCosmetics,
                            g_persistentLocalPlayerUuid, addIds, metadata);
        if (clearException(env, "local player add cosmetics")) failed = true;
    }

    std::set<jlong> verified;
    const bool captured = !failed &&
        captureLocalPlayerCosmeticSelection(env, manager, verified);
    const bool valid = captured && verified == selection;
    logLine("LOCAL_PLAYER_SELECTION_RESTORE requested=" +
            std::to_string(selection.size()) +
            " before=" + std::to_string(current.size()) +
            " verified=" + std::to_string(verified.size()) +
            " valid=" + std::to_string(valid));

    if (removeIds) env->DeleteLocalRef(removeIds);
    if (addIds) env->DeleteLocalRef(addIds);
    if (metadata) env->DeleteLocalRef(metadata);
    env->DeleteLocalRef(managerClass);
    env->DeleteLocalRef(arrayListClass);
    env->DeleteLocalRef(collectionClass);
    env->DeleteLocalRef(integerClass);
    env->DeleteLocalRef(hashMapClass);
    return valid;
}

bool captureManagerCosmeticSelection(JNIEnv* env, jobject manager,
                                     std::set<jlong>& selection) {
    selection.clear();
    if (!manager) return false;
    jclass managerClass = env->GetObjectClass(manager);
    jmethodID getEquipped = managerClass ? env->GetMethodID(
        managerClass, kEquippedCosmeticsGetter, "()Ljava/util/List;") : nullptr;
    jobject equipped = getEquipped ? env->CallObjectMethod(manager, getEquipped) : nullptr;
    jclass collectionClass = env->FindClass("java/util/Collection");
    jclass iteratorClass = env->FindClass("java/util/Iterator");
    jclass integerClass = env->FindClass("java/lang/Integer");
    jmethodID iteratorMethod = collectionClass
        ? env->GetMethodID(collectionClass, "iterator", "()Ljava/util/Iterator;") : nullptr;
    jmethodID hasNext = iteratorClass ? env->GetMethodID(iteratorClass, "hasNext", "()Z") : nullptr;
    jmethodID next = iteratorClass
        ? env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;") : nullptr;
    jmethodID intValue = integerClass
        ? env->GetMethodID(integerClass, "intValue", "()I") : nullptr;
    if (clearException(env, "manager cosmetic selection roots") || !managerClass ||
        !equipped || !collectionClass || !iteratorClass || !integerClass ||
        !iteratorMethod || !hasNext || !next || !intValue) {
        if (managerClass) env->DeleteLocalRef(managerClass);
        if (equipped) env->DeleteLocalRef(equipped);
        if (collectionClass) env->DeleteLocalRef(collectionClass);
        if (iteratorClass) env->DeleteLocalRef(iteratorClass);
        if (integerClass) env->DeleteLocalRef(integerClass);
        return false;
    }

    jobject cursor = env->CallObjectMethod(equipped, iteratorMethod);
    bool failed = clearException(env, "manager cosmetic selection iterator") || !cursor;
    while (!failed && env->CallBooleanMethod(cursor, hasNext) == JNI_TRUE) {
        if (env->PushLocalFrame(8) != JNI_OK) {
            failed = true;
            break;
        }
        jobject boxedId = env->CallObjectMethod(cursor, next);
        const jint cosmeticId = boxedId
            ? env->CallIntMethod(boxedId, intValue) : static_cast<jint>(-1);
        if (clearException(env, "manager cosmetic selection item")) {
            failed = true;
        } else if (boxedId && cosmeticId >= 0) {
            selection.insert(static_cast<jlong>(cosmeticId));
        }
        env->PopLocalFrame(nullptr);
    }
    if (!failed && clearException(env, "manager cosmetic selection completion")) failed = true;
    if (cursor) env->DeleteLocalRef(cursor);
    env->DeleteLocalRef(equipped);
    env->DeleteLocalRef(collectionClass);
    env->DeleteLocalRef(iteratorClass);
    env->DeleteLocalRef(integerClass);
    env->DeleteLocalRef(managerClass);
    return !failed;
}

bool restoreManagerCosmeticSelection(JNIEnv* env, jobject manager,
                                     const std::set<jlong>& selection) {
    if (!manager) return false;
    jclass managerClass = env->GetObjectClass(manager);
    jclass arrayListClass = env->FindClass("java/util/ArrayList");
    jclass collectionClass = env->FindClass("java/util/Collection");
    jclass integerClass = env->FindClass("java/lang/Integer");
    jmethodID constructor = arrayListClass
        ? env->GetMethodID(arrayListClass, "<init>", "()V") : nullptr;
    jmethodID add = collectionClass
        ? env->GetMethodID(collectionClass, "add", "(Ljava/lang/Object;)Z") : nullptr;
    jmethodID valueOf = integerClass
        ? env->GetStaticMethodID(integerClass, "valueOf", "(I)Ljava/lang/Integer;") : nullptr;
    jmethodID setEquipped = managerClass ? env->GetMethodID(
        managerClass, kEquippedCosmeticsSetter, "(Ljava/util/List;)V") : nullptr;
    if (clearException(env, "manager cosmetic restore roots") || !managerClass ||
        !arrayListClass || !collectionClass || !integerClass || !constructor ||
        !add || !valueOf || !setEquipped) {
        if (managerClass) env->DeleteLocalRef(managerClass);
        if (arrayListClass) env->DeleteLocalRef(arrayListClass);
        if (collectionClass) env->DeleteLocalRef(collectionClass);
        if (integerClass) env->DeleteLocalRef(integerClass);
        return false;
    }

    jobject staged = env->NewObject(arrayListClass, constructor);
    bool failed = clearException(env, "manager cosmetic restore list") || !staged;
    size_t stagedCount = 0;
    for (const jlong id : selection) {
        if (failed) break;
        if (id < 0 || id > INT_MAX || env->PushLocalFrame(4) != JNI_OK) {
            failed = true;
            break;
        }
        jobject boxedId = env->CallStaticObjectMethod(
            integerClass, valueOf, static_cast<jint>(id));
        if (boxedId) env->CallBooleanMethod(staged, add, boxedId);
        if (clearException(env, "manager cosmetic restore item") || !boxedId) {
            failed = true;
        } else {
            ++stagedCount;
        }
        env->PopLocalFrame(nullptr);
    }
    if (!failed) {
        env->CallVoidMethod(manager, setEquipped, staged);
        failed = clearException(env, "manager cosmetic restore commit");
    }

    std::set<jlong> verified;
    const bool captured = !failed &&
        captureManagerCosmeticSelection(env, manager, verified);
    const bool valid = captured && verified == selection &&
        stagedCount == selection.size();
    logLine("MANAGER_COSMETIC_SELECTION_RESTORE requested=" +
            std::to_string(selection.size()) +
            " staged=" + std::to_string(stagedCount) +
            " verified=" + std::to_string(verified.size()) +
            " valid=" + std::to_string(valid));

    if (staged) env->DeleteLocalRef(staged);
    env->DeleteLocalRef(arrayListClass);
    env->DeleteLocalRef(collectionClass);
    env->DeleteLocalRef(integerClass);
    env->DeleteLocalRef(managerClass);
    return valid;
}

bool captureEmoteSelection(JNIEnv* env, jobject manager, std::set<EmoteSelection>& selection) {
    selection.clear();
    if (!manager) return false;
    jclass managerClass = env->GetObjectClass(manager);
    jmethodID getEquipped = managerClass ? env->GetMethodID(
        managerClass, "IRRHCHOORRCCCROHRCRORHHRCOORRI", "()Ljava/util/Set;") : nullptr;
    jobject equipped = getEquipped ? env->CallObjectMethod(manager, getEquipped) : nullptr;
    jclass collectionClass = env->FindClass("java/util/Collection");
    jclass iteratorClass = env->FindClass("java/util/Iterator");
    jmethodID iteratorMethod = collectionClass
        ? env->GetMethodID(collectionClass, "iterator", "()Ljava/util/Iterator;") : nullptr;
    jmethodID hasNext = iteratorClass ? env->GetMethodID(iteratorClass, "hasNext", "()Z") : nullptr;
    jmethodID next = iteratorClass
        ? env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;") : nullptr;
    if (clearException(env, "emote selection roots") || !managerClass || !equipped ||
        !collectionClass || !iteratorClass || !iteratorMethod || !hasNext || !next) {
        if (managerClass) env->DeleteLocalRef(managerClass);
        if (equipped) env->DeleteLocalRef(equipped);
        return false;
    }
    jobject cursor = env->CallObjectMethod(equipped, iteratorMethod);
    bool failed = clearException(env, "emote selection iterator") || !cursor;
    while (!failed && env->CallBooleanMethod(cursor, hasNext) == JNI_TRUE) {
        if (env->PushLocalFrame(12) != JNI_OK) {
            failed = true;
            break;
        }
        jobject item = env->CallObjectMethod(cursor, next);
        jclass itemClass = item ? env->GetObjectClass(item) : nullptr;
        jmethodID getId = itemClass ? env->GetMethodID(itemClass, "getEmoteId", "()I") : nullptr;
        jmethodID getSlot = itemClass ? env->GetMethodID(itemClass, "getSlotId", "()I") : nullptr;
        jmethodID getJam = itemClass ? env->GetMethodID(itemClass, "getJamId", "()I") : nullptr;
        const jint id = item && getId ? env->CallIntMethod(item, getId) : -1;
        const jint slot = item && getSlot ? env->CallIntMethod(item, getSlot) : -1;
        const jint jam = item && getJam ? env->CallIntMethod(item, getJam) : -1;
        if (clearException(env, "emote selection item")) {
            failed = true;
        } else if (id >= 0 && slot >= 0 && jam >= 0) {
            selection.insert({id, slot, jam, true});
        }
        env->PopLocalFrame(nullptr);
    }
    if (!failed && clearException(env, "emote selection completion")) failed = true;
    if (cursor) env->DeleteLocalRef(cursor);
    env->DeleteLocalRef(collectionClass);
    env->DeleteLocalRef(iteratorClass);
    env->DeleteLocalRef(managerClass);
    env->DeleteLocalRef(equipped);
    return !failed;
}

bool restoreEmoteSelection(
    JNIEnv* env, jobject manager, const std::set<EmoteSelection>& selection) {
    if (!manager || !g_persistentEquippedEmoteClass) return false;
    jclass managerClass = env->GetObjectClass(manager);
    jfieldID catalogField = managerClass ? env->GetStaticFieldID(
        managerClass, "CIHHOOROCCRIOHCHORROHOORHCOORR", "Lcom/google/common/collect/BiMap;") : nullptr;
    jobject catalog = catalogField ? env->GetStaticObjectField(managerClass, catalogField) : nullptr;
    jmethodID setEquipped = managerClass ? env->GetMethodID(
        managerClass, "IHOCOOIIIROHRRCRRIROIOCRHCOOHR", "(Ljava/util/Set;)V") : nullptr;
    jmethodID findEquipped = managerClass ? env->GetMethodID(
        managerClass, "RHHHCOIRCICOIHRRRHCOCORIHICOHH", "(I)Ljava/util/Optional;") : nullptr;
    jclass mapClass = env->FindClass("java/util/Map");
    jclass hashSetClass = env->FindClass("java/util/HashSet");
    jclass integerClass = env->FindClass("java/lang/Integer");
    jclass optionalClass = env->FindClass("java/util/Optional");
    jmethodID containsKey = mapClass ? env->GetMethodID(
        mapClass, "containsKey", "(Ljava/lang/Object;)Z") : nullptr;
    jmethodID hashSetCtor = hashSetClass ? env->GetMethodID(hashSetClass, "<init>", "()V") : nullptr;
    jmethodID add = hashSetClass ? env->GetMethodID(
        hashSetClass, "add", "(Ljava/lang/Object;)Z") : nullptr;
    jmethodID integerValue = integerClass ? env->GetStaticMethodID(
        integerClass, "valueOf", "(I)Ljava/lang/Integer;") : nullptr;
    jmethodID optionalPresent = optionalClass ? env->GetMethodID(optionalClass, "isPresent", "()Z") : nullptr;
    jmethodID optionalGet = optionalClass ? env->GetMethodID(
        optionalClass, "get", "()Ljava/lang/Object;") : nullptr;
    jmethodID wrapperCtor = env->GetMethodID(
        g_persistentEquippedEmoteClass, "<init>", "(III)V");
    if (clearException(env, "emote restore roots") || !managerClass || !catalog || !setEquipped ||
        !findEquipped || !mapClass || !hashSetClass || !integerClass || !optionalClass ||
        !containsKey || !hashSetCtor || !add || !integerValue || !optionalPresent ||
        !optionalGet || !wrapperCtor) {
        if (managerClass) env->DeleteLocalRef(managerClass);
        if (catalog) env->DeleteLocalRef(catalog);
        if (mapClass) env->DeleteLocalRef(mapClass);
        if (hashSetClass) env->DeleteLocalRef(hashSetClass);
        if (integerClass) env->DeleteLocalRef(integerClass);
        if (optionalClass) env->DeleteLocalRef(optionalClass);
        return false;
    }
    jobject staged = env->NewObject(hashSetClass, hashSetCtor);
    size_t matched = 0;
    bool failed = clearException(env, "emote restore set") || !staged;
    for (const EmoteSelection& emote : selection) {
        if (failed) break;
        if (env->PushLocalFrame(12) != JNI_OK) {
            failed = true;
            break;
        }
        jobject boxed = env->CallStaticObjectMethod(integerClass, integerValue, emote.id);
        const bool cataloged = boxed &&
            env->CallBooleanMethod(catalog, containsKey, boxed) == JNI_TRUE;
        jobject equipped = nullptr;
        if (cataloged && !emote.layoutKnown) {
            jobject existing = env->CallObjectMethod(manager, findEquipped, emote.id);
            if (existing && env->CallBooleanMethod(existing, optionalPresent) == JNI_TRUE) {
                equipped = env->CallObjectMethod(existing, optionalGet);
            }
        }
        if (cataloged && !equipped) {
            equipped = env->NewObject(
                g_persistentEquippedEmoteClass, wrapperCtor,
                emote.id, emote.slot, emote.jam);
        }
        if (equipped) {
            env->CallBooleanMethod(staged, add, equipped);
            if (!clearException(env, "emote restore add")) ++matched;
            else failed = true;
        }
        if (!failed && clearException(env, "emote restore lookup")) failed = true;
        env->PopLocalFrame(nullptr);
    }
    if (!failed) {
        env->CallVoidMethod(manager, setEquipped, staged);
        failed = clearException(env, "emote restore commit");
    }
    if (staged) env->DeleteLocalRef(staged);
    env->DeleteLocalRef(mapClass);
    env->DeleteLocalRef(hashSetClass);
    env->DeleteLocalRef(integerClass);
    env->DeleteLocalRef(optionalClass);
    env->DeleteLocalRef(catalog);
    env->DeleteLocalRef(managerClass);
    const bool valid = !failed && matched == selection.size();
    logLine("EMOTE_SELECTION_RESTORE requested=" + std::to_string(selection.size()) +
            " matched=" + std::to_string(matched) +
            " valid=" + std::to_string(valid));
    return valid;
}

bool captureSpraySelection(JNIEnv* env, jobject manager, std::map<jint, jint>& selection) {
    selection.clear();
    if (!manager) return false;
    jclass managerClass = env->GetObjectClass(manager);
    jmethodID getEquipped = managerClass ? env->GetMethodID(
        managerClass, "OOOIIOOOIHRCIIRRIIRCHIORRCICOR", "()Ljava/util/Set;") : nullptr;
    jobject equipped = getEquipped ? env->CallObjectMethod(manager, getEquipped) : nullptr;
    jclass collectionClass = env->FindClass("java/util/Collection");
    jclass iteratorClass = env->FindClass("java/util/Iterator");
    jmethodID iteratorMethod = collectionClass
        ? env->GetMethodID(collectionClass, "iterator", "()Ljava/util/Iterator;") : nullptr;
    jmethodID hasNext = iteratorClass ? env->GetMethodID(iteratorClass, "hasNext", "()Z") : nullptr;
    jmethodID next = iteratorClass
        ? env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;") : nullptr;
    if (clearException(env, "spray selection roots") || !managerClass || !equipped ||
        !collectionClass || !iteratorClass || !iteratorMethod || !hasNext || !next) {
        if (managerClass) env->DeleteLocalRef(managerClass);
        if (equipped) env->DeleteLocalRef(equipped);
        return false;
    }
    jobject cursor = env->CallObjectMethod(equipped, iteratorMethod);
    bool failed = clearException(env, "spray selection iterator") || !cursor;
    while (!failed && env->CallBooleanMethod(cursor, hasNext) == JNI_TRUE) {
        if (env->PushLocalFrame(12) != JNI_OK) {
            failed = true;
            break;
        }
        jobject item = env->CallObjectMethod(cursor, next);
        jclass itemClass = item ? env->GetObjectClass(item) : nullptr;
        jmethodID getSlot = itemClass ? env->GetMethodID(itemClass, "getSlotNumber", "()I") : nullptr;
        jmethodID getId = itemClass ? env->GetMethodID(itemClass, "getSprayId", "()I") : nullptr;
        const jint slot = item && getSlot ? env->CallIntMethod(item, getSlot) : -1;
        const jint id = item && getId ? env->CallIntMethod(item, getId) : -1;
        if (clearException(env, "spray selection item")) {
            failed = true;
        } else if (slot >= 0 && id >= 0) {
            selection[slot] = id;
        }
        env->PopLocalFrame(nullptr);
    }
    if (!failed && clearException(env, "spray selection completion")) failed = true;
    if (cursor) env->DeleteLocalRef(cursor);
    env->DeleteLocalRef(collectionClass);
    env->DeleteLocalRef(iteratorClass);
    env->DeleteLocalRef(managerClass);
    env->DeleteLocalRef(equipped);
    return !failed;
}

bool restoreSpraySelection(JNIEnv* env, jobject manager, const std::map<jint, jint>& selection) {
    if (!manager) return false;
    jclass managerClass = env->GetObjectClass(manager);
    jmethodID setEquipped = managerClass ? env->GetMethodID(
        managerClass, "HICROHCCIOCRCORICOIHRCOOHHRCCH", "(Ljava/util/Set;)V") : nullptr;
    jclass sprayClass = env->FindClass("com/lunarclient/websocket/spray/v1/EquippedSpray");
    jclass hashSetClass = env->FindClass("java/util/HashSet");
    jclass collectionClass = env->FindClass("java/util/Collection");
    jmethodID newBuilder = sprayClass ? env->GetStaticMethodID(
        sprayClass, "newBuilder", (std::string("()") + kEquippedSprayBuilderDescriptor).c_str()) : nullptr;
    jmethodID hashSetCtor = hashSetClass ? env->GetMethodID(hashSetClass, "<init>", "()V") : nullptr;
    jmethodID add = collectionClass ? env->GetMethodID(
        collectionClass, "add", "(Ljava/lang/Object;)Z") : nullptr;
    if (clearException(env, "spray restore roots") || !managerClass || !setEquipped || !sprayClass ||
        !hashSetClass || !collectionClass || !newBuilder || !hashSetCtor || !add) {
        if (managerClass) env->DeleteLocalRef(managerClass);
        return false;
    }
    jobject staged = env->NewObject(hashSetClass, hashSetCtor);
    bool failed = clearException(env, "spray restore set") || !staged;
    size_t matched = 0;
    for (const auto& item : selection) {
        if (failed) break;
        if (env->PushLocalFrame(12) != JNI_OK) {
            failed = true;
            break;
        }
        jobject builder = env->CallStaticObjectMethod(sprayClass, newBuilder);
        jclass builderClass = builder ? env->GetObjectClass(builder) : nullptr;
        jmethodID setSlot = builderClass ? env->GetMethodID(
            builderClass, "setSlotNumber", "(I)Lcom/lunarclient/websocket/spray/v1/EquippedSpray$Builder;") : nullptr;
        jmethodID setId = builderClass ? env->GetMethodID(
            builderClass, "setSprayId", "(I)Lcom/lunarclient/websocket/spray/v1/EquippedSpray$Builder;") : nullptr;
        jmethodID build = builderClass ? env->GetMethodID(
            builderClass, "build", "()Lcom/lunarclient/websocket/spray/v1/EquippedSpray;") : nullptr;
        jobject chainedSlot = (builder && setSlot)
            ? env->CallObjectMethod(builder, setSlot, item.first) : nullptr;
        jobject chainedId = (builder && setId)
            ? env->CallObjectMethod(builder, setId, item.second) : nullptr;
        jobject equipped = (builder && build)
            ? env->CallObjectMethod(builder, build) : nullptr;
        if (chainedSlot) env->DeleteLocalRef(chainedSlot);
        if (chainedId) env->DeleteLocalRef(chainedId);
        if (!equipped || clearException(env, "spray restore build")) {
            failed = true;
        } else {
            env->CallBooleanMethod(staged, add, equipped);
            if (clearException(env, "spray restore add")) failed = true;
            else ++matched;
        }
        env->PopLocalFrame(nullptr);
    }
    if (!failed) {
        env->CallVoidMethod(manager, setEquipped, staged);
        failed = clearException(env, "spray restore commit");
    }
    if (staged) env->DeleteLocalRef(staged);
    env->DeleteLocalRef(collectionClass);
    env->DeleteLocalRef(hashSetClass);
    env->DeleteLocalRef(sprayClass);
    env->DeleteLocalRef(managerClass);
    logLine("SPRAY_SELECTION_RESTORE requested=" + std::to_string(selection.size()) +
            " matched=" + std::to_string(matched) +
            " valid=" + std::to_string(!failed));
    return !failed;
}

bool captureLunarPlusSelection(JNIEnv* env, jobject response, std::optional<jint>& selection) {
    selection.reset();
    if (!response) return false;
    jclass responseClass = env->GetObjectClass(response);
    jmethodID hasColor = responseClass ? env->GetMethodID(responseClass, "hasPlusColor", "()Z") : nullptr;
    jmethodID getColor = responseClass ? env->GetMethodID(
        responseClass, "getPlusColor", "()Lcom/lunarclient/common/v1/Color;") : nullptr;
    jclass colorClass = env->FindClass("com/lunarclient/common/v1/Color");
    jmethodID getValue = colorClass ? env->GetMethodID(colorClass, "getColor", "()I") : nullptr;
    if (clearException(env, "lunar plus selection roots") || !responseClass || !hasColor ||
        !getColor || !colorClass || !getValue) {
        if (responseClass) env->DeleteLocalRef(responseClass);
        if (colorClass) env->DeleteLocalRef(colorClass);
        return false;
    }
    const jboolean present = env->CallBooleanMethod(response, hasColor);
    jobject color = present == JNI_TRUE ? env->CallObjectMethod(response, getColor) : nullptr;
    if (clearException(env, "lunar plus selection read")) {
        env->DeleteLocalRef(colorClass);
        env->DeleteLocalRef(responseClass);
        return false;
    }
    if (color) {
        const jint value = env->CallIntMethod(color, getValue);
        if (!clearException(env, "lunar plus color value")) selection = value;
        env->DeleteLocalRef(color);
    }
    env->DeleteLocalRef(colorClass);
    env->DeleteLocalRef(responseClass);
    return true;
}

struct SelectionCaptureStatus {
    bool any = false;
    bool complete = true;
};

SelectionCaptureStatus captureSelection(JNIEnv* env, SelectionState& selection) {
    SelectionCaptureStatus status;
    if (g_persistentCosmeticManager && g_persistentLocalPlayerUuid) {
        std::set<jlong> cosmetics;
        if (captureLocalPlayerCosmeticSelection(
                env, g_persistentCosmeticManager, cosmetics)) {
            selection.cosmetics = std::move(cosmetics);
            status.any = true;
        } else status.complete = false;
    } else status.complete = false;
    if (g_persistentEmoteManager) {
        std::set<EmoteSelection> emotes;
        if (captureEmoteSelection(env, g_persistentEmoteManager, emotes)) {
            selection.emotes = std::move(emotes);
            status.any = true;
        } else status.complete = false;
    } else status.complete = false;
    if (g_persistentSprayManager) {
        std::map<jint, jint> sprays;
        if (captureSpraySelection(env, g_persistentSprayManager, sprays)) {
            selection.sprays = std::move(sprays);
            status.any = true;
        } else status.complete = false;
    } else status.complete = false;
    if (g_persistentLoginResponse) {
        std::optional<jint> lunarPlus;
        if (captureLunarPlusSelection(env, g_persistentLoginResponse, lunarPlus)) {
            selection.lunarPlus = lunarPlus;
            status.any = true;
        } else status.complete = false;
    } else status.complete = false;
    return status;
}

bool restoreSelection(JNIEnv* env, const SelectionState& selection) {
    bool valid = true;
    if (g_persistentCosmeticManager && g_persistentLocalPlayerUuid) {
        const bool managerRestored = restoreManagerCosmeticSelection(
            env, g_persistentCosmeticManager, selection.cosmetics);
        const bool playerRestored = restoreLocalPlayerCosmeticSelection(
            env, g_persistentCosmeticManager, selection.cosmetics);
        if (g_persistentPlayerCosmeticState) {
            const bool previewRestored = restorePlayerCosmeticSelection(
                env, g_persistentCosmeticManager,
                g_persistentPlayerCosmeticState, selection.cosmetics);
            logLine("LOADOUT_PREVIEW_RESTORE requested=" +
                    std::to_string(selection.cosmetics.size()) +
                    " valid=" + std::to_string(previewRestored));
        }
        g_localCosmeticSelectionInitialized = playerRestored;
        valid = managerRestored && playerRestored && valid;
    } else {
        g_localCosmeticSelectionInitialized = false;
        valid = false;
    }
    if (g_persistentEmoteManager) {
        valid = restoreEmoteSelection(env, g_persistentEmoteManager, selection.emotes) && valid;
    } else {
        valid = false;
    }
    if (g_persistentSprayManager) {
        valid = restoreSpraySelection(env, g_persistentSprayManager, selection.sprays) && valid;
    } else {
        valid = false;
    }
    if (!g_persistentLoginResponse) {
        valid = false;
    }
    return valid;
}

void monitorSelection(JNIEnv* env, jvmtiEnv* jvmti) {
    constexpr DWORD kMonitorIntervalMs = 1000;
    constexpr unsigned int kManagerHealthPeriod = 10;
    constexpr unsigned int kRecoveryProbePeriod = 5;
    constexpr unsigned int kDeferredPreviewProbe = 5;
    SelectionState previous = g_hasSavedSelection ? g_savedSelection : SelectionState{};
    bool started = false;
    bool waitingLogged = false;
    bool lastComplete = true;
    bool previewProbePending = g_persistentPlayerCosmeticState == nullptr;
    unsigned int iteration = 0;

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
    logLine("SELECTION_MONITOR_POLICY interval_ms=" +
            std::to_string(kMonitorIntervalMs) +
            " steady_heap_scan=0 preview_probe=deferred_once");

    for (;;) {
        if (env->PushLocalFrame(256) != JNI_OK) {
            logLine("SELECTION_LOCAL_FRAME_FAILED");
            Sleep(kMonitorIntervalMs);
            continue;
        }
        ++iteration;
        bool managerRebound = false;
        if (!g_persistentCosmeticManager &&
            (iteration == 1 || iteration % kRecoveryProbePeriod == 0)) {
            managerRebound = refreshPersistentCosmeticManager(env, jvmti);
            if (managerRebound) g_localCosmeticSelectionInitialized = false;
        }
        if (g_persistentCosmeticManager &&
            (iteration == 1 || iteration % kManagerHealthPeriod == 0)) {
            ManagerState live = inspectManager(
                env, jvmti, g_persistentCosmeticManager);
            if (live.catalog <= 0 || live.owned < 0 || live.ownedBySerial < 0) {
                managerRebound = refreshPersistentCosmeticManager(env, jvmti);
                if (managerRebound) {
                    g_localCosmeticSelectionInitialized = false;
                    live = inspectManager(env, jvmti, g_persistentCosmeticManager);
                }
            }
            const bool ownedLooksReset = live.catalog > 0 && live.owned >= 0 &&
                live.owned < (live.catalog * 3) / 4;
            if (ownedLooksReset) {
                const bool repaired = populateOwnedFromCatalog(
                    env, jvmti, g_persistentCosmeticManager);
                logLine("COSMETIC_UNLOCK_REFRESH rebound=" +
                        std::to_string(managerRebound) +
                        " owned_before=" + std::to_string(live.owned) +
                        " repaired=" + std::to_string(repaired));
            }
        }

        bool localPlayerAvailable = g_persistentLocalPlayerUuid != nullptr;
        if (!localPlayerAvailable &&
            (iteration == 1 || iteration % kRecoveryProbePeriod == 0)) {
            localPlayerAvailable = bindLocalPlayerUuid(env, jvmti);
        }

        if (!g_localCosmeticSelectionInitialized && localPlayerAvailable &&
            g_persistentCosmeticManager && (iteration == 1 || iteration % 4 == 0)) {
            const std::set<jlong> desired = g_hasSavedSelection
                ? g_savedSelection.cosmetics : std::set<jlong>{};
            const bool managerRestored = restoreManagerCosmeticSelection(
                env, g_persistentCosmeticManager, desired);
            const bool playerRestored = restoreLocalPlayerCosmeticSelection(
                env, g_persistentCosmeticManager, desired);
            g_localCosmeticSelectionInitialized = playerRestored;
            logLine(managerRestored && playerRestored
                        ? "LOCAL_PLAYER_DEFERRED_RESTORE_COMPLETE"
                        : "LOCAL_PLAYER_DEFERRED_RESTORE_FAILED");
            waitingLogged = false;
        }

        // Retry discovery once for a preview model created shortly after
        // injection. Further heap walks happen only after a real selection
        // change; periodic JVMTI walks force JVM safepoints and stall frames.
        if (previewProbePending && iteration == kDeferredPreviewProbe) {
            previewProbePending = false;
            if (!g_persistentPlayerCosmeticState &&
                bindPlayerCosmeticState(env, jvmti) &&
                g_persistentCosmeticManager) {
                const std::set<jlong> desired = g_hasSavedSelection
                    ? g_savedSelection.cosmetics : std::set<jlong>{};
                const bool previewRestored = restorePlayerCosmeticSelection(
                    env, g_persistentCosmeticManager,
                    g_persistentPlayerCosmeticState, desired);
                logLine("LOADOUT_PREVIEW_REFRESH requested=" +
                        std::to_string(desired.size()) +
                        " valid=" + std::to_string(previewRestored));
            }
        }

        SelectionState current = previous;
        const SelectionCaptureStatus status = captureSelection(env, current);
        if (!status.any) {
            if (!waitingLogged) {
                logLine("SELECTION_MONITOR_WAITING reason=MANAGERS_NOT_READY");
                waitingLogged = true;
            }
        } else if (!started) {
            // Do not establish a baseline from a partial snapshot.  During
            // client startup individual managers become available at
            // different times; persisting that intermediate state would
            // overwrite a valid config with empty categories.
            if (!status.complete) {
                if (!waitingLogged) {
                    logLine("SELECTION_MONITOR_WAITING reason=PARTIAL_STATE");
                    waitingLogged = true;
                }
            } else if (!saveSelection(current)) {
                logLine("SELECTION_SAVE_FAILED reason=MONITOR_BASELINE");
            } else {
                previous = std::move(current);
                g_savedSelection = previous;
                g_hasSavedSelection = true;
                started = true;
                lastComplete = true;
                logLine("SELECTION_MONITOR_STARTED interval_ms=" +
                        std::to_string(kMonitorIntervalMs) + " complete=1");
            }
        } else if (!status.complete) {
            // A transient JNI/manager failure must not be interpreted as a
            // user clearing a category.  Keep the last complete snapshot and
            // retry on the next interval.
            if (lastComplete) {
                logLine("SELECTION_CAPTURE_PARTIAL");
                lastComplete = false;
            }
        } else {
            if (status.complete != lastComplete) {
                logLine("SELECTION_CAPTURE_COMPLETE");
                lastComplete = status.complete;
            }
            if (current != previous) {
                if (saveSelection(current)) {
                    logLine("SELECTION_SAVE_CHANGED");
                    previous = std::move(current);
                    g_savedSelection = previous;
                    g_hasSavedSelection = true;
                    if (!g_persistentPlayerCosmeticState &&
                        g_persistentCosmeticManager &&
                        bindPlayerCosmeticState(env, jvmti)) {
                        const bool previewRestored = restorePlayerCosmeticSelection(
                            env, g_persistentCosmeticManager,
                            g_persistentPlayerCosmeticState,
                            g_savedSelection.cosmetics);
                        logLine("LOADOUT_PREVIEW_CHANGE_RESTORE requested=" +
                                std::to_string(g_savedSelection.cosmetics.size()) +
                                " valid=" + std::to_string(previewRestored));
                    }
                } else {
                    logLine("SELECTION_SAVE_FAILED reason=CHANGED");
                }
            }
        }
        env->PopLocalFrame(nullptr);
        Sleep(kMonitorIntervalMs);
    }
}

bool unlockEmotes(JNIEnv* env, jvmtiEnv* jvmti) {
    jclass managerClass = findLoadedClass(env, jvmti, kEmoteManagerSignature);
    jclass wrapperClass = findLoadedClass(env, jvmti, kEmoteOwnedWrapperSignature);
    jclass equippedWrapperClass = findLoadedClass(
        env, jvmti, kEquippedEmoteWrapperSignature);
    if (!managerClass || !wrapperClass || !equippedWrapperClass) return false;

    jobject manager = firstInstance(env, jvmti, managerClass);
    jfieldID catalogField = resolveField(
        env, jvmti, managerClass, "CIHHOOROCCRIOHCHORROHOORHCOORR",
        "Lcom/google/common/collect/BiMap;", true, "emote_catalog_field");
    jobject catalog = catalogField
        ? env->GetStaticObjectField(managerClass, catalogField)
        : nullptr;
    if (clearException(env, "emote manager/catalog") || !manager || !catalog) return false;

    jclass mapClass = env->FindClass("java/util/Map");
    jclass collectionClass = env->FindClass("java/util/Collection");
    jclass iteratorClass = env->FindClass("java/util/Iterator");
    jclass integerClass = env->FindClass("java/lang/Integer");
    jclass arrayListClass = env->FindClass("java/util/ArrayList");
    if (clearException(env, "emote Java classes") || !mapClass || !collectionClass ||
        !iteratorClass || !integerClass || !arrayListClass) return false;

    jmethodID keySet = env->GetMethodID(mapClass, "keySet", "()Ljava/util/Set;");
    jmethodID iterator = env->GetMethodID(
        collectionClass, "iterator", "()Ljava/util/Iterator;");
    jmethodID hasNext = env->GetMethodID(iteratorClass, "hasNext", "()Z");
    jmethodID next = env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;");
    jmethodID intValue = env->GetMethodID(integerClass, "intValue", "()I");
    jmethodID listCtor = env->GetMethodID(arrayListClass, "<init>", "()V");
    jmethodID add = env->GetMethodID(
        collectionClass, "add", "(Ljava/lang/Object;)Z");
    const std::string wrapperCtorDescriptor =
        std::string("(IJLjava/time/Instant;Ljava/util/List;") +
        kEmoteMetadataDescriptor + ")V";
    jmethodID wrapperCtor = env->GetMethodID(
        wrapperClass, "<init>", wrapperCtorDescriptor.c_str());
    jmethodID setOwned = resolveMethod(
        env, jvmti, managerClass, "OIRCHOOIOHIHCOCIOCRRIRROROOIOO",
        "(Ljava/util/List;)V", false, "emote_owned_setter");
    jmethodID getOwned = resolveMethod(
        env, jvmti, managerClass, "RRHOICCICRIICHHOCCICIIHCCROOCC",
        "()Ljava/util/List;", false, "emote_owned_getter");
    jmethodID setOwnsPlus = resolveMethod(
        env, jvmti, managerClass, "OOOOOOROIIRHCIORCIOHRCIROIHIHC",
        "(Z)V", false, "emote_plus_setter");
    if (clearException(env, "emote methods") || !keySet || !iterator || !hasNext ||
        !next || !intValue || !listCtor || !add || !wrapperCtor || !setOwned ||
        !getOwned || !setOwnsPlus) return false;

    jobject keys = env->CallObjectMethod(catalog, keySet);
    jobject keyIterator = keys ? env->CallObjectMethod(keys, iterator) : nullptr;
    jobject staged = env->NewObject(arrayListClass, listCtor);
    jobject emptySlots = env->NewObject(arrayListClass, listCtor);
    if (clearException(env, "emote staging setup") || !keys || !keyIterator ||
        !staged || !emptySlots) return false;

    jint built = 0;
    bool failed = false;
    while (env->CallBooleanMethod(keyIterator, hasNext) == JNI_TRUE) {
        if (clearException(env, "emote iterator hasNext") ||
            env->PushLocalFrame(12) != JNI_OK) {
            failed = true;
            break;
        }
        jobject boxedId = env->CallObjectMethod(keyIterator, next);
        const jint id = boxedId ? env->CallIntMethod(boxedId, intValue) : 0;
        jobject owned = boxedId
            ? env->NewObject(wrapperClass, wrapperCtor, id, static_cast<jlong>(-1),
                             static_cast<jobject>(nullptr), emptySlots,
                             static_cast<jobject>(nullptr))
            : nullptr;
        if (!owned || clearException(env, "emote wrapper build")) {
            env->PopLocalFrame(nullptr);
            failed = true;
            break;
        }
        env->CallBooleanMethod(staged, add, owned);
        if (clearException(env, "emote staging add")) {
            env->PopLocalFrame(nullptr);
            failed = true;
            break;
        }
        ++built;
        env->PopLocalFrame(nullptr);
    }

    const jint catalogCount = collectionSize(env, catalog);
    const jint stagedCount = collectionSize(env, staged);
    if (!failed && catalogCount > 0 && stagedCount == catalogCount) {
        env->CallVoidMethod(manager, setOwned, staged);
        env->CallVoidMethod(manager, setOwnsPlus, JNI_TRUE);
        failed = clearException(env, "emote commit");
    }
    jobject verifiedOwned = !failed ? env->CallObjectMethod(manager, getOwned) : nullptr;
    const jint verified = collectionSize(env, verifiedOwned);
    const bool valid = !failed && catalogCount > 0 && verified == catalogCount;
    if (valid) {
        replaceGlobalRef(env, g_persistentEmoteManager, manager);
        replaceGlobalClassRef(env, g_persistentEquippedEmoteClass, equippedWrapperClass);
    }
    logLine("EMOTE_UNLOCK catalog=" + std::to_string(catalogCount) +
            " built=" + std::to_string(built) +
            " owned=" + std::to_string(verified) +
            " valid=" + std::to_string(valid));
    return valid;
}

bool unlockJams(JNIEnv* env, jvmtiEnv* jvmti) {
    jclass managerClass = findLoadedClass(env, jvmti, kJamManagerSignature);
    jclass ownedClass = findLoadedClass(env, jvmti, kOwnedJamSignature);
    if (!managerClass || !ownedClass) return false;
    jobject manager = firstInstance(env, jvmti, managerClass);
    if (!manager) return false;

    jmethodID getCatalog = resolveMethod(
        env, jvmti, managerClass, "RCRIHOIIOICOORCRHRCHIICHRHORRI",
        "()Ljava/util/Map;", true, "jam_catalog_getter");
    jmethodID newBuilder = resolveMethod(
        env, jvmti, ownedClass, "newBuilder",
        std::string("()") + kOwnedJamBuilderDescriptor, true,
        "jam_builder_factory");
    if (clearException(env, "jam roots") || !getCatalog || !newBuilder) return false;
    jobject catalog = env->CallStaticObjectMethod(managerClass, getCatalog);
    if (clearException(env, "jam catalog") || !catalog) return false;

    jclass mapClass = env->FindClass("java/util/Map");
    jclass collectionClass = env->FindClass("java/util/Collection");
    jclass iteratorClass = env->FindClass("java/util/Iterator");
    jclass integerClass = env->FindClass("java/lang/Integer");
    jclass arrayListClass = env->FindClass("java/util/ArrayList");
    jmethodID keySet = mapClass
        ? env->GetMethodID(mapClass, "keySet", "()Ljava/util/Set;") : nullptr;
    jmethodID iterator = collectionClass
        ? env->GetMethodID(collectionClass, "iterator", "()Ljava/util/Iterator;") : nullptr;
    jmethodID hasNext = iteratorClass
        ? env->GetMethodID(iteratorClass, "hasNext", "()Z") : nullptr;
    jmethodID next = iteratorClass
        ? env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;") : nullptr;
    jmethodID intValue = integerClass
        ? env->GetMethodID(integerClass, "intValue", "()I") : nullptr;
    jmethodID listCtor = arrayListClass
        ? env->GetMethodID(arrayListClass, "<init>", "()V") : nullptr;
    jmethodID add = collectionClass
        ? env->GetMethodID(collectionClass, "add", "(Ljava/lang/Object;)Z") : nullptr;
    jmethodID setOwned = resolveMethod(
        env, jvmti, managerClass, "OHOOIRHHHROHOCIIOIOCHOHCCRROIO",
        "(Ljava/util/List;)V", false, "jam_owned_setter");
    jmethodID getOwned = resolveMethod(
        env, jvmti, managerClass, "CIROROCOIRIOROICHOIOCRRICIOICI",
        "()Ljava/util/List;", false, "jam_owned_getter");
    if (clearException(env, "jam Java methods") || !keySet || !iterator || !hasNext ||
        !next || !intValue || !listCtor || !add || !setOwned || !getOwned) return false;

    jobject keys = env->CallObjectMethod(catalog, keySet);
    jobject keyIterator = keys ? env->CallObjectMethod(keys, iterator) : nullptr;
    jobject staged = env->NewObject(arrayListClass, listCtor);
    if (clearException(env, "jam staging setup") || !keys || !keyIterator || !staged) {
        return false;
    }

    jint built = 0;
    bool failed = false;
    while (env->CallBooleanMethod(keyIterator, hasNext) == JNI_TRUE) {
        if (clearException(env, "jam iterator hasNext") ||
            env->PushLocalFrame(12) != JNI_OK) {
            failed = true;
            break;
        }
        jobject boxedId = env->CallObjectMethod(keyIterator, next);
        const jint id = boxedId ? env->CallIntMethod(boxedId, intValue) : 0;
        jobject builder = env->CallStaticObjectMethod(ownedClass, newBuilder);
        jclass builderClass = builder ? env->GetObjectClass(builder) : nullptr;
        jmethodID setId = builderClass
            ? env->GetMethodID(builderClass, "setJamId",
                               (std::string("(I)") + kOwnedJamBuilderDescriptor).c_str())
            : nullptr;
        jmethodID build = builderClass
            ? env->GetMethodID(builderClass, "build",
                               (std::string("()") + kOwnedJamDescriptor).c_str())
            : nullptr;
        jobject chained = setId ? env->CallObjectMethod(builder, setId, id) : nullptr;
        jobject owned = build ? env->CallObjectMethod(builder, build) : nullptr;
        if (chained) env->DeleteLocalRef(chained);
        if (!owned || clearException(env, "jam protobuf build")) {
            env->PopLocalFrame(nullptr);
            failed = true;
            break;
        }
        env->CallBooleanMethod(staged, add, owned);
        if (clearException(env, "jam staging add")) {
            env->PopLocalFrame(nullptr);
            failed = true;
            break;
        }
        ++built;
        env->PopLocalFrame(nullptr);
    }

    const jint catalogCount = collectionSize(env, catalog);
    const jint stagedCount = collectionSize(env, staged);
    if (!failed && catalogCount > 0 && stagedCount == catalogCount) {
        env->CallVoidMethod(manager, setOwned, staged);
        failed = clearException(env, "jam commit");
    }
    jobject verifiedOwned = !failed ? env->CallObjectMethod(manager, getOwned) : nullptr;
    const jint verified = collectionSize(env, verifiedOwned);
    const bool valid = !failed && catalogCount > 0 && verified == catalogCount;
    logLine("JAM_UNLOCK catalog=" + std::to_string(catalogCount) +
            " built=" + std::to_string(built) +
            " owned=" + std::to_string(verified) +
            " valid=" + std::to_string(valid));
    return valid;
}

bool unlockSprays(JNIEnv* env, jvmtiEnv* jvmti) {
    jclass managerClass = findLoadedClass(env, jvmti, kSprayManagerSignature);
    if (!managerClass) return false;
    jobject manager = firstInstance(env, jvmti, managerClass);
    if (!manager) return false;

    jmethodID getCatalog = resolveMethod(
        env, jvmti, managerClass, "RHRRICOCRHCIIRHOCOIRRHCOHIHCCH",
        "()Lit/unimi/dsi/fastutil/ints/Int2ObjectMap;", false,
        "spray_catalog_getter");
    jmethodID getOwned = resolveMethod(
        env, jvmti, managerClass, "HIIOIIOCIRCHOIHCOORHOHHRRCRCHH",
        "()Lit/unimi/dsi/fastutil/objects/Object2LongMap;", false,
        "spray_owned_getter");
    jmethodID setOwned = resolveMethod(
        env, jvmti, managerClass, "RCRROIORHICCOHOIIIRROHIORIIIHC",
        "(Lit/unimi/dsi/fastutil/objects/Object2LongMap;)V", false,
        "spray_owned_setter");
    if (clearException(env, "spray accessors") || !getCatalog || !getOwned || !setOwned) {
        return false;
    }
    jobject catalog = env->CallObjectMethod(manager, getCatalog);
    jobject targetOwned = env->CallObjectMethod(manager, getOwned);
    if (clearException(env, "spray collections") || !catalog || !targetOwned) return false;

    jclass targetClass = env->GetObjectClass(targetOwned);
    jclass mapClass = env->FindClass("java/util/Map");
    jclass collectionClass = env->FindClass("java/util/Collection");
    jclass iteratorClass = env->FindClass("java/util/Iterator");
    jmethodID mapValues = mapClass
        ? env->GetMethodID(mapClass, "values", "()Ljava/util/Collection;") : nullptr;
    jmethodID iterator = collectionClass
        ? env->GetMethodID(collectionClass, "iterator", "()Ljava/util/Iterator;") : nullptr;
    jmethodID hasNext = iteratorClass
        ? env->GetMethodID(iteratorClass, "hasNext", "()Z") : nullptr;
    jmethodID next = iteratorClass
        ? env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;") : nullptr;
    jmethodID targetCtor = targetClass
        ? env->GetMethodID(targetClass, "<init>", "()V") : nullptr;
    jmethodID put = targetClass
        ? env->GetMethodID(targetClass, "put", "(Ljava/lang/Object;J)J") : nullptr;
    if (clearException(env, "spray Java methods") || !mapValues || !iterator ||
        !hasNext || !next || !targetCtor || !put) return false;

    jobject staged = env->NewObject(targetClass, targetCtor);
    jobject values = env->CallObjectMethod(catalog, mapValues);
    jobject valueIterator = values ? env->CallObjectMethod(values, iterator) : nullptr;
    if (clearException(env, "spray staging setup") || !staged || !values || !valueIterator) {
        return false;
    }

    jint built = 0;
    bool failed = false;
    while (env->CallBooleanMethod(valueIterator, hasNext) == JNI_TRUE) {
        if (clearException(env, "spray iterator hasNext") ||
            env->PushLocalFrame(8) != JNI_OK) {
            failed = true;
            break;
        }
        jobject metadata = env->CallObjectMethod(valueIterator, next);
        if (!metadata || clearException(env, "spray iterator next")) {
            env->PopLocalFrame(nullptr);
            failed = true;
            break;
        }
        env->CallLongMethod(staged, put, metadata, static_cast<jlong>(-1));
        if (clearException(env, "spray staging put")) {
            env->PopLocalFrame(nullptr);
            failed = true;
            break;
        }
        ++built;
        env->PopLocalFrame(nullptr);
    }

    const jint catalogCount = collectionSize(env, catalog);
    const jint stagedCount = collectionSize(env, staged);
    if (!failed && catalogCount > 0 && stagedCount == catalogCount) {
        env->CallVoidMethod(manager, setOwned, staged);
        failed = clearException(env, "spray commit");
    }
    jobject verifiedOwned = !failed ? env->CallObjectMethod(manager, getOwned) : nullptr;
    const jint verified = collectionSize(env, verifiedOwned);
    const bool valid = !failed && catalogCount > 0 && verified == catalogCount;
    if (valid) replaceGlobalRef(env, g_persistentSprayManager, manager);
    logLine("SPRAY_UNLOCK catalog=" + std::to_string(catalogCount) +
            " built=" + std::to_string(built) +
            " owned=" + std::to_string(verified) +
            " valid=" + std::to_string(valid));
    return valid;
}

bool unlockBadges(JNIEnv* env, jvmtiEnv* jvmti) {
    jclass managerClass = findLoadedClass(env, jvmti, kBadgeManagerSignature);
    jclass wrapperClass = findLoadedClass(env, jvmti, kBadgeWrapperSignature);
    if (!managerClass || !wrapperClass) return false;
    jobject manager = firstInstance(env, jvmti, managerClass);
    if (!manager) return false;

    jmethodID getCatalog = resolveMethod(
        env, jvmti, managerClass, "OORHOCIOCIIROOOHHCRCHCCOICOCHH",
        "()Ljava/util/Map;", false, "badge_catalog_getter");
    const std::string converterDescriptor =
        std::string("(") + kBadgeMetadataDescriptor + ")" + kBadgeWrapperDescriptor;
    jmethodID convert = resolveMethod(
        env, jvmti, wrapperClass, "RCRROIORHICCOHOIIIRROHIORIIIHC",
        converterDescriptor, true, "badge_wrapper_factory");
    jmethodID setOwned = resolveMethod(
        env, jvmti, managerClass, "CIHROHOORRHCROOIRRCROCRIHROHIH",
        "(Ljava/util/List;)V", false, "badge_owned_setter");
    jfieldID ownedField = resolveField(
        env, jvmti, managerClass, "CCRHORCHOCCCOCOOCRRRRORHOHOHRI",
        "Ljava/util/List;", false, "badge_owned_field", {kBadgeWrapperDescriptor});
    if (clearException(env, "badge accessors") || !getCatalog || !convert ||
        !setOwned || !ownedField) return false;
    jobject catalog = env->CallObjectMethod(manager, getCatalog);
    if (clearException(env, "badge catalog") || !catalog) return false;

    jclass mapClass = env->FindClass("java/util/Map");
    jclass collectionClass = env->FindClass("java/util/Collection");
    jclass iteratorClass = env->FindClass("java/util/Iterator");
    jclass arrayListClass = env->FindClass("java/util/ArrayList");
    jmethodID mapValues = mapClass
        ? env->GetMethodID(mapClass, "values", "()Ljava/util/Collection;") : nullptr;
    jmethodID iterator = collectionClass
        ? env->GetMethodID(collectionClass, "iterator", "()Ljava/util/Iterator;") : nullptr;
    jmethodID hasNext = iteratorClass
        ? env->GetMethodID(iteratorClass, "hasNext", "()Z") : nullptr;
    jmethodID next = iteratorClass
        ? env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;") : nullptr;
    jmethodID listCtor = arrayListClass
        ? env->GetMethodID(arrayListClass, "<init>", "()V") : nullptr;
    jmethodID add = collectionClass
        ? env->GetMethodID(collectionClass, "add", "(Ljava/lang/Object;)Z") : nullptr;
    if (clearException(env, "badge Java methods") || !mapValues || !iterator ||
        !hasNext || !next || !listCtor || !add) return false;

    jobject values = env->CallObjectMethod(catalog, mapValues);
    jobject valueIterator = values ? env->CallObjectMethod(values, iterator) : nullptr;
    jobject staged = env->NewObject(arrayListClass, listCtor);
    if (clearException(env, "badge staging setup") || !values || !valueIterator || !staged) {
        return false;
    }

    jint built = 0;
    bool failed = false;
    while (env->CallBooleanMethod(valueIterator, hasNext) == JNI_TRUE) {
        if (clearException(env, "badge iterator hasNext") ||
            env->PushLocalFrame(8) != JNI_OK) {
            failed = true;
            break;
        }
        jobject metadata = env->CallObjectMethod(valueIterator, next);
        jobject owned = metadata
            ? env->CallStaticObjectMethod(wrapperClass, convert, metadata)
            : nullptr;
        if (!owned || clearException(env, "badge wrapper build")) {
            env->PopLocalFrame(nullptr);
            failed = true;
            break;
        }
        env->CallBooleanMethod(staged, add, owned);
        if (clearException(env, "badge staging add")) {
            env->PopLocalFrame(nullptr);
            failed = true;
            break;
        }
        ++built;
        env->PopLocalFrame(nullptr);
    }

    const jint catalogCount = collectionSize(env, catalog);
    const jint stagedCount = collectionSize(env, staged);
    if (!failed && catalogCount > 0 && stagedCount == catalogCount) {
        env->CallVoidMethod(manager, setOwned, staged);
        failed = clearException(env, "badge commit");
    }
    jobject verifiedOwned = !failed ? env->GetObjectField(manager, ownedField) : nullptr;
    const jint verified = collectionSize(env, verifiedOwned);
    const bool valid = !failed && catalogCount > 0 && verified == catalogCount;
    logLine("BADGE_UNLOCK catalog=" + std::to_string(catalogCount) +
            " built=" + std::to_string(built) +
            " owned=" + std::to_string(verified) +
            " valid=" + std::to_string(valid));
    return valid;
}

bool unlockLunarPlus(JNIEnv* env, jvmtiEnv* jvmti,
                     jclass responseClass, jobject response,
                     const std::optional<jint>& requestedColor) {
    jclass managerClass = findLoadedClass(env, jvmti, kLunarPlusManagerSignature);
    jclass colorClass = findLoadedClass(env, jvmti, kColorSignature);
    if (!managerClass || !colorClass || !responseClass || !response) return false;
    jobject manager = firstInstance(env, jvmti, managerClass);
    if (!manager) return false;

    jmethodID getAvailable = env->GetMethodID(
        responseClass, "getAvailableLunarPlusColorsList", "()Ljava/util/List;");
    jmethodID toBuilder = env->GetMethodID(
        responseClass, "toBuilder", (std::string("()") + kLoginResponseBuilderDescriptor).c_str());
    jmethodID newColorBuilder = env->GetStaticMethodID(
        colorClass, "newBuilder", (std::string("()") + kColorBuilderDescriptor).c_str());
    if (clearException(env, "lunar plus roots") || !getAvailable || !toBuilder ||
        !newColorBuilder) return false;

    jobject available = env->CallObjectMethod(response, getAvailable);
    jobject loginBuilder = env->CallObjectMethod(response, toBuilder);
    jclass loginBuilderClass = loginBuilder ? env->GetObjectClass(loginBuilder) : nullptr;
    jclass collectionClass = env->FindClass("java/util/Collection");
    jclass listClass = env->FindClass("java/util/List");
    jclass arrayListClass = env->FindClass("java/util/ArrayList");
    jmethodID listCtor = arrayListClass
        ? env->GetMethodID(arrayListClass, "<init>", "()V") : nullptr;
    jmethodID add = collectionClass
        ? env->GetMethodID(collectionClass, "add", "(Ljava/lang/Object;)Z") : nullptr;
    jmethodID addAll = collectionClass
        ? env->GetMethodID(collectionClass, "addAll", "(Ljava/util/Collection;)Z") : nullptr;
    jmethodID listGet = listClass
        ? env->GetMethodID(listClass, "get", "(I)Ljava/lang/Object;") : nullptr;
    jmethodID clearColors = loginBuilderClass
        ? env->GetMethodID(loginBuilderClass, "clearAvailableLunarPlusColors",
                           (std::string("()") + kLoginResponseBuilderDescriptor).c_str())
        : nullptr;
    jmethodID addAllColors = loginBuilderClass
        ? env->GetMethodID(loginBuilderClass, "addAllAvailableLunarPlusColors",
                           (std::string("(Ljava/lang/Iterable;)") +
                            kLoginResponseBuilderDescriptor).c_str())
        : nullptr;
    jmethodID setPlusColor = loginBuilderClass
        ? env->GetMethodID(loginBuilderClass, "setPlusColor",
                           (std::string("(") + kColorDescriptor + ")" +
                            kLoginResponseBuilderDescriptor).c_str())
        : nullptr;
    jmethodID buildLogin = loginBuilderClass
        ? env->GetMethodID(loginBuilderClass, "build",
                           (std::string("()") + kLoginResponseDescriptor).c_str())
        : nullptr;
    jmethodID handleLogin = resolveMethod(
        env, jvmti, managerClass, "RCRHHRIOICOORCIRCHOHHRHHIRRCRI",
        std::string("(") + kLoginResponseDescriptor + ")V", false,
        "lunar_plus_login_handler");
    jmethodID getColors = resolveMethod(
        env, jvmti, managerClass, "IOOCHIHIHRRIRORHHRHOIORORCIHCR",
        "()Ljava/util/Set;", false, "lunar_plus_colors_getter");
    jmethodID isActive = resolveMethod(
        env, jvmti, managerClass, "COOIHIRICCHICOOCORRCOCCHHIRHIC",
        "()Z", false, "lunar_plus_active_getter");
    // Provider refresh has several same-shaped siblings. Only call the known
    // name; the login handler itself is the authoritative state mutation.
    jmethodID refreshProvider = tryMethodByName(
        env, managerClass, "IOHRRCHCRIHRIOOHOHRIRHCHIROCRO", "()V", false);
    if (clearException(env, "lunar plus methods") || !available || !loginBuilder ||
        !listCtor || !add || !addAll || !listGet || !clearColors || !addAllColors ||
        !setPlusColor || !buildLogin || !handleLogin || !getColors || !isActive) {
        return false;
    }

    jobject stagedColors = env->NewObject(arrayListClass, listCtor);
    const jint responseColorCount = collectionSize(env, available);
    if (responseColorCount > 0) {
        env->CallBooleanMethod(stagedColors, addAll, available);
    } else {
        static constexpr jint kFallbackPalette[] = {
            0xFFFFFF, 0xE5E7EB, 0x9CA3AF, 0x4B5563, 0x111827,
            0xFF5555, 0xFB7185, 0xF472B6, 0xD946EF, 0xA855F7,
            0x8B5CF6, 0x6366F1, 0x4F46E5, 0x3B82F6, 0x0EA5E9,
            0x06B6D4, 0x14B8A6, 0x10B981, 0x22C55E, 0x84CC16,
            0xA3E635, 0xFACC15, 0xF59E0B, 0xF97316, 0xEF4444,
            0x7F1D1D, 0x7C2D12, 0x713F12, 0x14532D, 0x134E4A,
            0x164E63, 0x1E3A8A, 0x312E81, 0x581C87, 0x701A75,
            0x831843
        };
        for (jint rgb : kFallbackPalette) {
            if (env->PushLocalFrame(8) != JNI_OK) return false;
            jobject colorBuilder = env->CallStaticObjectMethod(colorClass, newColorBuilder);
            jclass colorBuilderClass = colorBuilder ? env->GetObjectClass(colorBuilder) : nullptr;
            jmethodID setColor = colorBuilderClass
                ? env->GetMethodID(colorBuilderClass, "setColor",
                                   (std::string("(I)") + kColorBuilderDescriptor).c_str())
                : nullptr;
            jmethodID buildColor = colorBuilderClass
                ? env->GetMethodID(colorBuilderClass, "build",
                                   (std::string("()") + kColorDescriptor).c_str())
                : nullptr;
            jobject chained = setColor ? env->CallObjectMethod(colorBuilder, setColor, rgb) : nullptr;
            jobject color = buildColor ? env->CallObjectMethod(colorBuilder, buildColor) : nullptr;
            if (chained) env->DeleteLocalRef(chained);
            if (!color || clearException(env, "lunar plus color build")) {
                env->PopLocalFrame(nullptr);
                return false;
            }
            env->CallBooleanMethod(stagedColors, add, color);
            if (clearException(env, "lunar plus color add")) {
                env->PopLocalFrame(nullptr);
                return false;
            }
            env->PopLocalFrame(nullptr);
        }
    }

    jint stagedCount = collectionSize(env, stagedColors);
    jobject selectedColor = nullptr;
    if (requestedColor.has_value() && stagedCount > 0) {
        jclass colorValueClass = env->FindClass("com/lunarclient/common/v1/Color");
        jmethodID getColorValue = colorValueClass
            ? env->GetMethodID(colorValueClass, "getColor", "()I") : nullptr;
        jmethodID listSize = listClass ? env->GetMethodID(listClass, "size", "()I") : nullptr;
        if (clearException(env, "lunar plus requested color methods") || !colorValueClass ||
            !getColorValue || !listSize) {
            if (colorValueClass) env->DeleteLocalRef(colorValueClass);
            return false;
        }
        const jint count = env->CallIntMethod(stagedColors, listSize);
        for (jint i = 0; i < count && !selectedColor; ++i) {
            jobject candidate = env->CallObjectMethod(stagedColors, listGet, i);
            const jint value = candidate ? env->CallIntMethod(candidate, getColorValue) : -1;
            if (!clearException(env, "lunar plus requested color read") &&
                value == requestedColor.value()) {
                selectedColor = candidate;
            } else if (candidate) {
                env->DeleteLocalRef(candidate);
            }
        }
        env->DeleteLocalRef(colorValueClass);
    }
    if (!selectedColor && requestedColor.has_value()) {
        if (env->PushLocalFrame(8) != JNI_OK) return false;
        jobject colorBuilder = env->CallStaticObjectMethod(colorClass, newColorBuilder);
        jclass colorBuilderClass = colorBuilder ? env->GetObjectClass(colorBuilder) : nullptr;
        jmethodID setColor = colorBuilderClass
            ? env->GetMethodID(colorBuilderClass, "setColor",
                               (std::string("(I)") + kColorBuilderDescriptor).c_str()) : nullptr;
        jmethodID buildColor = colorBuilderClass
            ? env->GetMethodID(colorBuilderClass, "build",
                               (std::string("()") + kColorDescriptor).c_str()) : nullptr;
        jobject chained = (colorBuilder && setColor)
            ? env->CallObjectMethod(colorBuilder, setColor, requestedColor.value()) : nullptr;
        jobject generated = (colorBuilder && buildColor)
            ? env->CallObjectMethod(colorBuilder, buildColor) : nullptr;
        if (chained) env->DeleteLocalRef(chained);
        if (!generated || clearException(env, "lunar plus requested color build")) {
            env->PopLocalFrame(nullptr);
            return false;
        }
        env->CallBooleanMethod(stagedColors, add, generated);
        if (clearException(env, "lunar plus requested color add")) {
            env->PopLocalFrame(nullptr);
            return false;
        }
        // Promote the generated object out of the temporary local frame.
        selectedColor = static_cast<jobject>(env->PopLocalFrame(generated));
        ++stagedCount;
    }
    if (!selectedColor && stagedCount > 0) {
        selectedColor = env->CallObjectMethod(stagedColors, listGet, 0);
    }
    jobject chainedClear = env->CallObjectMethod(loginBuilder, clearColors);
    jobject chainedAdd = env->CallObjectMethod(loginBuilder, addAllColors, stagedColors);
    jobject chainedColor = selectedColor
        ? env->CallObjectMethod(loginBuilder, setPlusColor, selectedColor)
        : nullptr;
    jobject patched = env->CallObjectMethod(loginBuilder, buildLogin);
    if (chainedClear) env->DeleteLocalRef(chainedClear);
    if (chainedAdd) env->DeleteLocalRef(chainedAdd);
    if (chainedColor) env->DeleteLocalRef(chainedColor);
    if (clearException(env, "lunar plus response build") || !patched) return false;

    env->CallVoidMethod(manager, handleLogin, patched);
    if (clearException(env, "lunar plus response commit")) return false;
    replaceGlobalRef(env, g_persistentLoginResponse, patched);
    if (refreshProvider) {
        env->CallVoidMethod(manager, refreshProvider);
        if (clearException(env, "lunar plus provider refresh")) return false;
    } else {
        logResolutionOnce("lunar_plus_provider_refresh", "skipped", "()V");
    }
    jobject verifiedColors = env->CallObjectMethod(manager, getColors);
    const jint verified = collectionSize(env, verifiedColors);
    const jboolean active = env->CallBooleanMethod(manager, isActive);
    const bool failed = clearException(env, "lunar plus verify");
    const bool valid = !failed && active == JNI_TRUE && stagedCount > 0 && verified == stagedCount;
    logLine("LUNARPLUS_UNLOCK response_colors=" + std::to_string(responseColorCount) +
            " staged=" + std::to_string(stagedCount) +
            " available=" + std::to_string(verified) +
            " active=" + std::to_string(active == JNI_TRUE) +
            " valid=" + std::to_string(valid));
    return valid;
}

bool configureJvmti(jvmtiEnv* jvmti) {
    jvmtiCapabilities capabilities{};
    if (jvmti->GetCapabilities(&capabilities) != JVMTI_ERROR_NONE) return false;
    if (!capabilities.can_tag_objects) {
        jvmtiCapabilities requested{};
        requested.can_tag_objects = 1;
        const jvmtiError error = jvmti->AddCapabilities(&requested);
        if (error != JVMTI_ERROR_NONE) {
            logLine("JVMTI_CAPABILITY_FAILED error=" + jvmtiErrorName(jvmti, error));
            return false;
        }
    }
    return true;
}

void runAgent() {
    wchar_t modulePath[32768]{};
    GetModuleFileNameW(g_module, modulePath, static_cast<DWORD>(_countof(modulePath)));
    const std::wstring moduleName = std::filesystem::path(modulePath).stem().wstring();
    g_logPath = std::filesystem::path(modulePath).parent_path() / L"lunar_unlock_agent.log";
    g_statusPath = std::filesystem::path(modulePath).parent_path() /
        (L"lunar_unlock_status_" + std::to_wstring(GetCurrentProcessId()) +
         L"_" + moduleName + L".txt");

    // Scope duplicate-load protection to the concrete agent module.  This
    // keeps a stale debug build from suppressing a rebuilt agent during
    // in-process version testing while still preventing the same DLL from
    // starting two workers in one JVM.
    const std::wstring mutexName = L"Local\\ColdEternityTeam_LunarCosmetics_" +
        std::to_wstring(GetCurrentProcessId()) + L"_" + moduleName;
    HANDLE singleInstance = CreateMutexW(nullptr, TRUE, mutexName.c_str());
    if (!singleInstance || GetLastError() == ERROR_ALREADY_EXISTS) {
        if (singleInstance) CloseHandle(singleInstance);
        return;
    }

    logLine("AGENT_LOADED pid=" + std::to_string(GetCurrentProcessId()));
    writeStatus("RUNNING");
    initializeSelectionPath();
    g_hasSavedSelection = loadSelection(g_savedSelection);
    logLine(std::string("SELECTION_CONFIG loaded=") + (g_hasSavedSelection ? "1" : "0"));
    HMODULE jvmModule = nullptr;
    for (int i = 0; i < 100 && !jvmModule; ++i) {
        jvmModule = GetModuleHandleW(L"jvm.dll");
        if (!jvmModule) Sleep(100);
    }
    if (!jvmModule) {
        logLine("JVM_NOT_FOUND");
        writeStatus("FAILED JVM_NOT_FOUND");
        CloseHandle(singleInstance);
        return;
    }

    using GetCreatedVMs = jint(JNICALL*)(JavaVM**, jsize, jsize*);
    auto getCreatedVMs = reinterpret_cast<GetCreatedVMs>(
        GetProcAddress(jvmModule, "JNI_GetCreatedJavaVMs"));
    if (!getCreatedVMs) {
        logLine("JNI_EXPORT_NOT_FOUND");
        writeStatus("FAILED JNI_EXPORT_NOT_FOUND");
        CloseHandle(singleInstance);
        return;
    }

    JavaVM* vms[4]{};
    jsize vmCount = 0;
    if (getCreatedVMs(vms, 4, &vmCount) != JNI_OK || vmCount == 0) {
        logLine("JVM_INSTANCE_NOT_FOUND");
        writeStatus("FAILED JVM_INSTANCE_NOT_FOUND");
        CloseHandle(singleInstance);
        return;
    }

    JNIEnv* env = nullptr;
    bool attached = false;
    jint envResult = vms[0]->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_8);
    if (envResult == JNI_EDETACHED) {
        // The monitor must not keep the JVM alive after the game window closes.
        if (vms[0]->AttachCurrentThreadAsDaemon(
                reinterpret_cast<void**>(&env), nullptr) != JNI_OK) {
            logLine("JNI_ATTACH_FAILED");
            writeStatus("FAILED JNI_ATTACH_FAILED");
            CloseHandle(singleInstance);
            return;
        }
        attached = true;
    } else if (envResult != JNI_OK) {
        logLine("JNI_ENV_FAILED result=" + std::to_string(envResult));
        writeStatus("FAILED JNI_ENV_FAILED");
        CloseHandle(singleInstance);
        return;
    }

    jvmtiEnv* jvmti = nullptr;
    const jint jvmtiResult = vms[0]->GetEnv(
        reinterpret_cast<void**>(&jvmti), JVMTI_VERSION_1_2);
    if (jvmtiResult != JNI_OK || !jvmti || !configureJvmti(jvmti)) {
        logLine("JVMTI_ENV_FAILED result=" + std::to_string(jvmtiResult));
        writeStatus("FAILED JVMTI_ENV_FAILED");
        if (attached) vms[0]->DetachCurrentThread();
        CloseHandle(singleInstance);
        return;
    }
    logLine("JVM_ATTACHED vm_count=" + std::to_string(vmCount));

    bool unlocked = false;
    for (int attempt = 1; attempt <= 90 && !unlocked; ++attempt) {
        if (env->PushLocalFrame(512) != JNI_OK) {
            logLine("JNI_LOCAL_FRAME_FAILED");
            break;
        }

        jclass managerClass = findLoadedClass(env, jvmti, kManagerSignature);
        jclass responseClass = findLoadedClass(env, jvmti, kLoginResponseSignature);
        if (!managerClass || !responseClass) {
            if (attempt == 1 || attempt % 10 == 0) {
                logLine("WAITING_FOR_CLASSES attempt=" + std::to_string(attempt) +
                        " manager=" + (managerClass ? "1" : "0") +
                        " response=" + (responseClass ? "1" : "0"));
            }
            env->PopLocalFrame(nullptr);
            Sleep(1000);
            continue;
        }

        std::vector<jobject> managers = findInstances(env, jvmti, managerClass);
        std::vector<jobject> responses = findInstances(env, jvmti, responseClass);
        ManagerState selectedManager;
        for (jobject manager : managers) {
            ManagerState current = inspectManager(env, jvmti, manager);
            if (current.catalog > selectedManager.catalog) selectedManager = current;
        }
        ResponseCandidate response = selectResponse(env, responseClass, responses);

        if (attempt == 1 || attempt % 5 == 0 ||
            (selectedManager.catalog > 0 && response.object)) {
            logLine("RUNTIME_STATE attempt=" + std::to_string(attempt) +
                    " managers=" + std::to_string(managers.size()) +
                    " responses=" + std::to_string(responses.size()) +
                    " catalog=" + std::to_string(selectedManager.catalog) +
                    " owned=" + std::to_string(selectedManager.owned) +
                    " response_owned=" + std::to_string(response.owned) +
                    " outfits=" + std::to_string(response.outfits) +
                    " outfit_tree=" + std::to_string(response.hasOutfitTree == JNI_TRUE) +
                    " all_flag=" + std::to_string(response.hasAll == JNI_TRUE));
        }

        const bool responseIsLive = response.object &&
            (response.hasOutfitTree == JNI_TRUE || response.outfits > 0 || response.owned > 0);
        bool applied = false;
        const char* path = "none";
        if (selectedManager.manager && selectedManager.catalog > 0 && responseIsLive) {
            applied = applyUnlock(
                env, jvmti, selectedManager.manager, responseClass, response.object);
            path = "login_response";
        } else if (selectedManager.manager && selectedManager.catalog > 0) {
            applied = populateOwnedFromCatalog(env, jvmti, selectedManager.manager);
            path = "direct_catalog";
        }
        if (applied) {
            const ManagerState verified = inspectManager(
                env, jvmti, selectedManager.manager);
            const bool countLooksValid = verified.owned > 0 &&
                verified.ownedBySerial == verified.owned &&
                verified.owned >= (verified.catalog * 3) / 4;
            if (countLooksValid) {
                replaceGlobalRef(env, g_persistentCosmeticManager, selectedManager.manager);
                logLine("UNLOCK_SUCCESS catalog=" + std::to_string(verified.catalog) +
                        " owned=" + std::to_string(verified.owned) +
                        " owned_by_serial=" + std::to_string(verified.ownedBySerial) +
                        " path=" + path);
                unlocked = true;
            } else {
                logLine("UNLOCK_VERIFY_FAILED catalog=" + std::to_string(verified.catalog) +
                        " owned=" + std::to_string(verified.owned) +
                        " owned_by_serial=" + std::to_string(verified.ownedBySerial));
            }
        }

        env->PopLocalFrame(nullptr);
        if (!unlocked) Sleep(1000);
    }

    if (!unlocked) logLine("UNLOCK_TIMEOUT reason=no_live_login_response_or_catalog");

    bool emotesUnlocked = false;
    bool jamsUnlocked = false;
    bool spraysUnlocked = false;
    bool badgesUnlocked = false;
    bool lunarPlusUnlocked = false;
    for (int attempt = 1; attempt <= 90; ++attempt) {
        const bool extrasComplete = emotesUnlocked && jamsUnlocked && spraysUnlocked &&
            badgesUnlocked && lunarPlusUnlocked;
        if (extrasComplete) break;
        if (env->PushLocalFrame(2048) != JNI_OK) {
            logLine("EXTRA_LOCAL_FRAME_FAILED");
            break;
        }

        const bool shouldProbe = attempt == 1 || attempt % 3 == 0;
        if (shouldProbe && !emotesUnlocked) {
            emotesUnlocked = unlockEmotes(env, jvmti);
        }
        if (shouldProbe && !jamsUnlocked) {
            jamsUnlocked = unlockJams(env, jvmti);
        }
        if (shouldProbe && !spraysUnlocked) {
            spraysUnlocked = unlockSprays(env, jvmti);
        }
        if (shouldProbe && !badgesUnlocked) {
            badgesUnlocked = unlockBadges(env, jvmti);
        }
        if (shouldProbe && !lunarPlusUnlocked) {
            jclass responseClass = findLoadedClass(env, jvmti, kLoginResponseSignature);
            if (responseClass) {
                std::vector<jobject> responses = findInstances(env, jvmti, responseClass);
                ResponseCandidate response = selectResponse(env, responseClass, responses);
                if (response.object) {
                    lunarPlusUnlocked = unlockLunarPlus(
                        env, jvmti, responseClass, response.object,
                        g_hasSavedSelection ? g_savedSelection.lunarPlus : std::optional<jint>{});
                }
            }
        }

        if (attempt == 1 || attempt % 10 == 0 ||
            (emotesUnlocked && jamsUnlocked && spraysUnlocked &&
             badgesUnlocked && lunarPlusUnlocked)) {
            logLine("EXTRA_STATE attempt=" + std::to_string(attempt) +
                    " emotes=" + std::to_string(emotesUnlocked) +
                    " jams=" + std::to_string(jamsUnlocked) +
                    " sprays=" + std::to_string(spraysUnlocked) +
                    " badges=" + std::to_string(badgesUnlocked) +
                    " lunar_plus=" + std::to_string(lunarPlusUnlocked));
        }
        env->PopLocalFrame(nullptr);
        if (!(emotesUnlocked && jamsUnlocked && spraysUnlocked &&
              badgesUnlocked && lunarPlusUnlocked)) {
            Sleep(1000);
        }
    }

    const bool extrasComplete = emotesUnlocked && jamsUnlocked && spraysUnlocked &&
        badgesUnlocked && lunarPlusUnlocked;
    logLine("EXTRA_COMPLETE success=" + std::to_string(extrasComplete) +
            " emotes=" + std::to_string(emotesUnlocked) +
            " jams=" + std::to_string(jamsUnlocked) +
            " sprays=" + std::to_string(spraysUnlocked) +
            " badges=" + std::to_string(badgesUnlocked) +
            " lunar_plus=" + std::to_string(lunarPlusUnlocked));

    if (env->PushLocalFrame(256) == JNI_OK) {
        bindLocalPlayerUuid(env, jvmti);
        bindPlayerCosmeticState(env, jvmti);
        bool restored = false;
        if (g_hasSavedSelection) {
            restored = restoreSelection(env, g_savedSelection);
            logLine(restored ? "SELECTION_RESTORE_COMPLETE" : "SELECTION_RESTORE_INCOMPLETE");
        } else if (g_persistentCosmeticManager && g_persistentLocalPlayerUuid) {
            // A clean install starts with no equipped Cosmetics. The catalog
            // remains fully unlocked, so the user can choose items manually.
            restored = restoreManagerCosmeticSelection(
                env, g_persistentCosmeticManager, {});
            const bool playerRestored = restoreLocalPlayerCosmeticSelection(
                env, g_persistentCosmeticManager, {});
            if (g_persistentPlayerCosmeticState) {
                const bool previewRestored = restorePlayerCosmeticSelection(
                    env, g_persistentCosmeticManager,
                    g_persistentPlayerCosmeticState, {});
                logLine("LOADOUT_PREVIEW_INITIAL_RESET valid=" +
                        std::to_string(previewRestored));
            }
            g_localCosmeticSelectionInitialized = playerRestored;
            restored = playerRestored && restored;
            logLine(restored ? "SELECTION_INITIAL_RESET" : "SELECTION_INITIAL_RESET_FAILED");
        }
        env->PopLocalFrame(nullptr);
    } else {
        logLine("SELECTION_LOCAL_FRAME_FAILED reason=RESTORE");
    }

    // Keep the worker attached while the game is running so changes made in
    // the Locker UI are persisted without requiring another injection.
    const bool complete = unlocked && extrasComplete;
    logLine("AGENT_COMPLETE success=" + std::to_string(complete));
    writeStatus(complete ? "SUCCESS" : "FAILED UNLOCK_INCOMPLETE");
    monitorSelection(env, jvmti);
    if (attached) vms[0]->DetachCurrentThread();
    CloseHandle(singleInstance);
}

DWORD WINAPI worker(void*) {
    runAgent();
    return 0;
}

}  // namespace

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_module = module;
        DisableThreadLibraryCalls(module);
        HANDLE thread = CreateThread(nullptr, 0, worker, nullptr, 0, nullptr);
        if (thread) CloseHandle(thread);
    }
    return TRUE;
}
