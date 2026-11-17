#ifndef APPLICATION_DISPATCH_MANAGER_H
#define APPLICATION_DISPATCH_MANAGER_H

#include "SpatialGraph.h"
#include "EmergencyPriorityQueue.h"
#include <cstdio>
#include "RescueEnums.h"
#include "CustomDateTime.h"
#include "Dispatcher.h"
#include "Victim.h"
#include "HumanVictim.h"
#include "AnimalVictim.h"
#include "RescueUnit.h"
#include "MedicalTeam.h"
#include "FireTeam.h"
#include "MechanicTeam.h"
#include "AnimalRescueTeam.h"
#include "EmergencyCase.h"

#define MAX_TOTAL_UNITS 60
#define MAX_HISTORY_CASES 500

// Cấu trúc lưu trữ số liệu Thống kê & Doanh thu Dịch vụ
struct SystemStatistics {
    int totalCasesReported;
    int pendingCasesCount;
    int activeDispatchedCount;
    int resolvedCasesCount;
    int totalHumansRescued;
    int totalAnimalsRescued;
    int outcomeSafeCount;
    int outcomeHospitalizedCount;
    int outcomeDeceasedCount;

    // Thống kê tài chính dịch vụ có phí (Kéo xe, Thay lốp, Kích bình)
    int paidServiceCasesCount;
    double totalPaidRevenueVND;
    double totalUnpaidPendingVND;
};

// ============================================================================
// BỘ QUẢN LÝ TRUNG TÂM (TỰ ĐỘNG ĐIỀU PHỐI NGAY LẬP TỨC + QUẢN LÝ ĐỘI + BILLING)
// ============================================================================
class DispatchManager {
private:
    SpatialGraph cityGraph;
    EmergencyPriorityQueue pendingQueue;

    RescueUnit* allUnits[MAX_TOTAL_UNITS];
    int totalUnitCount;

    EmergencyCase* activeAndResolvedCases[MAX_HISTORY_CASES];
    int historyCaseCount;

    CustomDateTime systemClock;
    int autoCaseCounter;

public:
    DispatchManager();
    ~DispatchManager();

    DispatchManager(const DispatchManager&) = delete;
    DispatchManager& operator=(const DispatchManager&) = delete;

    void initializeSystem(const char* mapFile, const char* unitsFile, const char* casesFile);
    void generateDefaultUnits(const char* unitsFile);
    void generateDefaultCases(const char* casesFile);

    bool saveUnitsToFile(const char* filePath) const;
    bool saveCasesToFile(const char* filePath) const;

    // Sinh trước mã ID sự cố tiếp theo để hiển thị trên Form tiếp nhận
    void peekNextCaseID(char* outBuf, int maxLen) const {
        snprintf(outBuf, maxLen, "SOS-%04d", autoCaseCounter);
    }

    // Tiếp nhận cuộc gọi VÀ TỰ ĐỘNG ĐIỀU PHỐI NGAY LẬP TỨC (Không chờ bấm thủ công)
    EmergencyCase* reportAndAutoDispatch(const char* locCode, const char* addr, const char* desc,
                                         IncidentType incType, EmergencyUrgency urgency,
                                         unsigned int equipMask, Victim* victim,
                                         DispatchResult& outDispatchRes,
                                         char* outLogMsg, int maxLogLen);

    // Tự động quét hàng đợi để điều động đội cho các ca đang chờ
    DispatchResult dispatchHighestPriorityCase(char* outLogMsg, int maxLogLen);

    // Hoàn thành ca đang xử lý (Và tự động điều đội vừa rảnh cho ca đang chờ nếu có)
    bool completeActiveCase(const char* caseId, RescueOutcome outcome, char* outLogMsg, int maxLogLen);

    // Thanh toán hóa đơn dịch vụ cho đội cứu hộ
    bool payServiceInvoice(const char* caseId, char* outLogMsg, int maxLogLen);

    // Thêm đội cứu hộ mới trực tiếp từ Tab Quản lý Đội cứu hộ
    RescueUnit* addNewRescueUnit(SpecialtyType spec, const char* unitId, const char* unitName,
                                 const char* stationCode, const char* vehicle,
                                 const char* leaderName, const char* leaderPhone);

    void advanceSystemTimeMinutes(int minutesToAdd);

    const SpatialGraph& getCityGraph() const { return cityGraph; }
    SpatialGraph& getMutableGraph() { return cityGraph; }
    const EmergencyPriorityQueue& getPendingQueue() const { return pendingQueue; }
    CustomDateTime getSystemClock() const { return systemClock; }

    int getTotalUnitCount() const { return totalUnitCount; }
    RescueUnit* getUnitAt(int idx) const {
        return (idx >= 0 && idx < totalUnitCount) ? allUnits[idx] : nullptr;
    }

    int getHistoryCaseCount() const { return historyCaseCount; }
    EmergencyCase* getHistoryCaseAt(int idx) const {
        return (idx >= 0 && idx < historyCaseCount) ? activeAndResolvedCases[idx] : nullptr;
    }

    SystemStatistics computeStatistics() const;
};

#endif // APPLICATION_DISPATCH_MANAGER_H
