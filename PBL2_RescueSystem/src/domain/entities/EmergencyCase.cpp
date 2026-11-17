#include "EmergencyCase.h"
#include "Victim.h"
#include "RescueUnit.h"
#include <cstdio>
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
// THỰC THI HỒ SƠ SỰ CỐ, AGING & TÍNH PHÍ HÓA ĐƠN (BILLING)
// ============================================================================
EmergencyCase::EmergencyCase(const char* id, const char* locCode, const char* addr, const char* desc,
                             CustomDateTime reportedTime, IncidentType type, EmergencyUrgency urgency,
                             unsigned int equipMask, Victim* v)
    : timeReported(reportedTime), timeDispatched(reportedTime), timeResolved(reportedTime),
    incidentType(type), baseUrgency(urgency), attachedEquipmentMask(equipMask),
    status(CASE_PENDING), victim(v), assignedUnit(nullptr), estimatedDistanceMinutes(5.0f),
    baseServiceFeeVND(0.0), equipmentSurchargeVND(0.0), distanceFeeVND(0.0),
    totalAmountVND(0.0), paymentStatus(PAYMENT_FREE_PUBLIC) {
    safeCopyStr(caseID, id, MAX_ID_LEN);
    safeCopyStr(locationCode, locCode, MAX_ID_LEN);
    safeCopyStr(detailedAddress, defaultIfEmpty(addr, "Đà Nẵng"), MAX_ADDR_LEN);
    safeCopyStr(sceneDescription, defaultIfEmpty(desc, "Yêu cầu hỗ trợ khẩn cấp"), MAX_DESC_LEN);

    // Tạo mã Hóa đơn tương ứng với mã Sự cố (VD: SOS-0001 -> BILL-0001)
    const char* numPart = strchr(id, '-');
    if (numPart) {
        snprintf(invoiceID, MAX_ID_LEN, "BILL%s", numPart);
    } else {
        snprintf(invoiceID, MAX_ID_LEN, "BILL-%s", id);
    }

    calculateServiceBill(5.0f);
}

EmergencyCase::~EmergencyCase() {
    if (victim != nullptr) {
        delete victim;
        victim = nullptr;
    }
}

double EmergencyCase::calculateDynamicPriority(const CustomDateTime& currentTime) const {
    double baseScore = (int)baseUrgency * 100.0;
    double vitalScore = 0.0;
    if (victim != nullptr) {
        if (!victim->checkBreathing()) vitalScore += 150.0;
        if (victim->getConsciousState() == CONSCIOUS_UNCONSCIOUS) vitalScore += 80.0;
        else if (victim->getConsciousState() == CONSCIOUS_PANICKED) vitalScore += 30.0;
        if (victim->checkTrapped()) vitalScore += 50.0;
    }
    long waitedMinutes = currentTime - timeReported;
    if (waitedMinutes < 0) waitedMinutes = 0;
    double agingScore = waitedMinutes * 3.5;

    return baseScore + vitalScore + agingScore;
}

void EmergencyCase::calculateServiceBill(float travelMinutes) {
    estimatedDistanceMinutes = (travelMinutes > 0.0f) ? travelMinutes : 2.0f;

    if (!isPaidServiceCase()) {
        baseServiceFeeVND = 0.0;
        equipmentSurchargeVND = 0.0;
        distanceFeeVND = 0.0;
        totalAmountVND = 0.0;
        paymentStatus = PAYMENT_FREE_PUBLIC;
        return;
    }

    // 1. Bảng giá cơ bản cho các dịch vụ cứu hộ giao thông có phí
    if (incidentType == INCIDENT_SERVICE_TOWING) {
        baseServiceFeeVND = 500000.0; // Phí cẩu/kéo xe cơ bản: 500,000 VNĐ
    } else if (incidentType == INCIDENT_SERVICE_TIRE_BATT) {
        baseServiceFeeVND = 200000.0; // Phí thay lốp / kích bình lưu động: 200,000 VNĐ
    } else if (incidentType == INCIDENT_SERVICE_UNLOCK) {
        baseServiceFeeVND = 300000.0; // Phí mở khóa / cứu hộ ngập nước: 300,000 VNĐ
    }

    // 2. Phụ phí thiết bị đi kèm từ kho chung
    equipmentSurchargeVND = 0.0;
    if (attachedEquipmentMask & EQUIP_HEAVY_TOW_CRANE)   equipmentSurchargeVND += 150000.0;
    if (attachedEquipmentMask & EQUIP_SPARE_TIRE_KIT)    equipmentSurchargeVND += 80000.0;
    if (attachedEquipmentMask & EQUIP_BATTERY_BOOSTER)   equipmentSurchargeVND += 70000.0;
    if (attachedEquipmentMask & EQUIP_HYDRAULIC_CUTTER)  equipmentSurchargeVND += 100000.0;

    // 3. Phí quãng đường tiếp cận (25,000 VNĐ / phút di chuyển định tuyến)
    distanceFeeVND = (double)((int)estimatedDistanceMinutes) * 25000.0;

    totalAmountVND = baseServiceFeeVND + equipmentSurchargeVND + distanceFeeVND;
    if (paymentStatus == PAYMENT_FREE_PUBLIC) {
        paymentStatus = PAYMENT_UNPAID;
    }
}

bool EmergencyCase::confirmBillPayment() {
    if (!isPaidServiceCase() || paymentStatus == PAYMENT_PAID) return false;
    paymentStatus = PAYMENT_PAID;
    if (assignedUnit != nullptr) {
        assignedUnit->addServiceRevenue(totalAmountVND);
    }
    return true;
}

void EmergencyCase::assignRescueUnit(RescueUnit* unit, const CustomDateTime& dispatchTime, float travelMinutes) {
    if (unit == nullptr) return;
    assignedUnit = unit;
    timeDispatched = dispatchTime;
    status = CASE_DISPATCHED;

    // Gắn thiết bị từ kho chung cho đội mang đi và cập nhật hóa đơn theo quãng đường thực tế
    unit->equipFromSharedPool(attachedEquipmentMask);
    unit->updateStatus(UNIT_EN_ROUTE);
    calculateServiceBill(travelMinutes);
}

void EmergencyCase::completeCase(RescueOutcome outcome, const CustomDateTime& resolveTime) {
    timeResolved = resolveTime;
    status = CASE_RESOLVED;
    if (victim != nullptr) {
        victim->setFinalOutcome(outcome);
    }
    if (assignedUnit != nullptr) {
        assignedUnit->recordMissionSuccess();
        assignedUnit->updateStatus(UNIT_AVAILABLE);
    }
}

void EmergencyCase::formatEquippedItemsText(char* outBuffer, int maxLen) const {
    if (attachedEquipmentMask == EQUIP_NONE) {
        safeCopyStr(outBuffer, "Trang bị tiêu chuẩn", maxLen);
        return;
    }
    outBuffer[0] = '\0';
    auto appendEq = [&](unsigned int bit, const char* label) {
        if (attachedEquipmentMask & bit) {
            if (outBuffer[0] != '\0' && (int)strlen(outBuffer) + 3 < maxLen) strcat(outBuffer, ", ");
            if ((int)strlen(outBuffer) + (int)strlen(label) < maxLen - 1) strcat(outBuffer, label);
        }
    };
    appendEq(EQUIP_FIRST_AID_KIT, "Sơ cứu");
    appendEq(EQUIP_AED_DEFIBRILLATOR, "Máy AED");
    appendEq(EQUIP_OXYGEN_TANK, "Bình Oxy");
    appendEq(EQUIP_HYDRAULIC_CUTTER, "Kìm thủy lực");
    appendEq(EQUIP_TRIPOD_WINCH, "Tời cáp");
    appendEq(EQUIP_SHORING_JACKS, "Cột chống");
    appendEq(EQUIP_FIRE_HOSE_PUMP, "Vòi rồng");
    appendEq(EQUIP_HEAVY_TOW_CRANE, "Cáp cẩu");
    appendEq(EQUIP_ANIMAL_CAGE_NET, "Lồng thú");
    appendEq(EQUIP_TRANQUILIZER_GUN, "Thuốc mê");
    appendEq(EQUIP_SPARE_TIRE_KIT, "Bộ thay lốp");
    appendEq(EQUIP_BATTERY_BOOSTER, "Kích bình");
}

void EmergencyCase::formatCaseSummary(char* outBuffer, int maxLen) const {
    const char* unitIdStr = (assignedUnit != nullptr) ? assignedUnit->getUnitID() : "Đang chờ";
    snprintf(outBuffer, maxLen, "Mã: %s | Khu vực: %s (%s) | Đội: %s | Phí: %.0f VNĐ",
             caseID, locationCode, detailedAddress, unitIdStr, totalAmountVND);
}
