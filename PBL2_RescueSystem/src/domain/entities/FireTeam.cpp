#include "FireTeam.h"
#include <cstdio>

// --- Đội PCCC & CNCH ---
FireTeam::FireTeam(const char* id, const char* name, const char* locCode, const char* vehicle,
                   const char* leader, const char* leadPhone, float waterCap, int ladderHeight)
    : RescueUnit(id, name, locCode, vehicle, leader, leadPhone),
    waterCapacityLiters(waterCap), maxLadderHeightMeters(ladderHeight) {}

float FireTeam::evaluateSuitabilityScore(IncidentType incident, const Victim* victim) const {
    if (incident == INCIDENT_FIRE_EXPLOSION || incident == INCIDENT_COLLAPSED_TUNNEL ||
        incident == INCIDENT_FALLEN_INTO_WELL || incident == INCIDENT_TRAPPED_FENCE) {
        return 130.0f;
    }
    if (incident == INCIDENT_SERVICE_TIRE_BATT || incident == INCIDENT_SERVICE_TOWING) {
        return -100.0f;
    }
    (void)victim;
    return 15.0f;
}

void FireTeam::formatUnitInfo(char* outBuffer, int maxLen) const {
    snprintf(outBuffer, maxLen, "%s | Đội trưởng: %s (%s) | Thành viên: %d | Bồn: %.0fL",
             vehicleType, leaderName, leaderPhone, memberCount, waterCapacityLiters);
}
