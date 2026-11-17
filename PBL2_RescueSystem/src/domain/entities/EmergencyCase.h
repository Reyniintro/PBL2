#ifndef EMERGENCY_CASE_H
#define EMERGENCY_CASE_H

#include "DomainConstants.h"
#include "RescueEnums.h"
#include "CustomDateTime.h"

class Victim;
class RescueUnit;

// ============================================================================
// LỚP HỒ SƠ SỰ CỐ & HÓA ĐƠN DỊCH VỤ (EMERGENCY CASE & BILLING)
// ============================================================================
class EmergencyCase {
private:
    char caseID[MAX_ID_LEN];
    char locationCode[MAX_ID_LEN];
    char detailedAddress[MAX_ADDR_LEN];
    char sceneDescription[MAX_DESC_LEN];

    CustomDateTime timeReported;
    CustomDateTime timeDispatched;
    CustomDateTime timeResolved;

    IncidentType incidentType;
    EmergencyUrgency baseUrgency;
    unsigned int attachedEquipmentMask; // Thiết bị lấy từ kho chung đi kèm ca này
    CaseStatus status;

    Victim* victim;
    RescueUnit* assignedUnit;
    float estimatedDistanceMinutes;

    // Thông tin Chi phí & Thanh toán hóa đơn (Dành cho dịch vụ Kéo xe, Thay lốp...)
    char invoiceID[MAX_ID_LEN];         // Mã hóa đơn (VD: "BILL-0001")
    double baseServiceFeeVND;           // Phí dịch vụ cơ bản
    double equipmentSurchargeVND;       // Phụ phí thiết bị / vật tư thay thế
    double distanceFeeVND;              // Phí vận chuyển theo quãng đường
    double totalAmountVND;              // Tổng tiền thanh toán
    PaymentStatus paymentStatus;

    EmergencyCase(const EmergencyCase&) = delete;
    EmergencyCase& operator=(const EmergencyCase&) = delete;

public:
    EmergencyCase(const char* id, const char* locCode, const char* addr, const char* desc,
                  CustomDateTime reportedTime, IncidentType type, EmergencyUrgency urgency,
                  unsigned int equipMask, Victim* v);
    ~EmergencyCase();

    double calculateDynamicPriority(const CustomDateTime& currentTime) const;

    void assignRescueUnit(RescueUnit* unit, const CustomDateTime& dispatchTime, float travelMinutes = 5.0f);
    void completeCase(RescueOutcome outcome, const CustomDateTime& resolveTime);

    // Nghiệp vụ tính phí tự động và Thanh toán Hóa đơn cho đội cứu hộ
    void calculateServiceBill(float travelMinutes);
    bool confirmBillPayment(); // Xác nhận khách đã thanh toán -> Cộng doanh thu cho Đội cứu hộ

    bool isPaidServiceCase() const {
        return (incidentType == INCIDENT_SERVICE_TOWING ||
                incidentType == INCIDENT_SERVICE_TIRE_BATT ||
                incidentType == INCIDENT_SERVICE_UNLOCK);
    }

    // Getters
    const char* getCaseID() const { return caseID; }
    const char* getLocationCode() const { return locationCode; }
    const char* getDetailedAddress() const { return detailedAddress; }
    const char* getSceneDescription() const { return sceneDescription; }
    IncidentType getIncidentType() const { return incidentType; }
    EmergencyUrgency getBaseUrgency() const { return baseUrgency; }
    unsigned int getRequiredEquipmentMask() const { return attachedEquipmentMask; }
    CaseStatus getStatus() const { return status; }
    Victim* getVictim() const { return victim; }
    RescueUnit* getAssignedUnit() const { return assignedUnit; }
    CustomDateTime getTimeReported() const { return timeReported; }

    const char* getInvoiceID() const { return invoiceID; }
    double getBaseServiceFeeVND() const { return baseServiceFeeVND; }
    double getEquipmentSurchargeVND() const { return equipmentSurchargeVND; }
    double getDistanceFeeVND() const { return distanceFeeVND; }
    double getTotalAmountVND() const { return totalAmountVND; }
    PaymentStatus getPaymentStatus() const { return paymentStatus; }

    void formatCaseSummary(char* outBuffer, int maxLen) const;
    void formatEquippedItemsText(char* outBuffer, int maxLen) const;
};

#endif // EMERGENCY_CASE_H
