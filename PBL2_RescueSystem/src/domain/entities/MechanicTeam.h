#ifndef MECHANIC_TEAM_H
#define MECHANIC_TEAM_H

#include "RescueUnit.h"

class MechanicTeam : public RescueUnit {
private:
    float maxTowCapacityTons;
public:
    MechanicTeam(const char* id, const char* name, const char* locCode, const char* vehicle,
                 const char* leader, const char* leadPhone, float maxTowTons);

    SpecialtyType getSpecialty() const override { return SPEC_MECHANIC; }
    const char* getSpecialtyName() const override { return "Cứu Hộ Giao Thông & Cơ Giới"; }
    float evaluateSuitabilityScore(IncidentType incident, const Victim* victim) const override;
    void formatUnitInfo(char* outBuffer, int maxLen) const override;
};

#endif // MECHANIC_TEAM_H
