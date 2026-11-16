#include "PersonBase.h"
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
// THỰC THI LỚP CON NGƯỜI CƠ SỞ
// ============================================================================
PersonBase::PersonBase(const char* name, const char* phone, int gen, CustomDateTime dob)
    : gender(gen), dateOfBirth(dob) {
    safeCopyStr(fullName, defaultIfEmpty(name, "Khach vang lai / Chua ro ten"), MAX_NAME_LEN);
    safeCopyStr(phoneNumber, defaultIfEmpty(phone, "Chua cung cap"), MAX_PHONE_LEN);
}
