#ifndef ANIMAL_VICTIM_H
#define ANIMAL_VICTIM_H

#include "Victim.h"
#include "DomainConstants.h"
#include "RescueEnums.h"

// ============================================================================
// LỚP NẠN NHÂN LÀ ĐỘNG VẬT
// ============================================================================
class AnimalVictim : public Victim {
private:
    char species[MAX_NAME_LEN];
    char identifyingFeatures[MAX_DESC_LEN];
    char reporterPhone[MAX_PHONE_LEN];
    float estimatedWeightKg;
    bool isAggressiveOrVenomous;

public:
    AnimalVictim(const char* id, const char* spec, const char* features, const char* repPhone,
                 float weight, bool aggressive, Consciousness state, bool breathing, bool trapped, int count = 1);

    bool isHuman() const override { return false; }
    void formatDetails(char* outBuffer, int maxLen) const override;
    void serializeToFile(char* outBuffer, int maxLen) const override;

    float getEstimatedWeight() const { return estimatedWeightKg; }
    bool checkAggressive() const { return isAggressiveOrVenomous; }
};

#endif // ANIMAL_VICTIM_H
