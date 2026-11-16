#ifndef RESCUE_UNIT_H
#define RESCUE_UNIT_H

#include "DomainConstants.h"
#include "RescueEnums.h"

class Victim;

// ============================================================================
// CÂY PHÂN CẤP ĐỘI CỨU HỘ (CÓ ĐỘI TRƯỞNG, THÀNH VIÊN & DOANH THU DỊCH VỤ)
// ============================================================================
class RescueUnit {
protected:
    char unitID[MAX_ID_LEN];
    char unitName[MAX_NAME_LEN];            // Tên phiên hiệu đội (VD: "Đội Cứu Hộ Sông Hàn 01")
    char stationLocationCode[MAX_ID_LEN];   // Mã khu vực đóng quân
    char currentLocationCode[MAX_ID_LEN];   // Vị trí hiện tại
    char vehicleType[MAX_NAME_LEN];         // Phương tiện chính

    // Thông tin Đội trưởng & Danh sách thành viên (Yêu cầu mới của Giảng viên)
    char leaderName[MAX_NAME_LEN];
    char leaderPhone[MAX_PHONE_LEN];
    char memberNames[MAX_TEAM_MEMBERS][MAX_NAME_LEN];
    int memberCount;

    UnitStatus status;
    unsigned int activeEquipmentMask;       // Thiết bị được tích chọn mang theo từ Kho chung
    int completedMissions;
    double totalRevenueVND;                 // Tổng số tiền dịch vụ đội đã thu / được thanh toán (VNĐ)

public:
    RescueUnit(const char* id, const char* name, const char* locCode, const char* vehicle,
               const char* leader, const char* leadPhone);
    virtual ~RescueUnit() {}

    virtual SpecialtyType getSpecialty() const = 0;
    virtual const char* getSpecialtyName() const = 0;
    virtual float evaluateSuitabilityScore(IncidentType incident, const Victim* victim) const = 0;
    virtual void formatUnitInfo(char* outBuffer, int maxLen) const = 0;

    // Quản lý thành viên trong đội
    void addTeamMember(const char* memberFullName);
    void setLeaderInfo(const char* name, const char* phone);
    const char* getLeaderName() const { return leaderName; }
    const char* getLeaderPhone() const { return leaderPhone; }
    int getMemberCount() const { return memberCount; }
    const char* getMemberNameAt(int idx) const {
        return (idx >= 0 && idx < memberCount) ? memberNames[idx] : "";
    }

    // Thiết bị từ kho chung luôn sẵn sàng để gắn cho đội khi xuất phát
    void equipFromSharedPool(unsigned int equipMask) { activeEquipmentMask = equipMask; }
    unsigned int getActiveEquipmentMask() const { return activeEquipmentMask; }

    void updateStatus(UnitStatus newStatus);
    void updateLocation(const char* newLocCode);
    void recordMissionSuccess() { completedMissions++; }
    void addServiceRevenue(double amountVND) { if (amountVND > 0) totalRevenueVND += amountVND; }

    const char* getUnitID() const { return unitID; }
    const char* getUnitName() const { return unitName; }
    const char* getStationLocationCode() const { return stationLocationCode; }
    const char* getCurrentLocationCode() const { return currentLocationCode; }
    const char* getVehicleType() const { return vehicleType; }
    UnitStatus getStatus() const { return status; }
    int getCompletedMissions() const { return completedMissions; }
    double getTotalRevenueVND() const { return totalRevenueVND; }
};

#endif // RESCUE_UNIT_H
