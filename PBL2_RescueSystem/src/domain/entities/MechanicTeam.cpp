#include "MechanicTeam.h"
#include <cstdio>

// --- Đội Cứu Hộ Giao Thông & Dịch Vụ Cơ Giới (Kéo xe, Thay lốp, Kích bình) ---
MechanicTeam::MechanicTeam(const char* id, const char* name, const char* locCode, const char* vehicle,
                           const char* leader, const char* leadPhone, float maxTowTons)
    : RescueUnit(id, name, locCode, vehicle, leader, leadPhone),
    maxTowCapacityTons(maxTowTons) {}

float MechanicTeam::evaluateSuitabilityScore(IncidentType incident, const Victim* victim) const {
    if (incident == INCIDENT_SERVICE_TOWING || incident == INCIDENT_SERVICE_TIRE_BATT ||
        incident == INCIDENT_SERVICE_UNLOCK) {
        return 140.0f + maxTowCapacityTons;
    }
    if (incident == INCIDENT_COLLAPSED_TUNNEL || incident == INCIDENT_TRAPPED_FENCE) {
        return 60.0f;
    }
    (void)victim;
    return -50.0f;
}

void MechanicTeam::formatUnitInfo(char* outBuffer, int maxLen) const {
    snprintf(outBuffer, maxLen, "%s (Tải %.0f tấn) | Đội trưởng: %s (%s) | Thành viên: %d",
             vehicleType, maxTowCapacityTons, leaderName, leaderPhone, memberCount);
}
