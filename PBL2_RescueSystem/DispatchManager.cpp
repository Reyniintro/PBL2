#include "DispatchManager.h"
#include <cstdio>
#include <cstring>

// ============================================================================
// 1. THỰC THI HÀNG ĐỢI ƯU TIÊN MAX-HEAP & AGING
// ============================================================================
EmergencyPriorityQueue::EmergencyPriorityQueue()
    : currentSize(0), referenceTime(10, 11, 2026, 8, 30, 0) {
    for (int i = 0; i < MAX_HEAP_CAPACITY; ++i) {
        heapArray[i] = nullptr;
    }
}

EmergencyPriorityQueue::~EmergencyPriorityQueue() {
    clearAll();
}

void EmergencyPriorityQueue::clearAll() {
    for (int i = 0; i < currentSize; ++i) {
        if (heapArray[i] != nullptr) {
            delete heapArray[i];
            heapArray[i] = nullptr;
        }
    }
    currentSize = 0;
}

void EmergencyPriorityQueue::swapNodes(int i, int j) {
    EmergencyCase* temp = heapArray[i];
    heapArray[i] = heapArray[j];
    heapArray[j] = temp;
}

void EmergencyPriorityQueue::heapifyUp(int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        double currScore = heapArray[index]->calculateDynamicPriority(referenceTime);
        double parentScore = heapArray[parent]->calculateDynamicPriority(referenceTime);
        if (currScore > parentScore) {
            swapNodes(index, parent);
            index = parent;
        } else {
            break;
        }
    }
}

void EmergencyPriorityQueue::heapifyDown(int index) {
    while (true) {
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int largest = index;

        double largestScore = heapArray[largest]->calculateDynamicPriority(referenceTime);

        if (left < currentSize) {
            double leftScore = heapArray[left]->calculateDynamicPriority(referenceTime);
            if (leftScore > largestScore) {
                largest = left;
                largestScore = leftScore;
            }
        }
        if (right < currentSize) {
            double rightScore = heapArray[right]->calculateDynamicPriority(referenceTime);
            if (rightScore > largestScore) {
                largest = right;
            }
        }

        if (largest != index) {
            swapNodes(index, largest);
            index = largest;
        } else {
            break;
        }
    }
}

void EmergencyPriorityQueue::updateReferenceTime(const CustomDateTime& now) {
    referenceTime = now;
    rebuildHeapForAging();
}

void EmergencyPriorityQueue::rebuildHeapForAging() {
    for (int i = (currentSize / 2) - 1; i >= 0; --i) {
        heapifyDown(i);
    }
}

bool EmergencyPriorityQueue::push(EmergencyCase* newCase) {
    if (!newCase || currentSize >= MAX_HEAP_CAPACITY) return false;
    heapArray[currentSize] = newCase;
    heapifyUp(currentSize);
    currentSize++;
    return true;
}

EmergencyCase* EmergencyPriorityQueue::popMax() {
    if (currentSize <= 0) return nullptr;
    EmergencyCase* topCase = heapArray[0];
    currentSize--;
    if (currentSize > 0) {
        heapArray[0] = heapArray[currentSize];
        heapArray[currentSize] = nullptr;
        heapifyDown(0);
    } else {
        heapArray[0] = nullptr;
    }
    return topCase;
}

bool EmergencyPriorityQueue::removeCaseByPointer(EmergencyCase* target) {
    if (!target || currentSize <= 0) return false;
    int targetIdx = -1;
    for (int i = 0; i < currentSize; ++i) {
        if (heapArray[i] == target) {
            targetIdx = i;
            break;
        }
    }
    if (targetIdx == -1) return false;

    currentSize--;
    if (targetIdx < currentSize) {
        heapArray[targetIdx] = heapArray[currentSize];
        heapArray[currentSize] = nullptr;
        heapifyUp(targetIdx);
        heapifyDown(targetIdx);
    } else {
        heapArray[targetIdx] = nullptr;
    }
    return true;
}

EmergencyCase* EmergencyPriorityQueue::peekMax() const {
    return (currentSize > 0) ? heapArray[0] : nullptr;
}

// ============================================================================
// 2. THỰC THI BỘ ĐIỀU PHỐI TRUNG TÂM (DISPATCH MANAGER)
// ============================================================================
DispatchManager::DispatchManager()
    : totalUnitCount(0), historyCaseCount(0),
    systemClock(10, 11, 2026, 8, 30, 0), autoCaseCounter(1) {
    for (int i = 0; i < MAX_TOTAL_UNITS; ++i) allUnits[i] = nullptr;
    for (int i = 0; i < MAX_HISTORY_CASES; ++i) activeAndResolvedCases[i] = nullptr;
}

DispatchManager::~DispatchManager() {
    pendingQueue.clearAll();
    for (int i = 0; i < historyCaseCount; ++i) {
        delete activeAndResolvedCases[i];
    }
    for (int i = 0; i < totalUnitCount; ++i) {
        delete allUnits[i];
    }
}

void DispatchManager::initializeSystem(const char* mapFile, const char* unitsFile, const char* casesFile) {
    cityGraph.loadMapFromFile(mapFile);
    pendingQueue.updateReferenceTime(systemClock);
    generateDefaultUnits(unitsFile);
    generateDefaultCases(casesFile);
}

// Khởi tạo 16 Đội cứu hộ quy mô lớn phủ kín Đà Nẵng, có đầy đủ Đội trưởng & Danh sách Thành viên
void DispatchManager::generateDefaultUnits(const char* unitsFile) {
    for (int i = 0; i < totalUnitCount; ++i) delete allUnits[i];
    totalUnitCount = 0;

    auto registerTeam = [&](RescueUnit* u, const char* m1 = nullptr, const char* m2 = nullptr, const char* m3 = nullptr, const char* m4 = nullptr) {
        if (m1) u->addTeamMember(m1);
        if (m2) u->addTeamMember(m2);
        if (m3) u->addTeamMember(m3);
        if (m4) u->addTeamMember(m4);
        allUnits[totalUnitCount++] = u;
        cityGraph.registerUnitToArea(u);
    };

    // --- 4 ĐỘI Y TẾ CẤP CỨU 115 ---
    registerTeam(new MedicalTeam("MED-HC01", "Đội Cấp Cứu 115 Trung Tâm Hải Châu", "HC-01",
                                 "Xe Cấp Cứu Mercedes Sprinter", "BS. Nguyễn Văn Minh", "0905115001", 2, 2),
                 "ĐD. Trần Thị Lan", "ĐD. Lê Hoàng Tuấn", "LX. Phạm Văn Hải", "BS. Võ Minh Đức");

    registerTeam(new MedicalTeam("MED-TK01", "Đội Cấp Cứu 115 Thanh Khê", "TK-01",
                                 "Xe Cấp Cứu Ford Transit ICU", "BS. Lê Quang Huy", "0905115002", 2, 2),
                 "ĐD. Nguyễn Thị Mai", "KTV. Hoàng Văn Nam", "LX. Đặng Quốc Bảo");

    registerTeam(new MedicalTeam("MED-ST03", "Đội Cấp Cứu Biển Mỹ Khê - Sơn Trà", "ST-03",
                                 "Xe Cấp Cứu Hyundai Solati", "BS. Phạm Thanh Tùng", "0905115003", 1, 1),
                 "ĐD. Bùi Thị Tuyết", "ĐD. Ngô Văn Long", "LX. Trịnh Đình Phong");

    registerTeam(new MedicalTeam("MED-CL01", "Đội Cấp Cứu 115 Khu Vực Cẩm Lệ", "CL-01",
                                 "Xe Cấp Cứu Toyota Hiace", "BS. Hoàng Thị Thảo", "0905115004", 1, 2),
                 "ĐD. Phan Văn Khải", "ĐD. Lý Thị Hương", "LX. Vũ Minh Tâm");

    // --- 4 ĐỘI PCCC & CỨU NẠN CỨU HỘ 114 ---
    registerTeam(new FireTeam("FIRE-HC02", "Đội Cảnh Sát PCCC & CNCH Hải Châu", "HC-02",
                              "Xe Thang CNCH Rosenbauer 32m", "Đại úy Trần Quốc Toản", "0905114001", 5000.0f, 32),
                 "Thượng úy Lê Văn Sơn", "Trung úy Nguyễn Hữu Đạt", "Chiến sĩ Phạm Tiến Dũng", "LX. Võ Văn Kiệt");

    registerTeam(new FireTeam("FIRE-LC01", "Đội PCCC & CNCH Liên Chiểu - Hải Vân", "LC-01",
                              "Xe Cứu Hỏa Kamaz Bồn Lớn", "Thiếu tá Nguyễn Đình Cương", "0905114002", 8000.0f, 45),
                 "Đại úy Phan Thanh Bình", "Trung úy Đỗ Minh Quân", "Chiến sĩ Lê Trung Kiên", "LX. Huỳnh Văn Tấn");

    registerTeam(new FireTeam("FIRE-ST01", "Đội PCCC & CNCH Khu Vực Sơn Trà", "ST-01",
                              "Xe CNCH Đa Năng Isuzu", "Đại úy Vũ Hoàng Nam", "0905114003", 4500.0f, 25),
                 "Thượng úy Ngô Quang Vinh", "Trung úy Trần Văn Lộc", "LX. Bùi Xuân Huấn");

    registerTeam(new FireTeam("FIRE-HV01", "Đội PCCC & CNCH Huyện Hòa Vang", "HV-01",
                              "Xe Cứu Hỏa Chuyên Dụng Hino", "Thượng úy Đặng Văn Lâm", "0905114004", 6000.0f, 30),
                 "Trung úy Mai Xuân Hợp", "Chiến sĩ Nguyễn Trọng Hoàng", "LX. Lê Văn Duyệt");

    // --- 5 ĐỘI CỨU HỘ GIAO THÔNG & DỊCH VỤ CƠ GIỚI (KÉO XE, THAY LỐP, KÍCH BÌNH) ---
    registerTeam(new MechanicTeam("MECH-TK03", "Đội Cứu Hộ Giao Thông Ngã Ba Huế", "TK-03",
                                  "Xe Cẩu Kéo Hyundai HD120", "KS. Đinh Trọng Nghĩa", "0905999001", 25.0f),
                 "KTV. Nguyễn Văn Hậu", "KTV. Trần Minh Hiếu", "LX. Phạm Quốc Khánh");

    registerTeam(new MechanicTeam("MECH-CL01", "Đội Cứu Hộ Ô Tô & Thay Lốp Cẩm Lệ", "CL-01",
                                  "Xe Sàn Trượt Hino 300", "KS. Bùi Tiến Dũng", "0905999002", 18.0f),
                 "KTV. Lê Văn Toàn", "KTV. Võ Thành Đạt", "LX. Nguyễn Thanh Sơn");

    registerTeam(new MechanicTeam("MECH-HC04", "Đội Dịch Vụ Lưu Động Nguyễn Tất Thành", "HC-04",
                                  "Xe Cứu Hộ Đa Năng Isuzu", "KS. Trương Văn Thành", "0905999003", 15.0f),
                 "KTV. Phan Hữu Tài", "KTV. Đặng Minh Trí");

    registerTeam(new MechanicTeam("MECH-NHS01", "Đội Cứu Hộ Cơ Giới Ngũ Hành Sơn", "NHS-01",
                                  "Xe Cẩu Hạng Nặng Dongfeng", "KS. Ngô Bá Khá", "0905999004", 35.0f),
                 "KTV. Lý Văn Hùng", "KTV. Trịnh Công Sơn", "LX. Hồ Văn Cường");

    registerTeam(new MechanicTeam("MECH-LC01", "Đội Cứu Hộ Đèo Hải Vân - Liên Chiểu", "LC-01",
                                  "Xe Kéo Hạng Nặng Howo", "KS. Hoàng Xuân Vinh", "0905999005", 40.0f),
                 "KTV. Nguyễn Văn Quyết", "KTV. Đỗ Hùng Dũng", "LX. Đoàn Văn Hậu");

    // --- 3 ĐỘI CỨU HỘ ĐỘNG VẬT & KIỂM LÂM ---
    registerTeam(new AnimalRescueTeam("ANIM-ST02", "Đội Cứu Hộ Động Vật Bán Đảo Sơn Trà", "ST-02",
                                      "Xe Bán Tải Ford Ranger Chuyên Dụng", "BSTY. Lê Thị Thu Thảo", "0905888001", 8, true),
                 "KTV. Nguyễn Minh Vương", "NV. Trần Quốc Tuấn", "LX. Phạm Văn Mách");

    registerTeam(new AnimalRescueTeam("ANIM-HC05", "Đội Kiểm soát & Cứu Hộ Thú Nuôi Hòa Cường", "HC-05",
                                      "Xe Chuyên Dụng Mitsubishi Triton", "BSTY. Trần Anh Khoa", "0905888002", 6, true),
                 "NV. Lê Công Vinh", "NV. Phan Văn Tài Em");

    registerTeam(new AnimalRescueTeam("ANIM-TK02", "Đội Cứu Hộ Động Vật Thanh Khê", "TK-02",
                                      "Xe Bán Tải Mazda BT-50", "Đội trưởng Võ Hoàng Yến", "0905888003", 5, false),
                 "NV. Nguyễn Tuấn Anh", "NV. Lương Xuân Trường");

    saveUnitsToFile(unitsFile);
}

RescueUnit* DispatchManager::addNewRescueUnit(SpecialtyType spec, const char* unitId, const char* unitName,
                                              const char* stationCode, const char* vehicle,
                                              const char* leaderName, const char* leaderPhone) {
    if (totalUnitCount >= MAX_TOTAL_UNITS) return nullptr;

    RescueUnit* newUnit = nullptr;
    if (spec == SPEC_MEDICAL) {
        newUnit = new MedicalTeam(unitId, unitName, stationCode, vehicle, leaderName, leaderPhone, 1, 2);
    } else if (spec == SPEC_FIRE) {
        newUnit = new FireTeam(unitId, unitName, stationCode, vehicle, leaderName, leaderPhone, 5000.0f, 30);
    } else if (spec == SPEC_MECHANIC) {
        newUnit = new MechanicTeam(unitId, unitName, stationCode, vehicle, leaderName, leaderPhone, 20.0f);
    } else {
        newUnit = new AnimalRescueTeam(unitId, unitName, stationCode, vehicle, leaderName, leaderPhone, 6, true);
    }

    allUnits[totalUnitCount++] = newUnit;
    cityGraph.registerUnitToArea(newUnit);
    saveUnitsToFile("RescueUnits.txt");
    return newUnit;
}

bool DispatchManager::saveUnitsToFile(const char* filePath) const {
    if (!filePath) return false;
    FILE* fp = fopen(filePath, "w");
    if (!fp) return false;
    fprintf(fp, "# DANH SACH DOI CUU HO TP. DA NANG\n");
    for (int i = 0; i < totalUnitCount; ++i) {
        char info[256];
        allUnits[i]->formatUnitInfo(info, sizeof(info));
        fprintf(fp, "%s | %s | %s\n", allUnits[i]->getUnitID(), allUnits[i]->getUnitName(), info);
    }
    fclose(fp);
    return true;
}

void DispatchManager::generateDefaultCases(const char* casesFile) {
    // Khởi tạo sẵn 3 ca mẫu và TỰ ĐỘNG ĐIỀU PHỐI NGAY LẬP TỨC để người dùng mở app thấy hệ thống đang hoạt động thực tế
    DispatchResult dummyRes;
    char logBuf[512];

    // Ca 1: Dịch vụ Kéo xe & Thay lốp có phí tại Cầu Rồng (HC-02)
    Victim* v1 = new HumanVictim("VIC-001", "Anh Nguyễn Quốc Tuấn", "0905123456", 1,
                                 CustomDateTime(15, 6, 1988), "Đầu Cầu Rồng, P. Phước Ninh",
                                 "Xe ô tô 7 chỗ nổ lốp và hỏng trục trước cần kéo về gara",
                                 CONSCIOUS_ALERT, true, false, 1);
    reportAndAutoDispatch("HC-02", "Đầu Cầu Rồng, P. Phước Ninh",
                          "Cứu hộ kéo xe ô tô 7 chỗ & thay lốp dự phòng",
                          INCIDENT_SERVICE_TOWING, URGENCY_STANDARD,
                          EQUIP_HEAVY_TOW_CRANE | EQUIP_SPARE_TIRE_KIT, v1, dummyRes, logBuf, sizeof(logBuf));

    // Ca 2: Mắc kẹt song sắt ban công (Cứu hộ khẩn cấp miễn phí)
    Victim* v2 = new HumanVictim("VIC-002", "Cháu Lê Minh Khôi", "0914223344", 1,
                                 CustomDateTime(12, 5, 2020), "Chung cư F-Home, P. Thanh Bình",
                                 "Hoảng loạn, kẹt chân vào lan can sắt",
                                 CONSCIOUS_PANICKED, true, true, 1);
    reportAndAutoDispatch("HC-04", "Tầng 4 Chung cư F-Home, P. Thanh Bình",
                          "Trẻ em mắc kẹt song sắt ban công, cần kìm thủy lực",
                          INCIDENT_TRAPPED_FENCE, URGENCY_URGENT,
                          EQUIP_HYDRAULIC_CUTTER | EQUIP_FIRST_AID_KIT, v2, dummyRes, logBuf, sizeof(logBuf));

    // Ca 3: Dịch vụ Kích bình & Thay lốp tại Mỹ Khê (ST-03) - Đã hoàn thành để có sẵn mẫu Hóa đơn ở Tab 4
    Victim* v3 = new HumanVictim("VIC-003", "Chị Trần Thanh Tâm", "0988776655", 0,
                                 CustomDateTime(20, 9, 1993), "Đường Võ Nguyên Giáp, Biển Mỹ Khê",
                                 "Xe chết bình ắc-quy và xẹp lốp sau",
                                 CONSCIOUS_ALERT, true, false, 1);
    EmergencyCase* c3 = reportAndAutoDispatch("ST-03", "218 Võ Nguyên Giáp, P. Phước Mỹ",
                                              "Thay lốp lưu động & kích bình ắc-quy tận nơi",
                                              INCIDENT_SERVICE_TIRE_BATT, URGENCY_STANDARD,
                                              EQUIP_SPARE_TIRE_KIT | EQUIP_BATTERY_BOOSTER,
                                              v3, dummyRes, logBuf, sizeof(logBuf));
    if (c3) {
        completeActiveCase(c3->getCaseID(), OUTCOME_SAFE_ON_SITE, logBuf, sizeof(logBuf));
    }

    saveCasesToFile(casesFile);
}

bool DispatchManager::saveCasesToFile(const char* filePath) const {
    if (!filePath) return false;
    FILE* fp = fopen(filePath, "w");
    if (!fp) return false;

    fprintf(fp, "# DANH SACH HO SO SU CO & HOA DON CUU HO TP. DA NANG\n");
    for (int i = 0; i < pendingQueue.size(); ++i) {
        char buf[300];
        pendingQueue.getCaseAt(i)->formatCaseSummary(buf, sizeof(buf));
        fprintf(fp, "[PENDING] %s\n", buf);
    }
    for (int i = 0; i < historyCaseCount; ++i) {
        char buf[300];
        activeAndResolvedCases[i]->formatCaseSummary(buf, sizeof(buf));
        fprintf(fp, "[PROCESSED] %s\n", buf);
    }
    fclose(fp);
    return true;
}

// TIẾP NHẬN VÀ TỰ ĐỘNG ĐIỀU PHỐI NGAY LẬP TỨC (ZERO-WAIT AUTO DISPATCH)
EmergencyCase* DispatchManager::reportAndAutoDispatch(const char* locCode, const char* addr, const char* desc,
                                                      IncidentType incType, EmergencyUrgency urgency,
                                                      unsigned int equipMask, Victim* victim,
                                                      DispatchResult& outDispatchRes,
                                                      char* outLogMsg, int maxLogLen) {
    char caseId[MAX_ID_LEN];
    snprintf(caseId, sizeof(caseId), "SOS-%04d", autoCaseCounter++);

    EmergencyCase* newCase = new EmergencyCase(caseId, locCode, addr, desc,
                                               systemClock, incType, urgency, equipMask, victim);

    // Tìm ngay đội cứu hộ gần nhất bằng thuật toán Dijkstra
    outDispatchRes = cityGraph.findBestRescueUnit(newCase);

    if (outDispatchRes.bestUnit != nullptr) {
        // Điều động ngay lập tức không chờ thủ công!
        newCase->assignRescueUnit(outDispatchRes.bestUnit, systemClock, outDispatchRes.estimatedTravelMinutes);
        if (historyCaseCount < MAX_HISTORY_CASES) {
            activeAndResolvedCases[historyCaseCount++] = newCase;
        }

        char eqText[128];
        newCase->formatEquippedItemsText(eqText, sizeof(eqText));

        if (newCase->isPaidServiceCase()) {
            snprintf(outLogMsg, maxLogLen,
                     "⚡ TỰ ĐỘNG ĐIỀU PHỐI NGAY [%s] -> Giao cho [%s] (Đội trưởng: %s) | ETA: %.1f phút | "
                     "Thiết bị mang theo: [%s] | Tạm tính Hóa đơn [%s]: %.0f VNĐ",
                     newCase->getCaseID(), outDispatchRes.bestUnit->getUnitID(),
                     outDispatchRes.bestUnit->getLeaderName(), outDispatchRes.estimatedTravelMinutes,
                     eqText, newCase->getInvoiceID(), newCase->getTotalAmountVND());
        } else {
            snprintf(outLogMsg, maxLogLen,
                     "⚡ TỰ ĐỘNG ĐIỀU PHỐI NGAY [%s] -> Giao cho [%s] (Đội trưởng: %s) | ETA: %.1f phút | "
                     "Thiết bị mang theo: [%s] | Lộ trình: %s",
                     newCase->getCaseID(), outDispatchRes.bestUnit->getUnitID(),
                     outDispatchRes.bestUnit->getLeaderName(), outDispatchRes.estimatedTravelMinutes,
                     eqText, outDispatchRes.routeDescription);
        }
    } else {
        // Chỉ khi tất cả các đội cùng chuyên môn đều đang bận thì mới đưa vào Hàng đợi Max-Heap chờ đội rảnh
        pendingQueue.push(newCase);
        snprintf(outLogMsg, maxLogLen,
                 "⏳ ĐÃ TIẾP NHẬN [%s]: Hiện các đội chuyên môn phù hợp đều đang bận. Hồ sơ đã được xếp vào Hàng đợi ưu tiên để tự động điều phối ngay khi có đội rảnh!",
                 newCase->getCaseID());
    }

    saveCasesToFile("CasesHistory.txt");
    return newCase;
}

DispatchResult DispatchManager::dispatchHighestPriorityCase(char* outLogMsg, int maxLogLen) {
    DispatchResult emptyRes;
    emptyRes.bestUnit = nullptr;
    emptyRes.estimatedTravelMinutes = INF_TIME;
    emptyRes.suitabilityScore = 0.0f;
    emptyRes.routeDescription[0] = '\0';

    int qSize = pendingQueue.size();
    if (qSize <= 0) {
        snprintf(outLogMsg, maxLogLen, "Không có ca nào đang tồn đọng trong hàng đợi chờ.");
        return emptyRes;
    }

    EmergencyCase* candidates[MAX_HEAP_CAPACITY];
    for (int i = 0; i < qSize; ++i) candidates[i] = pendingQueue.getCaseAt(i);
    for (int i = 0; i < qSize - 1; ++i) {
        for (int j = i + 1; j < qSize; ++j) {
            if (candidates[j]->calculateDynamicPriority(systemClock) >
                candidates[i]->calculateDynamicPriority(systemClock)) {
                EmergencyCase* tmp = candidates[i];
                candidates[i] = candidates[j];
                candidates[j] = tmp;
            }
        }
    }

    for (int i = 0; i < qSize; ++i) {
        EmergencyCase* currentCandidate = candidates[i];
        DispatchResult res = cityGraph.findBestRescueUnit(currentCandidate);

        if (res.bestUnit != nullptr) {
            pendingQueue.removeCaseByPointer(currentCandidate);
            currentCandidate->assignRescueUnit(res.bestUnit, systemClock, res.estimatedTravelMinutes);

            if (historyCaseCount < MAX_HISTORY_CASES) {
                activeAndResolvedCases[historyCaseCount++] = currentCandidate;
            }

            snprintf(outLogMsg, maxLogLen,
                     "⚡ TỰ ĐỘNG ĐIỀU PHỐI TỪ HÀNG ĐỢI [%s] -> Đội [%s] | ETA: %.1f phút | Lộ trình: %s",
                     currentCandidate->getCaseID(), res.bestUnit->getUnitID(),
                     res.estimatedTravelMinutes, res.routeDescription);

            saveCasesToFile("CasesHistory.txt");
            return res;
        }
    }

    snprintf(outLogMsg, maxLogLen, "Các đội chuyên môn tương ứng với hàng đợi hiện vẫn đang làm nhiệm vụ.");
    return emptyRes;
}

bool DispatchManager::completeActiveCase(const char* caseId, RescueOutcome outcome, char* outLogMsg, int maxLogLen) {
    for (int i = 0; i < historyCaseCount; ++i) {
        EmergencyCase* c = activeAndResolvedCases[i];
        if (c && strcmp(c->getCaseID(), caseId) == 0 && c->getStatus() == CASE_DISPATCHED) {
            RescueUnit* unit = c->getAssignedUnit();
            const char* unitId = unit ? unit->getUnitID() : "N/A";
            c->completeCase(outcome, systemClock);

            // NGAY KHI ĐỘI VỪA RẢNH: Tự động kiểm tra xem có ca nào đang chờ trong hàng đợi không để giao việc luôn!
            if (!pendingQueue.isEmpty()) {
                char autoDispatchLog[256];
                DispatchResult autoRes = dispatchHighestPriorityCase(autoDispatchLog, sizeof(autoDispatchLog));
                if (autoRes.bestUnit != nullptr) {
                    snprintf(outLogMsg, maxLogLen,
                             "✅ Đã hoàn thành [%s] (Đội [%s] vừa rảnh) -> %s",
                             caseId, unitId, autoDispatchLog);
                    saveCasesToFile("CasesHistory.txt");
                    return true;
                }
            }

            snprintf(outLogMsg, maxLogLen,
                     "✅ HOÀN THÀNH CA [%s]: Đội [%s] đã rút về trạm trực (%s) và sẵn sàng nhận nhiệm vụ mới.",
                     caseId, unitId, unit ? unit->getStationLocationCode() : "");
            saveCasesToFile("CasesHistory.txt");
            return true;
        }
    }
    snprintf(outLogMsg, maxLogLen, "Không tìm thấy ca đang thực thi có mã [%s]!", caseId);
    return false;
}

bool DispatchManager::payServiceInvoice(const char* caseId, char* outLogMsg, int maxLogLen) {
    for (int i = 0; i < historyCaseCount; ++i) {
        EmergencyCase* c = activeAndResolvedCases[i];
        if (c && strcmp(c->getCaseID(), caseId) == 0) {
            if (!c->isPaidServiceCase()) {
                snprintf(outLogMsg, maxLogLen, "Hồ sơ [%s] thuộc diện cứu hộ khẩn cấp miễn phí (0 VNĐ).", caseId);
                return false;
            }
            if (c->getPaymentStatus() == PAYMENT_PAID) {
                snprintf(outLogMsg, maxLogLen, "Hóa đơn [%s] của hồ sơ [%s] đã được thanh toán trước đó!",
                         c->getInvoiceID(), caseId);
                return false;
            }
            if (c->confirmBillPayment()) {
                const char* unitId = c->getAssignedUnit() ? c->getAssignedUnit()->getUnitID() : "N/A";
                snprintf(outLogMsg, maxLogLen,
                         "💳 THANH TOÁN THÀNH CÔNG HÓA ĐƠN [%s] (Ca %s): Số tiền %.0f VNĐ đã được chuyển cho đội [%s]!",
                         c->getInvoiceID(), caseId, c->getTotalAmountVND(), unitId);
                saveCasesToFile("CasesHistory.txt");
                saveUnitsToFile("RescueUnits.txt");
                return true;
            }
        }
    }
    snprintf(outLogMsg, maxLogLen, "Không tìm thấy hồ sơ dịch vụ cần thanh toán!");
    return false;
}

void DispatchManager::advanceSystemTimeMinutes(int minutesToAdd) {
    if (minutesToAdd <= 0) return;
    int totalMin = systemClock.hour * 60 + systemClock.minute + minutesToAdd;
    systemClock.day += totalMin / 1440;
    totalMin %= 1440;
    systemClock.hour = totalMin / 60;
    systemClock.minute = totalMin % 60;

    pendingQueue.updateReferenceTime(systemClock);
}

SystemStatistics DispatchManager::computeStatistics() const {
    SystemStatistics stats;
    memset(&stats, 0, sizeof(stats));
    stats.pendingCasesCount = pendingQueue.size();
    stats.totalCasesReported = stats.pendingCasesCount + historyCaseCount;

    for (int i = 0; i < historyCaseCount; ++i) {
        const EmergencyCase* c = activeAndResolvedCases[i];
        if (!c) continue;

        if (c->isPaidServiceCase()) {
            stats.paidServiceCasesCount++;
            if (c->getPaymentStatus() == PAYMENT_PAID) {
                stats.totalPaidRevenueVND += c->getTotalAmountVND();
            } else {
                stats.totalUnpaidPendingVND += c->getTotalAmountVND();
            }
        }

        if (c->getStatus() == CASE_DISPATCHED || c->getStatus() == CASE_IN_PROGRESS) {
            stats.activeDispatchedCount++;
        } else if (c->getStatus() == CASE_RESOLVED) {
            stats.resolvedCasesCount++;
            const Victim* v = c->getVictim();
            if (v) {
                int count = v->getVictimCount();
                if (v->isHuman()) stats.totalHumansRescued += count;
                else stats.totalAnimalsRescued += count;

                if (v->getOutcome() == OUTCOME_SAFE_ON_SITE) stats.outcomeSafeCount += count;
                else if (v->getOutcome() == OUTCOME_HOSPITALIZED) stats.outcomeHospitalizedCount += count;
                else if (v->getOutcome() == OUTCOME_DECEASED) stats.outcomeDeceasedCount += count;
            }
        }
    }
    return stats;
}