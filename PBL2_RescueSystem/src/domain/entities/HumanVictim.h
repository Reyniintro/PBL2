#ifndef HUMAN_VICTIM_H
#define HUMAN_VICTIM_H

#include "Victim.h"
#include "PersonBase.h"
#include "DomainConstants.h"
#include "RescueEnums.h"
#include "CustomDateTime.h"

// ============================================================================
// LỚP NẠN NHÂN LÀ CON NGƯỜI / KHÁCH HÀNG
// ============================================================================
class HumanVictim : public Victim, public PersonBase {
private:
    char address[MAX_ADDR_LEN];
    char medicalHistory[MAX_DESC_LEN];

public:
    HumanVictim(const char* id, const char* name, const char* phone, int gen, CustomDateTime dob,
                const char* addr, const char* medHist, Consciousness state, bool breathing, bool trapped, int count = 1);

    bool isHuman() const override { return true; }
    void formatDetails(char* outBuffer, int maxLen) const override;
    void serializeToFile(char* outBuffer, int maxLen) const override;

    void updateMedicalHistory(const char* newHistory);
    const char* getMedicalHistory() const { return medicalHistory; }
};

#endif // HUMAN_VICTIM_H
