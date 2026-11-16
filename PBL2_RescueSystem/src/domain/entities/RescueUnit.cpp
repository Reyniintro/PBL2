#include "RescueUnit.h"
#include <cstring>

static void safeCopyStr(char* dest, const char* src, int maxLen) {
    if (!dest || maxLen <= 0) return;
    if (!src) {
        dest[0] = '\0';
        return;
    }
    strncpy(dest, src, maxLen - 1);
    dest[maxLen - 1] = '\0';
}

// Kiểm tra chuỗi trống để tự điền giá trị mặc định khi người dùng bỏ trống thông tin cá nhân
static const char* defaultIfEmpty(const char* str, const char* fallback) {
    if (!str || str[0] == '\0') return fallback;
    return str;
}

// ============================================================================
// THỰC THI CÂY ĐỘI CỨU HỘ (QUẢN LÝ ĐỘI TRƯỞNG, THÀNH VIÊN & DOANH THU)
// ============================================================================
RescueUnit::RescueUnit(const char* id, const char* name, const char* locCode, const char* vehicle,
                       const char* leader, const char* leadPhone)
    : memberCount(0), status(UNIT_AVAILABLE), activeEquipmentMask(EQUIP_NONE),
    completedMissions(0), totalRevenueVND(0.0) {
    safeCopyStr(unitID, id, MAX_ID_LEN);
    safeCopyStr(unitName, name, MAX_NAME_LEN);
    safeCopyStr(stationLocationCode, locCode, MAX_ID_LEN);
    safeCopyStr(currentLocationCode, locCode, MAX_ID_LEN);
    safeCopyStr(vehicleType, vehicle, MAX_NAME_LEN);
    safeCopyStr(leaderName, defaultIfEmpty(leader, "Đội trưởng trực ban"), MAX_NAME_LEN);
    safeCopyStr(leaderPhone, defaultIfEmpty(leadPhone, "0905000114"), MAX_PHONE_LEN);

    for (int i = 0; i < MAX_TEAM_MEMBERS; ++i) {
        memberNames[i][0] = '\0';
    }
}

void RescueUnit::addTeamMember(const char* memberFullName) {
    if (!memberFullName || memberFullName[0] == '\0' || memberCount >= MAX_TEAM_MEMBERS) return;
    safeCopyStr(memberNames[memberCount++], memberFullName, MAX_NAME_LEN);
}

void RescueUnit::setLeaderInfo(const char* name, const char* phone) {
    safeCopyStr(leaderName, name, MAX_NAME_LEN);
    safeCopyStr(leaderPhone, phone, MAX_PHONE_LEN);
}

void RescueUnit::updateStatus(UnitStatus newStatus) {
    status = newStatus;
    if (newStatus == UNIT_AVAILABLE) {
        safeCopyStr(currentLocationCode, stationLocationCode, MAX_ID_LEN);
        activeEquipmentMask = EQUIP_NONE; // Trả thiết bị về kho chung khi xong nhiệm vụ
    }
}

void RescueUnit::updateLocation(const char* newLocCode) {
    safeCopyStr(currentLocationCode, newLocCode, MAX_ID_LEN);
}
