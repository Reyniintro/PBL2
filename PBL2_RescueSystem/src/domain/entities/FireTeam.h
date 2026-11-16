#ifndef FIRE_TEAM_H
#define FIRE_TEAM_H

#include "RescueUnit.h"

class FireTeam : public RescueUnit {
private:
    float waterCapacityLiters;
    int maxLadderHeightMeters;
public:
    FireTeam(const char* id, const char* name, const char* locCode, const char* vehicle,
             const char* leader, const char* leadPhone, float waterCap, int ladderHeight);

    SpecialtyType getSpecialty() const override { return SPEC_FIRE; }
    const char* getSpecialtyName() const override { return "PCCC & Cứu Nạn 114"; }
    float evaluateSuitabilityScore(IncidentType incident, const Victim* victim) const override;
    void formatUnitInfo(char* outBuffer, int maxLen) const override;
};

#endif // FIRE_TEAM_H
