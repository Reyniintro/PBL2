#ifndef PERSON_BASE_H
#define PERSON_BASE_H

#include "DomainConstants.h"
#include "CustomDateTime.h"

// ============================================================================
// LỚP CON NGƯỜI CƠ SỞ
// ============================================================================
class PersonBase {
protected:
    char fullName[MAX_NAME_LEN];
    char phoneNumber[MAX_PHONE_LEN];
    int gender;
    CustomDateTime dateOfBirth;

public:
    PersonBase(const char* name, const char* phone, int gen, CustomDateTime dob);
    virtual ~PersonBase() {}

    const char* getFullName() const { return fullName; }
    const char* getPhoneNumber() const { return phoneNumber; }
    int getGender() const { return gender; }
    CustomDateTime getDateOfBirth() const { return dateOfBirth; }
};

#endif // PERSON_BASE_H
