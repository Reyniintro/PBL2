#include "CoreEntities.h"

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
// 1. THỰC THI CUSTOM DATETIME
// ============================================================================
long CustomDateTime::toTotalMinutes() const {
    long totalDays = (long)year * 365 + (long)month * 30 + day;
    return totalDays * 1440 + hour * 60 + minute;
}

// ============================================================================
// 2. THỰC THI LỚP CON NGƯỜI CƠ SỞ & TỔNG ĐÀI VIÊN
// ============================================================================
PersonBase::PersonBase(const char* name, const char* phone, int gen, CustomDateTime dob)
    : gender(gen), dateOfBirth(dob) {
    safeCopyStr(fullName, defaultIfEmpty(name, "Khach vang lai / Chua ro ten"), MAX_NAME_LEN);
    safeCopyStr(phoneNumber, defaultIfEmpty(phone, "Chua cung cap"), MAX_PHONE_LEN);
}

Dispatcher::Dispatcher(const char* id, const char* pass, const char* name,
                       const char* phone, int gen, CustomDateTime dob, const char* shift)
    : PersonBase(name, phone, gen, dob), handledCasesCount(0) {
    safeCopyStr(staffID, id, MAX_ID_LEN);
    safeCopyStr(password, pass, 32);
    safeCopyStr(shiftCode, shift, MAX_ID_LEN);
}

// ============================================================================
// 3. THỰC THI CÂY PHÂN CẤP NẠN NHÂN (CHO PHÉP ĐỂ TRỐNG THÔNG TIN CÁ NHÂN)
// ============================================================================
Victim::Victim(const char* id, Consciousness state, bool breathing, bool trapped, int count)
    : consciousState(state), isBreathing(breathing), isTrapped(trapped),
    victimCount(count > 0 ? count : 1), finalOutcome(OUTCOME_NONE) {
    safeCopyStr(victimID, id, MAX_ID_LEN);
}

void Victim::updateVitalSigns(Consciousness newState, bool breathing, bool trapped) {
    consciousState = newState;
    isBreathing = breathing;
    isTrapped = trapped;
}

HumanVictim::HumanVictim(const char* id, const char* name, const char* phone, int gen, CustomDateTime dob,
                         const char* addr, const char* medHist, Consciousness state, bool breathing, bool trapped, int count)
    : Victim(id, state, breathing, trapped, count), PersonBase(name, phone, gen, dob) {
    safeCopyStr(address, defaultIfEmpty(addr, "Hien truong tai Da Nang"), MAX_ADDR_LEN);
    safeCopyStr(medicalHistory, defaultIfEmpty(medHist, "Khong co / Chua ro"), MAX_DESC_LEN);
}

void HumanVictim::formatDetails(char* outBuffer, int maxLen) const {
    snprintf(outBuffer, maxLen, "%s | SĐT: %s | SL: %d | Ghi chú: %s",
             fullName, phoneNumber, victimCount, medicalHistory);
}

void HumanVictim::serializeToFile(char* outBuffer, int maxLen) const {
    snprintf(outBuffer, maxLen, "HUMAN|%s|%s|%s|%d|%s|%d|%d|%d",
             victimID, fullName, phoneNumber, victimCount, medicalHistory,
             (int)consciousState, isBreathing ? 1 : 0, isTrapped ? 1 : 0);
}

void HumanVictim::updateMedicalHistory(const char* newHistory) {
    safeCopyStr(medicalHistory, defaultIfEmpty(newHistory, "Khong co"), MAX_DESC_LEN);
}

AnimalVictim::AnimalVictim(const char* id, const char* spec, const char* features, const char* repPhone,
                           float weight, bool aggressive, Consciousness state, bool breathing, bool trapped, int count)
    : Victim(id, state, breathing, trapped, count), estimatedWeightKg(weight), isAggressiveOrVenomous(aggressive) {
    safeCopyStr(species, defaultIfEmpty(spec, "Dong vat chua ro loai"), MAX_NAME_LEN);
    safeCopyStr(identifyingFeatures, defaultIfEmpty(features, "Chua ro dac diem"), MAX_DESC_LEN);
    safeCopyStr(reporterPhone, defaultIfEmpty(repPhone, "Chua cung cap"), MAX_PHONE_LEN);
}

void AnimalVictim::formatDetails(char* outBuffer, int maxLen) const {
    snprintf(outBuffer, maxLen, "[Động vật] %s (%.1fkg) | SĐT báo: %s | Đặc điểm: %s",
             species, estimatedWeightKg, reporterPhone, identifyingFeatures);
}

void AnimalVictim::serializeToFile(char* outBuffer, int maxLen) const {
    snprintf(outBuffer, maxLen, "ANIMAL|%s|%s|%.1f|%d|%s|%s|%d",
             victimID, species, estimatedWeightKg, isAggressiveOrVenomous ? 1 : 0,
             identifyingFeatures, reporterPhone, victimCount);
}

// ============================================================================
// 4. THỰC THI CÂY ĐỘI CỨU HỘ (QUẢN LÝ ĐỘI TRƯỞNG, THÀNH VIÊN & DOANH THU)
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

// ============================================================================
// 5. THỰC THI HỒ SƠ SỰ CỐ, AGING & TÍNH PHÍ HÓA ĐƠN (BILLING)
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