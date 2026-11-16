#ifndef MEDICAL_TEAM_H
#define MEDICAL_TEAM_H

#include "RescueUnit.h"

class MedicalTeam : public RescueUnit {
private:
    int ambulanceCount;
    int doctorCount;
public:
    MedicalTeam(const char* id, const char* name, const char* locCode, const char* vehicle,
                const char* leader, const char* leadPhone, int ambulances, int doctors);

    SpecialtyType getSpecialty() const override { return SPEC_MEDICAL; }
    const char* getSpecialtyName() const override { return "Y Tế Cấp Cứu 115"; }
    float evaluateSuitabilityScore(IncidentType incident, const Victim* victim) const override;
    void formatUnitInfo(char* outBuffer, int maxLen) const override;
};

#endif // MEDICAL_TEAM_H
