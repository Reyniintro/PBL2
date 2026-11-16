#include "HumanVictim.h"
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
// THỰC THI LỚP NẠN NHÂN LÀ CON NGƯỜI
// ============================================================================
HumanVictim::HumanVictim(const char* id, const char* name, const char* phone, int gen, CustomDateTime dob,
                         const char* addr, const char* medHist, Consciousness state, bool breathing, bool trapped, int count)
    : Victim(id, state, breathing, trapped, count), PersonBase(name, phone, gen, dob) {
    safeCopyStr(address, defaultIfEmpty(addr, "Hien truong tai Da Nang"), MAX_ADDR_LEN);
    safeCopyStr(medicalHistory, defaultIfEmpty(medHist, "Khong co / Chua ro"), MAX_DESC_LEN);
}

void HumanVictim::formatDetails(char* outBuffer, int maxLen) const {
    snprintf(outBuffer, maxLen, "%s | SĐT: %s | SL: %d | Ghi chú: %s",
             fullName, phoneNumber, victimCount, medicalHistory);
}

void HumanVictim::serializeToFile(char* outBuffer, int maxLen) const {
    snprintf(outBuffer, maxLen, "HUMAN|%s|%s|%s|%d|%s|%d|%d|%d",
             victimID, fullName, phoneNumber, victimCount, medicalHistory,
             (int)consciousState, isBreathing ? 1 : 0, isTrapped ? 1 : 0);
}

void HumanVictim::updateMedicalHistory(const char* newHistory) {
    safeCopyStr(medicalHistory, defaultIfEmpty(newHistory, "Khong co"), MAX_DESC_LEN);
}
