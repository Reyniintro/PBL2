#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <cstring>
#include "DomainConstants.h"
#include "PersonBase.h"

// ============================================================================
// LỚP TỔNG ĐÀI VIÊN (ADMIN)
// ============================================================================
class Dispatcher : public PersonBase {
private:
    char staffID[MAX_ID_LEN];
    char password[32];
    char shiftCode[MAX_ID_LEN];
    int handledCasesCount;

public:
    Dispatcher(const char* id, const char* pass, const char* name,
               const char* phone, int gen, CustomDateTime dob, const char* shift);

    const char* getStaffID() const { return staffID; }
    const char* getShiftCode() const { return shiftCode; }
    bool verifyPassword(const char* inputPass) const {
        return (inputPass != nullptr) && (strcmp(password, inputPass) == 0);
    }
    void incrementHandledCases() { handledCasesCount++; }
    int getHandledCasesCount() const { return handledCasesCount; }
};

#endif // DISPATCHER_H
