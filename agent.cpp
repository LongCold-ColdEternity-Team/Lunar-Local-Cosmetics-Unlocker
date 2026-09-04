#include <windows.h>
#include <jni.h>
#include <jvmti.h>

#include <atomic>
#include <climits>
#include <filesystem>
#include <fstream>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace {

constexpr char kManagerSignature[] =
    "Lcom/moonsworth/lunar/client/HIOHRIRIOIHOICCCCOHCOOIICIICOH/"
    "OHOORCOHRORRIHROOCROOIIHHOOHRI/OIHRRCOOHRHIHICHRICIRIOCROHOCC;";
constexpr char kLoginResponseSignature[] =
    "Lcom/lunarclient/websocket/cosmetic/v2/LoginResponse;";
constexpr char kLoginResponseDescriptor[] =
    "Lcom/lunarclient/websocket/cosmetic/v2/LoginResponse;";
constexpr char kLoginResponseBuilderDescriptor[] =
    "Lcom/lunarclient/websocket/cosmetic/v2/LoginResponse$Builder;";
constexpr char kOwnedCosmeticDescriptor[] =
    "Lcom/moonsworth/lunar/client/RROCOHOCHIHOIOIOROCORHRHRCIRHI/"
    "RROHOCRCCHCIORHICIHCOHRIRCIHCI/COOCHIRORIICRCIIRIROHIIRIRICCH;";
constexpr char kCatalogCosmeticDescriptor[] =
    "Lcom/moonsworth/lunar/client/RROCOHOCHIHOIOIOROCORHRHRCIRHI/"
    "RROHOCRCCHCIORHICIHCOHRIRCIHCI/RROHOCRCCHCIORHICIHCOHRIRCIHCI;";
constexpr char kCosmeticTypeDescriptor[] =
    "Lcom/moonsworth/lunar/client/RROCOHOCHIHOIOIOROCORHRHRCIRHI/"
    "RROHOCRCCHCIORHICIHCOHRIRCIHCI/RORCICOHRIOIIIOIOHIIIICRCIHRII;";
constexpr char kItemMaterialDescriptor[] =
    "Lcom/moonsworth/lunar/client/OCCHHCHCOICROCHHIOCRRIIORHIOOH/"
    "COOCHIRORIICRCIIRIROHIIRIRICCH/COOCHIRORIICRCIIRIROHIIRIRICCH/"
    "OIHRRCOOHRHIHICHRICIRIOCROHOCC;";
constexpr char kOwnedMetadataDescriptor[] =
    "Lcom/moonsworth/lunar/client/RROCOHOCHIHOIOIOROCORHRHRCIRHI/"
    "RROHOCRCCHCIORHICIHCOHRIRCIHCI/OHRORRHCHCRICORRRCIRIOCCIRCRII;";
constexpr char kLocalCosmeticDescriptor[] =
    "Lcom/moonsworth/lunar/client/RROCOHOCHIHOIOIOROCORHRHRCIRHI/"
    "RROHOCRCCHCIORHICIHCOHRIRCIHCI/IRCROOHIIOOHIRCRRHOHIORHCOHIOR;";

constexpr char kEmoteManagerSignature[] =
    "Lcom/moonsworth/lunar/client/HIOHRIRIOIHOICCCCOHCOOIICIICOH/"
    "OHOORCOHRORRIHROOCROOIIHHOOHRI/COCCCORHOHRORRICOOHOCHOOCHHICR;";
constexpr char kJamManagerSignature[] =
    "Lcom/moonsworth/lunar/client/HIOHRIRIOIHOICCCCOHCOOIICIICOH/"
    "COCCCORHOHRORRICOOHOCHOOCHHICR/RROHOCRCCHCIORHICIHCOHRIRCIHCI;";
constexpr char kSprayManagerSignature[] =
    "Lcom/moonsworth/lunar/client/HIOHRIRIOIHOICCCCOHCOOIICIICOH/"
    "OHOORCOHRORRIHROOCROOIIHHOOHRI/HORCRCHHIRHRRHROOIOCRCICCICOCH;";
constexpr char kBadgeManagerSignature[] =
    "Lcom/moonsworth/lunar/client/HIOHRIRIOIHOICCCCOHCOOIICIICOH/"
    "OHOORCOHRORRIHROOCROOIIHHOOHRI/OHRORRHCHCRICORRRCIRIOCCIRCRII;";
constexpr char kLunarPlusManagerSignature[] =
    "Lcom/moonsworth/lunar/client/HIOHRIRIOIHOICCCCOHCOOIICIICOH/"
    "OHOORCOHRORRIHROOCROOIIHHOOHRI/HIOHRIRIOIHOICCCCOHCOOIICIICOH;";

constexpr char kEmoteOwnedWrapperSignature[] =
    "Lcom/moonsworth/lunar/client/RROCOHOCHIHOIOIOROCORHRHRCIRHI/"
    "RORCICOHRIOIIIOIOHIIIICRCIHRII/RORCICOHRIOIIIOIOHIIIICRCIHRII;";
constexpr char kEmoteOwnedWrapperDescriptor[] =
    "Lcom/moonsworth/lunar/client/RROCOHOCHIHOIOIOROCORHRHRCIRHI/"
    "RORCICOHRIOIIIOIOHIIIICRCIHRII/RORCICOHRIOIIIOIOHIIIICRCIHRII;";
constexpr char kEquippedEmoteWrapperSignature[] =
    "Lcom/moonsworth/lunar/client/OHOORCOHRORRIHROOCROOIIHHOOHRI/"
    "ORCOOOCHIICOIICCROCRCOOROORIHR;";
constexpr char kEmoteMetadataDescriptor[] =
    "Lcom/moonsworth/lunar/client/RROCOHOCHIHOIOIOROCORHRHRCIRHI/"
    "RROHOCRCCHCIORHICIHCOHRIRCIHCI/OHRORRHCHCRICORRRCIRIOCCIRCRII;";

constexpr char kOwnedJamSignature[] = "Lcom/lunarclient/websocket/jam/v1/OwnedJam;";
constexpr char kOwnedJamDescriptor[] = "Lcom/lunarclient/websocket/jam/v1/OwnedJam;";
constexpr char kOwnedJamBuilderDescriptor[] =
    "Lcom/lunarclient/websocket/jam/v1/OwnedJam$Builder;";

constexpr char kBadgeWrapperSignature[] =
    "Lcom/moonsworth/lunar/client/RORCICOHRIOIIIOIOHIIIICRCIHRII/"
    "RROHOCRCCHCIORHICIHCOHRIRCIHCI;";
constexpr char kBadgeWrapperDescriptor[] =
    "Lcom/moonsworth/lunar/client/RORCICOHRIOIIIOIOHIIIICRCIHRII/"
    "RROHOCRCCHCIORHICIHCOHRIRCIHCI;";
constexpr char kBadgeMetadataDescriptor[] =
    "Lcom/moonsworth/lunar/client/RORCICOHRIOIIIOIOHIIIICRCIHRII/"
    "COOCHIRORIICRCIIRIROHIIRIRICCH;";

constexpr char kColorSignature[] = "Lcom/lunarclient/common/v1/Color;";
constexpr char kColorDescriptor[] = "Lcom/lunarclient/common/v1/Color;";
constexpr char kColorBuilderDescriptor[] = "Lcom/lunarclient/common/v1/Color$Builder;";
constexpr char kEquippedSpraySignature[] = "Lcom/lunarclient/websocket/spray/v1/EquippedSpray;";
constexpr char kEquippedSprayDescriptor[] = "Lcom/lunarclient/websocket/spray/v1/EquippedSpray;";
constexpr char kEquippedSprayBuilderDescriptor[] =
    "Lcom/lunarclient/websocket/spray/v1/EquippedSpray$Builder;";

HMODULE g_module = nullptr;
std::filesystem::path g_logPath;
std::filesystem::path g_selectionPath;
std::mutex g_logMutex;
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

void logLine(const std::string& line) {
    std::lock_guard<std::mutex> guard(g_logMutex);
    std::ofstream output(g_logPath, std::ios::app);
    if (output) output << line << '\n';
    OutputDebugStringA(("[lunar_unlock_agent] " + line + "\n").c_str());
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

ManagerState inspectManager(JNIEnv* env, jobject manager) {
    ManagerState state;
    state.manager = manager;
    jclass managerClass = env->GetObjectClass(manager);
    if (!managerClass) {
        clearException(env, "manager class");
        return state;
    }

    jmethodID catalogAccessor = env->GetMethodID(
        managerClass, "IRRRHICIORORRRORRIOIHIORIHCOIH", "()Ljava/util/Map;");
    jmethodID ownedAccessor = env->GetMethodID(
        managerClass, "OHCHOCIROCIRRIIHHHOOIRCCIIRORO", "()Ljava/util/Set;");
    jmethodID serialAccessor = env->GetMethodID(
        managerClass, "RIHCRCROCIHOCHHCOIIOHHOHICHHCO", "()Ljava/util/Map;");
    if (clearException(env, "manager accessors") ||
        !catalogAccessor || !ownedAccessor || !serialAccessor) {
        env->DeleteLocalRef(managerClass);
        return state;
    }

    jobject catalog = env->CallObjectMethod(manager, catalogAccessor);
    jobject owned = env->CallObjectMethod(manager, ownedAccessor);
    jobject ownedBySerial = env->CallObjectMethod(manager, serialAccessor);
    if (!clearException(env, "manager collections")) {
        state.catalog = collectionSize(env, catalog);
        state.owned = collectionSize(env, owned);
        state.ownedBySerial = collectionSize(env, ownedBySerial);
    }
    if (catalog) env->DeleteLocalRef(catalog);
    if (owned) env->DeleteLocalRef(owned);
    if (ownedBySerial) env->DeleteLocalRef(ownedBySerial);
    env->DeleteLocalRef(managerClass);
    return state;
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

bool applyUnlock(JNIEnv* env, jobject manager, jclass responseClass, jobject response) {
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
        ? env->GetMethodID(managerClass, "RROHOCRCCHCIORHICIHCOHRIRCIHCI",
                           handlerDescriptor.c_str())
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

bool populateOwnedFromCatalog(JNIEnv* env, jobject manager) {
    jclass managerClass = env->GetObjectClass(manager);
    if (!managerClass) {
        clearException(env, "direct manager class");
        return false;
    }

    jmethodID catalogAccessor = env->GetMethodID(
        managerClass, "IRRRHICIORORRRORRIOIHIORIHCOIH", "()Ljava/util/Map;");
    jmethodID ownedAccessor = env->GetMethodID(
        managerClass, "OHCHOCIROCIRRIIHHHOOIRCCIIRORO", "()Ljava/util/Set;");
    jmethodID serialAccessor = env->GetMethodID(
        managerClass, "RIHCRCROCIHOCHHCOIIOHHOHICHHCO", "()Ljava/util/Map;");
    const std::string factoryDescriptor =
        std::string("(ILjava/lang/String;") + kCosmeticTypeDescriptor +
        "Ljava/lang/String;ZJLjava/time/Instant;Ljava/util/List;Ljava/util/List;Z" +
        kItemMaterialDescriptor + kOwnedMetadataDescriptor + ")" + kOwnedCosmeticDescriptor;
    jmethodID factory = env->GetMethodID(
        managerClass, "COOCHIRORIICRCIIRIROHIIRIRICCH", factoryDescriptor.c_str());
    if (clearException(env, "direct manager methods") ||
        !catalogAccessor || !ownedAccessor || !serialAccessor || !factory) {
        env->DeleteLocalRef(managerClass);
        return false;
    }

    jobject catalog = env->CallObjectMethod(manager, catalogAccessor);
    jobject targetOwned = env->CallObjectMethod(manager, ownedAccessor);
    jobject targetSerials = env->CallObjectMethod(manager, serialAccessor);
    if (clearException(env, "direct manager collections") ||
        !catalog || !targetOwned || !targetSerials) {
        if (catalog) env->DeleteLocalRef(catalog);
        if (targetOwned) env->DeleteLocalRef(targetOwned);
        if (targetSerials) env->DeleteLocalRef(targetSerials);
        env->DeleteLocalRef(managerClass);
        return false;
    }

    jclass mapClass = env->FindClass("java/util/Map");
    jclass collectionClass = env->FindClass("java/util/Collection");
    jclass iteratorClass = env->FindClass("java/util/Iterator");
    jclass optionalClass = env->FindClass("java/util/Optional");
    jclass hashSetClass = env->FindClass("java/util/HashSet");
    jclass hashMapClass = env->FindClass("java/util/HashMap");
    jclass longClass = env->FindClass("java/lang/Long");
    if (clearException(env, "direct Java collection classes") ||
        !mapClass || !collectionClass || !iteratorClass || !optionalClass ||
        !hashSetClass || !hashMapClass || !longClass) {
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
    jmethodID longValueOf = env->GetStaticMethodID(
        longClass, "valueOf", "(J)Ljava/lang/Long;");
    if (clearException(env, "direct collection methods") ||
        !mapValues || !mapClear || !mapPutAll || !mapPut || !collectionIterator ||
        !collectionClear || !collectionAdd || !collectionAddAll || !iteratorHasNext ||
        !iteratorNext || !optionalIsPresent || !optionalGet || !hashSetCtor ||
        !hashMapCtor || !longValueOf) {
        env->DeleteLocalRef(catalog);
        env->DeleteLocalRef(targetOwned);
        env->DeleteLocalRef(targetSerials);
        env->DeleteLocalRef(managerClass);
        return false;
    }

    jobject stagedOwned = env->NewObject(hashSetClass, hashSetCtor);
    jobject stagedSerials = env->NewObject(hashMapClass, hashMapCtor);
    jobject values = env->CallObjectMethod(catalog, mapValues);
    jobject iterator = values ? env->CallObjectMethod(values, collectionIterator) : nullptr;
    if (clearException(env, "direct staging setup") ||
        !stagedOwned || !stagedSerials || !values || !iterator) {
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
    jmethodID getVariant = nullptr;
    jmethodID getHidden = nullptr;
    jmethodID getTags = nullptr;
    jmethodID getColors = nullptr;
    jmethodID getAnimated = nullptr;
    jmethodID getMaterial = nullptr;
    jclass ownedClass = nullptr;
    jmethodID getSerial = nullptr;

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
            getId = env->GetMethodID(catalogClass, "getId", "()I");
            getName = env->GetMethodID(catalogClass, "getName", "()Ljava/lang/String;");
            getType = env->GetMethodID(
                catalogClass, "RCCCIIHHHRCIHIHIOCORCOCRICOCCC", "()Ljava/util/Optional;");
            getVariant = env->GetMethodID(
                catalogClass, "RRCHRIRHORICHRHRHRCHOIOICCHCCC", "()Ljava/lang/String;");
            getHidden = env->GetMethodID(
                catalogClass, "IHCRCCCIHCRCCRHRRIOIOHOHORHHRO", "()Z");
            getTags = env->GetMethodID(
                catalogClass, "HRRRHOIHCCOIIRRIRHIICIRHHCHIOC", "()Ljava/util/List;");
            getColors = env->GetMethodID(catalogClass, "getColors", "()Ljava/util/List;");
            getAnimated = env->GetMethodID(
                catalogClass, "ORRHHHHOCIRRCOIRIORCOOCRRIIIHI", "()Z");
            const std::string materialGetter = std::string("()") + kItemMaterialDescriptor;
            getMaterial = env->GetMethodID(
                catalogClass, "RRRICCCIRRRHIROCOHCRORORCOCRRC", materialGetter.c_str());
            if (clearException(env, "catalog item methods") ||
                !getId || !getName || !getType || !getVariant || !getHidden ||
                !getTags || !getColors || !getAnimated || !getMaterial) {
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
        jobject variant = env->CallObjectMethod(item, getVariant);
        const jboolean hidden = env->CallBooleanMethod(item, getHidden);
        jobject tags = env->CallObjectMethod(item, getTags);
        jobject colors = env->CallObjectMethod(item, getColors);
        const jboolean animated = env->CallBooleanMethod(item, getAnimated);
        jobject material = env->CallObjectMethod(item, getMaterial);
        if (clearException(env, "catalog item values")) {
            env->PopLocalFrame(nullptr);
            failed = true;
            break;
        }

        jobject owned = env->CallObjectMethod(
            manager, factory, id, name, type, variant, hidden, static_cast<jlong>(-1),
            nullptr, tags, colors, animated, material, nullptr);
        if (clearException(env, "owned cosmetic factory") || !owned) {
            env->PopLocalFrame(nullptr);
            failed = true;
            break;
        }

        if (!ownedClass) {
            ownedClass = static_cast<jclass>(env->NewGlobalRef(env->GetObjectClass(owned)));
            getSerial = env->GetMethodID(
                ownedClass, "OCRIOHIIOCCCHOOCIRORIRHIRRCHCH", "()J");
            if (clearException(env, "owned serial method") || !getSerial) {
                env->PopLocalFrame(nullptr);
                failed = true;
                break;
            }
        }

        const jlong serial = env->CallLongMethod(owned, getSerial);
        jobject boxedSerial = env->CallStaticObjectMethod(longClass, longValueOf, serial);
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
    if (ownedClass) env->DeleteGlobalRef(ownedClass);
    env->DeleteLocalRef(iterator);
    env->DeleteLocalRef(values);
    env->DeleteLocalRef(stagedOwned);
    env->DeleteLocalRef(stagedSerials);
    env->DeleteLocalRef(catalog);
    env->DeleteLocalRef(targetOwned);
    env->DeleteLocalRef(targetSerials);
    env->DeleteLocalRef(mapClass);
    env->DeleteLocalRef(collectionClass);
    env->DeleteLocalRef(iteratorClass);
    env->DeleteLocalRef(optionalClass);
    env->DeleteLocalRef(hashSetClass);
    env->DeleteLocalRef(hashMapClass);
    env->DeleteLocalRef(longClass);
    env->DeleteLocalRef(managerClass);
    return stagingValid && !failed;
}

jobject firstInstance(JNIEnv* env, jvmtiEnv* jvmti, jclass klass) {
    std::vector<jobject> instances = findInstances(env, jvmti, klass);
    return instances.empty() ? nullptr : instances.front();
}


bool bindPlayerCosmeticState(JNIEnv* env, jvmtiEnv* jvmti) {
    if (g_persistentPlayerCosmeticState) return true;
    constexpr char kPlayerStateSignature[] =
        "Lcom/moonsworth/lunar/client/OHOORCCRHHHHROIICHOCRHOCRICROH/"
        "RROHOCRCCHCIORHICIHCOHRIRCIHCI/OHOORCOHRORRIHROOCROOIIHHOOHRI/"
        "RROHOCRCCHCIORHICIHCOHRIRCIHCI/COOCHIRORIICRCIIRIROHIIRIRICCH;";
    jclass stateClass = findLoadedClass(env, jvmti, kPlayerStateSignature);
    if (!stateClass) return false;
    std::vector<jobject> states = findInstances(env, jvmti, stateClass);
    if (states.empty()) {
        env->DeleteLocalRef(stateClass);
        return false;
    }
    // In the 1.8.9 client the local player state is created first.  Prefer a
    // state with a non-empty cosmetic list when several remote states exist.
    jobject selected = states.front();
    jint selectedSize = -1;
    for (jobject state : states) {
        jobject cosmetics = invokeNoArgReflective(
            env, state, "RRRIRRICOOHRIIOOIOCHCIIIIRHIIR", "player state bind list");
        const jint size = collectionSize(env, cosmetics);
        if (size > selectedSize) {
            selected = state;
            selectedSize = size;
        }
        if (cosmetics) env->DeleteLocalRef(cosmetics);
    }
    replaceGlobalRef(env, g_persistentPlayerCosmeticState, selected);
    logLine("PLAYER_COSMETIC_STATE_BOUND size=" + std::to_string(selectedSize));
    for (jobject state : states) env->DeleteLocalRef(state);
    env->DeleteLocalRef(stateClass);
    return g_persistentPlayerCosmeticState != nullptr;
}

bool capturePlayerCosmeticSelection(JNIEnv* env, jobject state,
                                    std::set<jlong>& selection) {
    selection.clear();
    if (!state) return false;
    jobject equipped = invokeNoArgReflective(
        env, state, "RRRIRRICOOHRIIOOIOCHCIIIIRHIIR", "player selection list");
    jclass collectionClass = env->FindClass("java/util/Collection");
    jclass iteratorClass = env->FindClass("java/util/Iterator");
    jmethodID iteratorMethod = collectionClass
        ? env->GetMethodID(collectionClass, "iterator", "()Ljava/util/Iterator;") : nullptr;
    jmethodID hasNext = iteratorClass
        ? env->GetMethodID(iteratorClass, "hasNext", "()Z") : nullptr;
    jmethodID next = iteratorClass
        ? env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;") : nullptr;
    if (clearException(env, "player selection roots") || !equipped ||
        !collectionClass || !iteratorClass || !iteratorMethod || !hasNext || !next) {
        if (equipped) env->DeleteLocalRef(equipped);
        if (collectionClass) env->DeleteLocalRef(collectionClass);
        if (iteratorClass) env->DeleteLocalRef(iteratorClass);
        return false;
    }
    jobject cursor = env->CallObjectMethod(equipped, iteratorMethod);
    bool failed = clearException(env, "player selection iterator") || !cursor;
    while (!failed && env->CallBooleanMethod(cursor, hasNext) == JNI_TRUE) {
        if (env->PushLocalFrame(16) != JNI_OK) {
            failed = true;
            break;
        }
        jobject wrapper = env->CallObjectMethod(cursor, next);
        jobject owned = wrapper
            ? invokeNoArgReflective(env, wrapper, "IOIRCCICHROHOIHRCORROCRHRRIHCH",
                                    "player selection owned") : nullptr;
        const auto id = invokeLongNoArgReflective(
            env, owned ? owned : wrapper,
            "OCRIOHIIOCCCHOOCIRORIRHIRRCHCH", "player selection id");
        if (clearException(env, "player selection item")) {
            failed = true;
        } else if (id.has_value() && id.value() >= 0) {
            selection.insert(id.value());
        }
        env->PopLocalFrame(nullptr);
    }
    if (!failed && clearException(env, "player selection completion")) failed = true;
    if (cursor) env->DeleteLocalRef(cursor);
    env->DeleteLocalRef(equipped);
    env->DeleteLocalRef(collectionClass);
    env->DeleteLocalRef(iteratorClass);
    return !failed;
}

bool restorePlayerCosmeticSelection(JNIEnv* env, jobject manager, jobject state,
                                    const std::set<jlong>& selection) {
    if (!manager || !state) return false;
    jobject equipped = invokeNoArgReflective(
        env, state, "RRRIRRICOOHRIIOOIOCHCIIIIRHIIR", "player restore list");
    jclass collectionClass = env->FindClass("java/util/Collection");
    jclass managerClass = env->GetObjectClass(manager);
    jmethodID clear = collectionClass
        ? env->GetMethodID(collectionClass, "clear", "()V") : nullptr;
    jmethodID add = collectionClass
        ? env->GetMethodID(collectionClass, "add", "(Ljava/lang/Object;)Z") : nullptr;
    jmethodID factory = managerClass
        ? env->GetMethodID(managerClass, "IRCCRCOOHRIHOOCRRHIHCIHRRCHHHO",
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
        managerClass, "CORHICICCROHRHCOOCORRCOOCRRICO", "()V");
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

bool captureCosmeticSelection(JNIEnv* env, jobject manager, std::set<jlong>& selection) {
    selection.clear();
    if (!manager) return false;
    jclass managerClass = env->GetObjectClass(manager);
    jmethodID ownedAccessor = managerClass ? env->GetMethodID(
        managerClass, "OHCHOCIROCIRRIIHHHOOIRCCIIRORO", "()Ljava/util/Set;") : nullptr;
    jobject owned = ownedAccessor ? env->CallObjectMethod(manager, ownedAccessor) : nullptr;
    jclass collectionClass = env->FindClass("java/util/Collection");
    jclass iteratorClass = env->FindClass("java/util/Iterator");
    jmethodID iteratorMethod = collectionClass
        ? env->GetMethodID(collectionClass, "iterator", "()Ljava/util/Iterator;") : nullptr;
    jmethodID hasNext = iteratorClass ? env->GetMethodID(iteratorClass, "hasNext", "()Z") : nullptr;
    jmethodID next = iteratorClass
        ? env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;") : nullptr;
    if (clearException(env, "cosmetic selection roots") || !managerClass || !owned ||
        !collectionClass || !iteratorClass || !iteratorMethod || !hasNext || !next) {
        if (managerClass) env->DeleteLocalRef(managerClass);
        if (owned) env->DeleteLocalRef(owned);
        return false;
    }

    jobject cursor = env->CallObjectMethod(owned, iteratorMethod);
    bool failed = clearException(env, "cosmetic selection iterator") || !cursor;
    while (!failed && env->CallBooleanMethod(cursor, hasNext) == JNI_TRUE) {
        if (env->PushLocalFrame(12) != JNI_OK) {
            failed = true;
            break;
        }
        jobject item = env->CallObjectMethod(cursor, next);
        jclass itemClass = item ? env->GetObjectClass(item) : nullptr;
        jmethodID active = itemClass
            ? env->GetMethodID(itemClass, "CIHHOHHROOCCHHRRCRORCHCOOHCHIH", "()Z") : nullptr;
        jmethodID id = itemClass
            ? env->GetMethodID(itemClass, "OCRIOHIIOCCCHOOCIRORIRHIRRCHCH", "()J") : nullptr;
        const jboolean isActive = (item && active)
            ? env->CallBooleanMethod(item, active) : JNI_FALSE;
        const jlong cosmeticId = (item && id)
            ? env->CallLongMethod(item, id) : static_cast<jlong>(-1);
        if (clearException(env, "cosmetic selection item")) {
            failed = true;
        } else if (isActive == JNI_TRUE && cosmeticId >= 0) {
            selection.insert(cosmeticId);
        }
        env->PopLocalFrame(nullptr);
    }
    if (!failed && clearException(env, "cosmetic selection completion")) failed = true;
    if (cursor) env->DeleteLocalRef(cursor);
    env->DeleteLocalRef(collectionClass);
    env->DeleteLocalRef(iteratorClass);
    env->DeleteLocalRef(managerClass);
    env->DeleteLocalRef(owned);
    return !failed;
}

bool restoreCosmeticSelection(JNIEnv* env, jobject manager, const std::set<jlong>& selection) {
    if (!manager) return false;
    jclass managerClass = env->GetObjectClass(manager);
    jmethodID ownedAccessor = managerClass ? env->GetMethodID(
        managerClass, "OHCHOCIROCIRRIIHHHOOIRCCIIRORO", "()Ljava/util/Set;") : nullptr;
    jobject owned = ownedAccessor ? env->CallObjectMethod(manager, ownedAccessor) : nullptr;
    jclass collectionClass = env->FindClass("java/util/Collection");
    jclass iteratorClass = env->FindClass("java/util/Iterator");
    jmethodID iteratorMethod = collectionClass
        ? env->GetMethodID(collectionClass, "iterator", "()Ljava/util/Iterator;") : nullptr;
    jmethodID hasNext = iteratorClass ? env->GetMethodID(iteratorClass, "hasNext", "()Z") : nullptr;
    jmethodID next = iteratorClass
        ? env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;") : nullptr;
    if (clearException(env, "cosmetic restore roots") || !managerClass || !owned ||
        !collectionClass || !iteratorClass || !iteratorMethod || !hasNext || !next) {
        if (managerClass) env->DeleteLocalRef(managerClass);
        if (owned) env->DeleteLocalRef(owned);
        return false;
    }

    jobject cursor = env->CallObjectMethod(owned, iteratorMethod);
    bool failed = clearException(env, "cosmetic restore iterator") || !cursor;
    size_t matched = 0;
    while (!failed && env->CallBooleanMethod(cursor, hasNext) == JNI_TRUE) {
        if (env->PushLocalFrame(12) != JNI_OK) {
            failed = true;
            break;
        }
        jobject item = env->CallObjectMethod(cursor, next);
        jclass itemClass = item ? env->GetObjectClass(item) : nullptr;
        jmethodID id = itemClass
            ? env->GetMethodID(itemClass, "OCRIOHIIOCCCHOOCIRORIRHIRRCHCH", "()J") : nullptr;
        jmethodID setActive = itemClass
            ? env->GetMethodID(itemClass, "HCRCIRHIOCOOIHIROORHCOOHIOHHCR", "(Z)V") : nullptr;
        const jlong cosmeticId = (item && id)
            ? env->CallLongMethod(item, id) : static_cast<jlong>(-1);
        const bool wanted = cosmeticId >= 0 && selection.find(cosmeticId) != selection.end();
        if (item && setActive) env->CallVoidMethod(item, setActive, wanted ? JNI_TRUE : JNI_FALSE);
        if (clearException(env, "cosmetic restore item")) {
            failed = true;
        } else if (wanted) {
            ++matched;
        }
        env->PopLocalFrame(nullptr);
    }
    if (!failed && clearException(env, "cosmetic restore completion")) failed = true;
    if (cursor) env->DeleteLocalRef(cursor);
    env->DeleteLocalRef(collectionClass);
    env->DeleteLocalRef(iteratorClass);
    env->DeleteLocalRef(managerClass);
    env->DeleteLocalRef(owned);
    logLine("COSMETIC_SELECTION_RESTORE requested=" + std::to_string(selection.size()) +
            " matched=" + std::to_string(matched) +
            " valid=" + std::to_string(!failed));
    return !failed;
}

bool captureEmoteSelection(JNIEnv* env, jobject manager, std::set<EmoteSelection>& selection) {
    selection.clear();
    if (!manager) return false;
    jclass managerClass = env->GetObjectClass(manager);
    jmethodID getEquipped = managerClass ? env->GetMethodID(
        managerClass, "CICIORCCIHCCRCIRCHCIOIRHIIHIOH", "()Ljava/util/Set;") : nullptr;
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
        managerClass, "CROHCOROCHIORIHRIIHHOROOIIOCHO", "Lcom/google/common/collect/BiMap;") : nullptr;
    jobject catalog = catalogField ? env->GetStaticObjectField(managerClass, catalogField) : nullptr;
    jmethodID setEquipped = managerClass ? env->GetMethodID(
        managerClass, "IROCROOIOIORORHHCCOOCCHRRRIOIC", "(Ljava/util/Set;)V") : nullptr;
    jmethodID findEquipped = managerClass ? env->GetMethodID(
        managerClass, "HOOOHOOHCIIROCCICRCRIICCRCIHOH", "(I)Ljava/util/Optional;") : nullptr;
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
        managerClass, "RCOCICCIHRROOHOHCCCRRHICOOCHHO", "()Ljava/util/Set;") : nullptr;
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
        managerClass, "COOIOIRCCICRCCIHCCRHRCIHROHCCI", "(Ljava/util/Set;)V") : nullptr;
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
    if (g_persistentPlayerCosmeticState && g_persistentCosmeticManager) {
        std::set<jlong> cosmetics;
        if (capturePlayerCosmeticSelection(
                env, g_persistentPlayerCosmeticState, cosmetics)) {
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
    if (g_persistentCosmeticManager && g_persistentPlayerCosmeticState) {
        valid = restorePlayerCosmeticSelection(
            env, g_persistentCosmeticManager, g_persistentPlayerCosmeticState,
            selection.cosmetics) && valid;
        // Keep the catalog wrappers in sync for Locker's selected-state UI;
        // rendering itself consumes the local player state list above.
        valid = restoreCosmeticSelection(
            env, g_persistentCosmeticManager, selection.cosmetics) && valid;
    } else {
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

void monitorSelection(JNIEnv* env) {
    SelectionState previous = g_hasSavedSelection ? g_savedSelection : SelectionState{};
    bool started = false;
    bool waitingLogged = false;
    bool lastComplete = true;

    for (;;) {
        if (env->PushLocalFrame(256) != JNI_OK) {
            logLine("SELECTION_LOCAL_FRAME_FAILED");
            Sleep(750);
            continue;
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
                started = true;
                lastComplete = true;
                logLine("SELECTION_MONITOR_STARTED interval_ms=750 complete=1");
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
                } else {
                    logLine("SELECTION_SAVE_FAILED reason=CHANGED");
                }
            }
        }
        env->PopLocalFrame(nullptr);
        Sleep(750);
    }
}

bool unlockEmotes(JNIEnv* env, jvmtiEnv* jvmti) {
    jclass managerClass = findLoadedClass(env, jvmti, kEmoteManagerSignature);
    jclass wrapperClass = findLoadedClass(env, jvmti, kEmoteOwnedWrapperSignature);
    jclass equippedWrapperClass = findLoadedClass(
        env, jvmti, kEquippedEmoteWrapperSignature);
    if (!managerClass || !wrapperClass || !equippedWrapperClass) return false;

    jobject manager = firstInstance(env, jvmti, managerClass);
    jfieldID catalogField = env->GetStaticFieldID(
        managerClass, "CROHCOROCHIORIHRIIHHOROOIIOCHO",
        "Lcom/google/common/collect/BiMap;");
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
    jmethodID setOwned = env->GetMethodID(
        managerClass, "OOHCHIRHROIORHHRHIOHIOHCCCCIIR", "(Ljava/util/List;)V");
    jmethodID getOwned = env->GetMethodID(
        managerClass, "CCRIOCOIRIHCRCIHIHROOOCHIOORIR", "()Ljava/util/List;");
    jmethodID setOwnsPlus = env->GetMethodID(
        managerClass, "RROOHRRHRHRCIRCIICROHRRRCROIIH", "(Z)V");
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

    jmethodID getCatalog = env->GetStaticMethodID(
        managerClass, "ORIHIIRRICRHRCCRRHIHHRIHIRHRRH", "()Ljava/util/Map;");
    jmethodID newBuilder = env->GetStaticMethodID(
        ownedClass, "newBuilder", (std::string("()") + kOwnedJamBuilderDescriptor).c_str());
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
    jmethodID setOwned = env->GetMethodID(
        managerClass, "CORCCCCORCIIRRCHCCHROOIIOICORC", "(Ljava/util/List;)V");
    jmethodID getOwned = env->GetMethodID(
        managerClass, "IHIHCHCOORIHHORRIORCHCRCCCIIOH", "()Ljava/util/List;");
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

    jmethodID getCatalog = env->GetMethodID(
        managerClass, "OOICHCCRRHIRHRRHHCIOHHCOIICHHC",
        "()Lit/unimi/dsi/fastutil/ints/Int2ObjectMap;");
    jmethodID getOwned = env->GetMethodID(
        managerClass, "OOROCCOIRCIROIHOCHHCOHCIIOCRHI",
        "()Lit/unimi/dsi/fastutil/objects/Object2LongMap;");
    jmethodID setOwned = env->GetMethodID(
        managerClass, "COOCHIRORIICRCIIRIROHIIRIRICCH",
        "(Lit/unimi/dsi/fastutil/objects/Object2LongMap;)V");
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

    jmethodID getCatalog = env->GetMethodID(
        managerClass, "IIOOHRCOOCIHRIHCCRRHCOROIOOHCR", "()Ljava/util/Map;");
    const std::string converterDescriptor =
        std::string("(") + kBadgeMetadataDescriptor + ")" + kBadgeWrapperDescriptor;
    jmethodID convert = env->GetStaticMethodID(
        wrapperClass, "COOCHIRORIICRCIIRIROHIIRIRICCH", converterDescriptor.c_str());
    jmethodID setOwned = env->GetMethodID(
        managerClass, "CHOOCRRRCRHRCOOOCIICORHHOIROOR", "(Ljava/util/List;)V");
    jfieldID ownedField = env->GetFieldID(
        managerClass, "HIRRHOHIOOCCCIHRIHCHROICOROICH", "Ljava/util/List;");
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
    jmethodID handleLogin = env->GetMethodID(
        managerClass, "RORCICOHRIOIIIOIOHIIIICRCIHRII",
        (std::string("(") + kLoginResponseDescriptor + ")V").c_str());
    jmethodID getColors = env->GetMethodID(
        managerClass, "OHCHOCIROCIRRIIHHHOOIRCCIIRORO", "()Ljava/util/Set;");
    jmethodID isActive = env->GetMethodID(
        managerClass, "RHICHOCCRORRIIHHCHICIRRORHHCCO", "()Z");
    jmethodID refreshProvider = env->GetMethodID(
        managerClass, "CHOHIHHIROOCORRCOORHRCIOIIOOCO", "()V");
    if (clearException(env, "lunar plus methods") || !available || !loginBuilder ||
        !listCtor || !add || !addAll || !listGet || !clearColors || !addAllColors ||
        !setPlusColor || !buildLogin || !handleLogin || !getColors || !isActive ||
        !refreshProvider) {
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
    env->CallVoidMethod(manager, refreshProvider);
    if (clearException(env, "lunar plus provider refresh")) return false;
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
    g_logPath = std::filesystem::path(modulePath).parent_path() / L"lunar_unlock_agent.log";

    // Scope duplicate-load protection to the concrete agent module.  This
    // keeps a stale debug build from suppressing a rebuilt agent during
    // in-process version testing while still preventing the same DLL from
    // starting two workers in one JVM.
    const std::wstring moduleName = std::filesystem::path(modulePath).stem().wstring();
    const std::wstring mutexName = L"Local\\ColdEternityTeam_LunarCosmetics_" +
        std::to_wstring(GetCurrentProcessId()) + L"_" + moduleName;
    HANDLE singleInstance = CreateMutexW(nullptr, TRUE, mutexName.c_str());
    if (!singleInstance || GetLastError() == ERROR_ALREADY_EXISTS) {
        if (singleInstance) CloseHandle(singleInstance);
        return;
    }

    logLine("AGENT_LOADED pid=" + std::to_string(GetCurrentProcessId()));
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
        CloseHandle(singleInstance);
        return;
    }

    using GetCreatedVMs = jint(JNICALL*)(JavaVM**, jsize, jsize*);
    auto getCreatedVMs = reinterpret_cast<GetCreatedVMs>(
        GetProcAddress(jvmModule, "JNI_GetCreatedJavaVMs"));
    if (!getCreatedVMs) {
        logLine("JNI_EXPORT_NOT_FOUND");
        CloseHandle(singleInstance);
        return;
    }

    JavaVM* vms[4]{};
    jsize vmCount = 0;
    if (getCreatedVMs(vms, 4, &vmCount) != JNI_OK || vmCount == 0) {
        logLine("JVM_INSTANCE_NOT_FOUND");
        CloseHandle(singleInstance);
        return;
    }

    JNIEnv* env = nullptr;
    bool attached = false;
    jint envResult = vms[0]->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_8);
    if (envResult == JNI_EDETACHED) {
        if (vms[0]->AttachCurrentThread(reinterpret_cast<void**>(&env), nullptr) != JNI_OK) {
            logLine("JNI_ATTACH_FAILED");
            CloseHandle(singleInstance);
            return;
        }
        attached = true;
    } else if (envResult != JNI_OK) {
        logLine("JNI_ENV_FAILED result=" + std::to_string(envResult));
        CloseHandle(singleInstance);
        return;
    }

    jvmtiEnv* jvmti = nullptr;
    const jint jvmtiResult = vms[0]->GetEnv(
        reinterpret_cast<void**>(&jvmti), JVMTI_VERSION_1_2);
    if (jvmtiResult != JNI_OK || !jvmti || !configureJvmti(jvmti)) {
        logLine("JVMTI_ENV_FAILED result=" + std::to_string(jvmtiResult));
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
            ManagerState current = inspectManager(env, manager);
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
            applied = applyUnlock(env, selectedManager.manager, responseClass, response.object);
            path = "login_response";
        } else if (selectedManager.manager && selectedManager.catalog > 0) {
            applied = populateOwnedFromCatalog(env, selectedManager.manager);
            path = "direct_catalog";
        }
        if (applied) {
            const ManagerState verified = inspectManager(env, selectedManager.manager);
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
        bindPlayerCosmeticState(env, jvmti);
        bool restored = false;
        if (g_hasSavedSelection) {
            restored = restoreSelection(env, g_savedSelection);
            logLine(restored ? "SELECTION_RESTORE_COMPLETE" : "SELECTION_RESTORE_INCOMPLETE");
        } else if (g_persistentCosmeticManager && g_persistentPlayerCosmeticState) {
            // A clean install starts with no equipped Cosmetics. The catalog
            // remains fully unlocked, so the user can choose items manually.
            restored = restorePlayerCosmeticSelection(
                env, g_persistentCosmeticManager, g_persistentPlayerCosmeticState, {});
            logLine(restored ? "SELECTION_INITIAL_RESET" : "SELECTION_INITIAL_RESET_FAILED");
        }
        env->PopLocalFrame(nullptr);
    } else {
        logLine("SELECTION_LOCAL_FRAME_FAILED reason=RESTORE");
    }

    // Keep the worker attached while the game is running so changes made in
    // the Locker UI are persisted without requiring another injection.
    logLine("AGENT_COMPLETE success=" + std::to_string(unlocked && extrasComplete));
    monitorSelection(env);
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
