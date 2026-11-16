#include "MedicalTeam.h"
#include "Victim.h"
#include <cstdio>

// --- Đội Y Tế ---
MedicalTeam::MedicalTeam(const char* id, const char* name, const char* locCode, const char* vehicle,
                         const char* leader, const char* leadPhone, int ambulances, int doctors)
    : RescueUnit(id, name, locCode, vehicle, leader, leadPhone),
    ambulanceCount(ambulances), doctorCount(doctors) {}

float MedicalTeam::evaluateSuitabilityScore(IncidentType incident, const Victim* victim) const {
    if (incident == INCIDENT_MEDICAL_EMERGENCY) return 120.0f + doctorCount * 5.0f;
    if (incident == INCIDENT_SERVICE_TOWING || incident == INCIDENT_SERVICE_TIRE_BATT ||
        incident == INCIDENT_SERVICE_UNLOCK || incident == INCIDENT_ANIMAL_RESCUE) {
        return -100.0f; // Không điều xe cứu thương đi thay lốp / kéo xe / bắt thú
    }
    float score = 20.0f;
    if (victim && victim->isHuman() && !victim->checkBreathing()) score += 40.0f;
    return score;
}

void MedicalTeam::formatUnitInfo(char* outBuffer, int maxLen) const {
    snprintf(outBuffer, maxLen, "%s | Đội trưởng: %s (%s) | Thành viên: %d | Bác sĩ: %d",
             vehicleType, leaderName, leaderPhone, memberCount, doctorCount);
}
