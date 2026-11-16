#include "AnimalVictim.h"
#include <cstdio>
#include <cstring>

static void safeCopyStr(char* dest, const char* src, int maxLen) {
    if (!dest || maxLen <= 0) return;
    if (!src) {
        dest[0] = '\0';
        return;
    }
    strncpy(dest, src, maxLen - 1);
    dest[maxLen - 1] = '\0';
}

// Kiểm tra chuỗi trống để tự điền giá trị mặc định khi người dùng bỏ trống thông tin cá nhân
static const char* defaultIfEmpty(const char* str, const char* fallback) {
    if (!str || str[0] == '\0') return fallback;
    return str;
}

// ============================================================================
// THỰC THI LỚP NẠN NHÂN LÀ ĐỘNG VẬT
// ============================================================================
AnimalVictim::AnimalVictim(const char* id, const char* spec, const char* features, const char* repPhone,
                           float weight, bool aggressive, Consciousness state, bool breathing, bool trapped, int count)
    : Victim(id, state, breathing, trapped, count), estimatedWeightKg(weight), isAggressiveOrVenomous(aggressive) {
    safeCopyStr(species, defaultIfEmpty(spec, "Dong vat chua ro loai"), MAX_NAME_LEN);
    safeCopyStr(identifyingFeatures, defaultIfEmpty(features, "Chua ro dac diem"), MAX_DESC_LEN);
    safeCopyStr(reporterPhone, defaultIfEmpty(repPhone, "Chua cung cap"), MAX_PHONE_LEN);
}

void AnimalVictim::formatDetails(char* outBuffer, int maxLen) const {
    snprintf(outBuffer, maxLen, "[Động vật] %s (%.1fkg) | SĐT báo: %s | Đặc điểm: %s",
             species, estimatedWeightKg, reporterPhone, identifyingFeatures);
}

void AnimalVictim::serializeToFile(char* outBuffer, int maxLen) const {
    snprintf(outBuffer, maxLen, "ANIMAL|%s|%s|%.1f|%d|%s|%s|%d",
             victimID, species, estimatedWeightKg, isAggressiveOrVenomous ? 1 : 0,
             identifyingFeatures, reporterPhone, victimCount);
}
