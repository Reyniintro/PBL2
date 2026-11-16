#ifndef ANIMAL_RESCUE_TEAM_H
#define ANIMAL_RESCUE_TEAM_H

#include "RescueUnit.h"

class AnimalRescueTeam : public RescueUnit {
private:
    int specializedCages;
    bool hasVeterinarian;
public:
    AnimalRescueTeam(const char* id, const char* name, const char* locCode, const char* vehicle,
                     const char* leader, const char* leadPhone, int cages, bool hasVet);

    SpecialtyType getSpecialty() const override { return SPEC_ANIMAL; }
    const char* getSpecialtyName() const override { return "Cứu Hộ Động Vật"; }
    float evaluateSuitabilityScore(IncidentType incident, const Victim* victim) const override;
    void formatUnitInfo(char* outBuffer, int maxLen) const override;
};

#endif // ANIMAL_RESCUE_TEAM_H
