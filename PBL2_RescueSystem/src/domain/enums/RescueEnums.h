#ifndef RESCUE_ENUMS_H
#define RESCUE_ENUMS_H

// ============================================================================
// HỆ THỐNG ENUM CHUẨN HÓA NGHIỆP VỤ CỨU HỘ & DỊCH VỤ CÓ PHÍ
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

#endif // RESCUE_ENUMS_H
