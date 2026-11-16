#include "AnimalRescueTeam.h"
#include "Victim.h"
#include <cstdio>

// --- Đội Cứu Hộ Động Vật ---
AnimalRescueTeam::AnimalRescueTeam(const char* id, const char* name, const char* locCode, const char* vehicle,
                                   const char* leader, const char* leadPhone, int cages, bool hasVet)
    : RescueUnit(id, name, locCode, vehicle, leader, leadPhone),
    specializedCages(cages), hasVeterinarian(hasVet) {}

float AnimalRescueTeam::evaluateSuitabilityScore(IncidentType incident, const Victim* victim) const {
    if (incident == INCIDENT_ANIMAL_RESCUE) return 140.0f + (hasVeterinarian ? 15.0f : 0.0f);
    if (victim && !victim->isHuman()) return 100.0f;
    return -100.0f;
}

void AnimalRescueTeam::formatUnitInfo(char* outBuffer, int maxLen) const {
    snprintf(outBuffer, maxLen, "%s | Đội trưởng: %s (%s) | Thành viên: %d | Thú y: %s",
             vehicleType, leaderName, leaderPhone, memberCount, hasVeterinarian ? "Có" : "Không");
}
