#include <windows.h>
#include <jni.h>
#include <jvmti.h>

#include <atomic>
#include <filesystem>
#include <fstream>
#include <mutex>
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

HMODULE g_module = nullptr;
std::filesystem::path g_logPath;
std::mutex g_logMutex;
std::atomic<unsigned long> g_tagSequence{1};

void logLine(const std::string& line) {
    std::lock_guard<std::mutex> guard(g_logMutex);
    std::ofstream output(g_logPath, std::ios::app);
    if (output) output << line << '\n';
    OutputDebugStringA(("[lunar_unlock_agent] " + line + "\n").c_str());
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

bool unlockEmotes(JNIEnv* env, jvmtiEnv* jvmti) {
    jclass managerClass = findLoadedClass(env, jvmti, kEmoteManagerSignature);
    jclass wrapperClass = findLoadedClass(env, jvmti, kEmoteOwnedWrapperSignature);
    if (!managerClass || !wrapperClass) return false;

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
                     jclass responseClass, jobject response) {
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

    const jint stagedCount = collectionSize(env, stagedColors);
    jobject selectedColor = stagedCount > 0
        ? env->CallObjectMethod(stagedColors, listGet, 0)
        : nullptr;
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

    const std::wstring mutexName = L"Local\\ColdEternityTeam_LunarCosmetics_" +
        std::to_wstring(GetCurrentProcessId());
    HANDLE singleInstance = CreateMutexW(nullptr, TRUE, mutexName.c_str());
    if (!singleInstance || GetLastError() == ERROR_ALREADY_EXISTS) {
        if (singleInstance) CloseHandle(singleInstance);
        return;
    }

    logLine("AGENT_LOADED pid=" + std::to_string(GetCurrentProcessId()));
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
                        env, jvmti, responseClass, response.object);
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
    if (attached) vms[0]->DetachCurrentThread();
    logLine("AGENT_COMPLETE success=" + std::to_string(unlocked && extrasComplete));
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
