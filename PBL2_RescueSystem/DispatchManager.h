#ifndef DISPATCH_MANAGER_H
#define DISPATCH_MANAGER_H

#include "SpatialGraph.h"

#define MAX_HEAP_CAPACITY 200
#define MAX_TOTAL_UNITS 60
#define MAX_HISTORY_CASES 500

// ============================================================================
// 1. HÀNG ĐỢI ƯU TIÊN MAX-HEAP (LƯU CÁC CA CHỜ KHI ĐỘI ĐANG BẬN HẾT)
// ============================================================================
class EmergencyPriorityQueue {
private:
    EmergencyCase* heapArray[MAX_HEAP_CAPACITY];
    int currentSize;
    CustomDateTime referenceTime;

    void swapNodes(int i, int j);
    void heapifyUp(int index);
    void heapifyDown(int index);

public:
    EmergencyPriorityQueue();
    ~EmergencyPriorityQueue();

    void updateReferenceTime(const CustomDateTime& now);
    bool push(EmergencyCase* newCase);
    EmergencyCase* popMax();
    bool removeCaseByPointer(EmergencyCase* target);
    EmergencyCase* peekMax() const;
    void rebuildHeapForAging();

    int size() const { return currentSize; }
    bool isEmpty() const { return currentSize == 0; }
    EmergencyCase* getCaseAt(int idx) const {
        return (idx >= 0 && idx < currentSize) ? heapArray[idx] : nullptr;
    }
    void clearAll();
};

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
// 2. BỘ QUẢN LÝ TRUNG TÂM (TỰ ĐỘNG ĐIỀU PHỐI NGAY LẬP TỨC + QUẢN LÝ ĐỘI + BILLING)
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

#endif // DISPATCH_MANAGER_H