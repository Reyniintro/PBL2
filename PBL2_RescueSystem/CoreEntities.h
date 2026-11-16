#ifndef CORE_ENTITIES_H
#define CORE_ENTITIES_H

#include <cstdio>
#include <cstring>

// ============================================================================
// 1. CÁC HẰNG SỐ KÍCH THƯỚC BỘ NHỚ TĨNH (KHÔNG DÙNG STL)
// ============================================================================
#define MAX_ID_LEN 14
#define MAX_NAME_LEN 64
#define MAX_PHONE_LEN 16
#define MAX_ADDR_LEN 128
#define MAX_DESC_LEN 256
#define MAX_TEAM_MEMBERS 8   // Tối đa 8 thành viên trong 1 đội cứu hộ

// ============================================================================
// 2. HỆ THỐNG ENUM CHUẨN HÓA NGHIỆP VỤ CỨU HỘ & DỊCH VỤ CÓ PHÍ
// ============================================================================

enum UnitStatus {
    UNIT_AVAILABLE = 0,  // Đang rảnh tại trạm trực
    UNIT_EN_ROUTE  = 1,  // Đang di chuyển tới hiện trường
    UNIT_ON_SCENE  = 2,  // Đang làm nhiệm vụ
    UNIT_RETURNING = 3,  // Đang quay về trạm
    UNIT_OFFLINE   = 4   // Bảo trì / Nghỉ ca
};

enum CaseStatus {
    CASE_PENDING     = 0, // Đang chờ trong Hàng đợi ưu tiên
    CASE_DISPATCHED  = 1, // Đã điều động đội cứu hộ (Tự động)
    CASE_IN_PROGRESS = 2, // Đang xử lý tại hiện trường
    CASE_RESOLVED    = 3, // Đã hoàn thành nhiệm vụ
    CASE_CANCELLED   = 4  // Hủy bỏ
};

enum EmergencyUrgency {
    URGENCY_STANDARD = 1, // Mức 1: Hỗ trợ thông thường / Dịch vụ kỹ thuật
    URGENCY_URGENT   = 2, // Mức 2: Khẩn cấp
    URGENCY_CRITICAL = 3  // Mức 3: Nguy kịch tính mạng
};

enum Consciousness {
    CONSCIOUS_ALERT       = 0, // Tỉnh táo hoàn toàn
    CONSCIOUS_PANICKED    = 1, // Hoảng loạn / Kích động
    CONSCIOUS_CONFUSED    = 2, // Lơ mơ / Mất phương hướng
    CONSCIOUS_UNCONSCIOUS = 3  // Bất tỉnh / Hôn mê
};

enum RescueOutcome {
    OUTCOME_NONE             = 0,
    OUTCOME_SAFE_ON_SITE     = 1, // An toàn tại chỗ / Đã khắc phục xong sự cố xe
    OUTCOME_HOSPITALIZED     = 2, // Đã sơ cứu & Chuyển viện / Đưa xe về gara
    OUTCOME_DECEASED         = 3  // Không qua khỏi
};

// Mở rộng thêm các Dịch vụ Cứu hộ Giao thông có phí theo yêu cầu Giảng viên
enum IncidentType {
    INCIDENT_MEDICAL_EMERGENCY = 0, // Cấp cứu Y tế (Miễn phí công cộng)
    INCIDENT_TRAPPED_FENCE     = 1, // Mắc kẹt song cửa / Thang máy (Miễn phí)
    INCIDENT_COLLAPSED_TUNNEL  = 2, // Sập hầm / Công trình (Miễn phí)
    INCIDENT_FALLEN_INTO_WELL  = 3, // Rớt giếng / Hố sâu (Miễn phí)
    INCIDENT_FIRE_EXPLOSION    = 4, // Hỏa hoạn / Cháy nổ (Miễn phí)
    INCIDENT_ANIMAL_RESCUE     = 5, // Giải cứu Động vật (Miễn phí / Hỗ trợ)
    INCIDENT_SERVICE_TOWING    = 6, // [DỊCH VỤ CÓ PHÍ] Kéo xe / Cẩu xe tai nạn, chết máy
    INCIDENT_SERVICE_TIRE_BATT = 7, // [DỊCH VỤ CÓ PHÍ] Thay lốp lưu động / Kích bình ắc-quy
    INCIDENT_SERVICE_UNLOCK    = 8  // [DỊCH VỤ CÓ PHÍ] Mở khóa xe / Cứu hộ xe ngập nước
};

enum SpecialtyType {
    SPEC_MEDICAL  = 0, // Đội Y tế Cấp cứu 115
    SPEC_FIRE     = 1, // Đội PCCC & CNCH 114
    SPEC_MECHANIC = 2, // Đội Cứu hộ Giao thông & Dịch vụ Cơ giới
    SPEC_ANIMAL   = 3  // Đội Cứu hộ Động vật & Kiểm lâm
};

// KHO THIẾT BỊ DÙNG CHUNG (Luôn có sẵn để tích mang theo bất kỳ đội nào)
enum EquipmentMask : unsigned int {
    EQUIP_NONE              = 0,
    EQUIP_FIRST_AID_KIT     = 1 << 0, // Bộ sơ cứu y tế
    EQUIP_AED_DEFIBRILLATOR = 1 << 1, // Máy sốc tim tự động AED
    EQUIP_OXYGEN_TANK       = 1 << 2, // Bình dưỡng khí Oxy
    EQUIP_HYDRAULIC_CUTTER  = 1 << 3, // Kìm thủy lực phá cắt sắt
    EQUIP_TRIPOD_WINCH      = 1 << 4, // Giá 3 chân & Tời cáp giếng
    EQUIP_SHORING_JACKS     = 1 << 5, // Cột chống thủy lực sập hầm
    EQUIP_FIRE_HOSE_PUMP    = 1 << 6, // Vòi rồng & Bơm áp lực cao
    EQUIP_HEAVY_TOW_CRANE   = 1 << 7, // Cáp cẩu & Xe kéo chuyên dụng
    EQUIP_ANIMAL_CAGE_NET   = 1 << 8, // Lồng sắt & Lưới bắt thú
    EQUIP_TRANQUILIZER_GUN  = 1 << 9, // Súng bắn thuốc mê
    EQUIP_SPARE_TIRE_KIT    = 1 << 10,// Bộ thay lốp lưu động & Súng bắn ốc
    EQUIP_BATTERY_BOOSTER   = 1 << 11 // Máy kích bình ắc-quy công suất lớn
};

// Trạng thái Hóa đơn thanh toán (Billing Status)
enum PaymentStatus {
    PAYMENT_FREE_PUBLIC = 0, // Miễn phí (Ngân sách nhà nước hỗ trợ 114/115)
    PAYMENT_UNPAID      = 1, // Có phí - Chưa thanh toán
    PAYMENT_PAID        = 2  // Có phí - Đã thanh toán cho Đội cứu hộ
};

// ============================================================================
// 3. CẤU TRÚC THỜI GIAN TỰ CÀI ĐẶT
// ============================================================================
struct CustomDateTime {
    int day, month, year;
    int hour, minute, second;

    CustomDateTime(int d = 10, int m = 11, int y = 2026, int h = 8, int min = 0, int s = 0)
        : day(d), month(m), year(y), hour(h), minute(min), second(s) {}

    long toTotalMinutes() const;

    long operator-(const CustomDateTime& other) const {
        return this->toTotalMinutes() - other.toTotalMinutes();
    }

    void formatDate(char* buffer, int maxLen) const {
        snprintf(buffer, maxLen, "%02d/%02d/%04d", day, month, year);
    }

    void formatDateTime(char* buffer, int maxLen) const {
        snprintf(buffer, maxLen, "%02d/%02d/%04d %02d:%02d:%02d", day, month, year, hour, minute, second);
    }
};

// ============================================================================
// 4. LỚP CON NGƯỜI CƠ SỞ & TỔNG ĐÀI VIÊN (ADMIN)
// ============================================================================
class PersonBase {
protected:
    char fullName[MAX_NAME_LEN];
    char phoneNumber[MAX_PHONE_LEN];
    int gender;
    CustomDateTime dateOfBirth;

public:
    PersonBase(const char* name, const char* phone, int gen, CustomDateTime dob);
    virtual ~PersonBase() {}

    const char* getFullName() const { return fullName; }
    const char* getPhoneNumber() const { return phoneNumber; }
    int getGender() const { return gender; }
    CustomDateTime getDateOfBirth() const { return dateOfBirth; }
};

class Dispatcher : public PersonBase {
private:
    char staffID[MAX_ID_LEN];
    char password[32];
    char shiftCode[MAX_ID_LEN];
    int handledCasesCount;

public:
    Dispatcher(const char* id, const char* pass, const char* name,
               const char* phone, int gen, CustomDateTime dob, const char* shift);

    const char* getStaffID() const { return staffID; }
    const char* getShiftCode() const { return shiftCode; }
    bool verifyPassword(const char* inputPass) const {
        return (inputPass != nullptr) && (strcmp(password, inputPass) == 0);
    }
    void incrementHandledCases() { handledCasesCount++; }
    int getHandledCasesCount() const { return handledCasesCount; }
};

// ============================================================================
// 5. CÂY PHÂN CẤP NẠN NHÂN / KHÁCH HÀNG (HỖ TRỢ ĐỂ TRỐNG THÔNG TIN CÁ NHÂN)
// ============================================================================
class Victim {
protected:
    char victimID[MAX_ID_LEN];
    Consciousness consciousState;
    bool isBreathing;
    bool isTrapped;
    int victimCount;
    RescueOutcome finalOutcome;

public:
    Victim(const char* id, Consciousness state, bool breathing, bool trapped, int count = 1);
    virtual ~Victim() {}

    virtual bool isHuman() const = 0;
    virtual void formatDetails(char* outBuffer, int maxLen) const = 0;
    virtual void serializeToFile(char* outBuffer, int maxLen) const = 0;

    const char* getVictimID() const { return victimID; }
    Consciousness getConsciousState() const { return consciousState; }
    bool checkBreathing() const { return isBreathing; }
    bool checkTrapped() const { return isTrapped; }
    int getVictimCount() const { return victimCount; }
    RescueOutcome getOutcome() const { return finalOutcome; }

    void updateVitalSigns(Consciousness newState, bool breathing, bool trapped);
    void setFinalOutcome(RescueOutcome outcome) { finalOutcome = outcome; }
};

class HumanVictim : public Victim, public PersonBase {
private:
    char address[MAX_ADDR_LEN];
    char medicalHistory[MAX_DESC_LEN];

public:
    HumanVictim(const char* id, const char* name, const char* phone, int gen, CustomDateTime dob,
                const char* addr, const char* medHist, Consciousness state, bool breathing, bool trapped, int count = 1);

    bool isHuman() const override { return true; }
    void formatDetails(char* outBuffer, int maxLen) const override;
    void serializeToFile(char* outBuffer, int maxLen) const override;

    void updateMedicalHistory(const char* newHistory);
    const char* getMedicalHistory() const { return medicalHistory; }
};

class AnimalVictim : public Victim {
private:
    char species[MAX_NAME_LEN];
    char identifyingFeatures[MAX_DESC_LEN];
    char reporterPhone[MAX_PHONE_LEN];
    float estimatedWeightKg;
    bool isAggressiveOrVenomous;

public:
    AnimalVictim(const char* id, const char* spec, const char* features, const char* repPhone,
                 float weight, bool aggressive, Consciousness state, bool breathing, bool trapped, int count = 1);

    bool isHuman() const override { return false; }
    void formatDetails(char* outBuffer, int maxLen) const override;
    void serializeToFile(char* outBuffer, int maxLen) const override;

    float getEstimatedWeight() const { return estimatedWeightKg; }
    bool checkAggressive() const { return isAggressiveOrVenomous; }
};

// ============================================================================
// 6. CÂY PHÂN CẤP ĐỘI CỨU HỘ (CÓ ĐỘI TRƯỞNG, THÀNH VIÊN & DOANH THU DỊCH VỤ)
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

class MedicalTeam : public RescueUnit {
private:
    int ambulanceCount;
    int doctorCount;
public:
    MedicalTeam(const char* id, const char* name, const char* locCode, const char* vehicle,
                const char* leader, const char* leadPhone, int ambulances, int doctors);

    SpecialtyType getSpecialty() const override { return SPEC_MEDICAL; }
    const char* getSpecialtyName() const override { return "Y Tế Cấp Cứu 115"; }
    float evaluateSuitabilityScore(IncidentType incident, const Victim* victim) const override;
    void formatUnitInfo(char* outBuffer, int maxLen) const override;
};

class FireTeam : public RescueUnit {
private:
    float waterCapacityLiters;
    int maxLadderHeightMeters;
public:
    FireTeam(const char* id, const char* name, const char* locCode, const char* vehicle,
             const char* leader, const char* leadPhone, float waterCap, int ladderHeight);

    SpecialtyType getSpecialty() const override { return SPEC_FIRE; }
    const char* getSpecialtyName() const override { return "PCCC & Cứu Nạn 114"; }
    float evaluateSuitabilityScore(IncidentType incident, const Victim* victim) const override;
    void formatUnitInfo(char* outBuffer, int maxLen) const override;
};

class MechanicTeam : public RescueUnit {
private:
    float maxTowCapacityTons;
public:
    MechanicTeam(const char* id, const char* name, const char* locCode, const char* vehicle,
                 const char* leader, const char* leadPhone, float maxTowTons);

    SpecialtyType getSpecialty() const override { return SPEC_MECHANIC; }
    const char* getSpecialtyName() const override { return "Cứu Hộ Giao Thông & Cơ Giới"; }
    float evaluateSuitabilityScore(IncidentType incident, const Victim* victim) const override;
    void formatUnitInfo(char* outBuffer, int maxLen) const override;
};

class AnimalRescueTeam : public RescueUnit {
private:
    int specializedCages;
    bool hasVeterinarian;
public:
    AnimalRescueTeam(const char* id, const char* name, const char* locCode, const char* vehicle,
                     const char* leader, const char* leadPhone, int cages, bool hasVet);

    SpecialtyType getSpecialty() const override { return SPEC_ANIMAL; }
    const char* getSpecialtyName() const override { return "Cứu Hộ Động Vật"; }
    float evaluateSuitabilityScore(IncidentType incident, const Victim* victim) const override;
    void formatUnitInfo(char* outBuffer, int maxLen) const override;
};

// ============================================================================
// 7. LỚP HỒ SƠ SỰ CỐ & HÓA ĐƠN DỊCH VỤ (EMERGENCY CASE & BILLING)
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

#endif // CORE_ENTITIES_H