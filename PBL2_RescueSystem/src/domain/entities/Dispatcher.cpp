#include "Dispatcher.h"
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

// ============================================================================
// THỰC THI LỚP TỔNG ĐÀI VIÊN (ADMIN)
// ============================================================================
Dispatcher::Dispatcher(const char* id, const char* pass, const char* name,
                       const char* phone, int gen, CustomDateTime dob, const char* shift)
    : PersonBase(name, phone, gen, dob), handledCasesCount(0) {
    safeCopyStr(staffID, id, MAX_ID_LEN);
    safeCopyStr(password, pass, 32);
    safeCopyStr(shiftCode, shift, MAX_ID_LEN);
}
