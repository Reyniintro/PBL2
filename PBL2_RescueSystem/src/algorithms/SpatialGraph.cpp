#include "SpatialGraph.h"
#include "RescueUnit.h"
#include "EmergencyCase.h"
#include "Victim.h"
#include "RescueEnums.h"
#include <cstdio>
#include <cstring>

static void safeCopyStrGraph(char* dest, const char* src, int maxLen) {
    if (!dest || maxLen <= 0) return;
    if (!src) {
        dest[0] = '\0';
        return;
    }
    strncpy(dest, src, maxLen - 1);
    dest[maxLen - 1] = '\0';
}

// ============================================================================
// 1. KHỞI TẠO CÁC CẤU TRÚC NODE CƠ BẢN (ĐỈNH & BẢNG BĂM)
// ============================================================================
AreaNode::AreaNode() : headEdge(nullptr), unitCount(0) {
    areaCode[0] = '\0';
    areaName[0] = '\0';
    for (int i = 0; i < MAX_UNITS_PER_AREA; ++i) {
        stationedUnits[i] = nullptr;
    }
}

HashEntry::HashEntry(const char* k, int idx, HashEntry* nxt)
    : vertexIndex(idx), next(nxt) {
    safeCopyStrGraph(key, k, MAX_ID_LEN);
}

// ============================================================================
// 2. QUẢN LÝ BỘ NHỚ ĐỒ THỊ & BẢNG BĂM (MODULE 1)
// ============================================================================
SpatialGraph::SpatialGraph() : vertexCount(0) {
    for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
        hashTable[i] = nullptr;
    }
}

SpatialGraph::~SpatialGraph() {
    clearGraph();
}

void SpatialGraph::clearGraph() {
    for (int i = 0; i < vertexCount; ++i) {
        RoadEdge* curr = vertices[i].headEdge;
        while (curr != nullptr) {
            RoadEdge* temp = curr;
            curr = curr->next;
            delete temp;
        }
        vertices[i].headEdge = nullptr;
        vertices[i].unitCount = 0;
        for (int j = 0; j < MAX_UNITS_PER_AREA; ++j) {
            vertices[i].stationedUnits[j] = nullptr;
        }
    }
    vertexCount = 0;

    for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
        HashEntry* curr = hashTable[i];
        while (curr != nullptr) {
            HashEntry* temp = curr;
            curr = curr->next;
            delete temp;
        }
        hashTable[i] = nullptr;
    }
}

int SpatialGraph::computeHash(const char* key) const {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return (int)(hash % HASH_TABLE_SIZE);
}

int SpatialGraph::getAreaIndex(const char* code) const {
    if (!code || code[0] == '\0') return -1;
    int bucket = computeHash(code);
    HashEntry* curr = hashTable[bucket];
    while (curr != nullptr) {
        if (strcmp(curr->key, code) == 0) {
            return curr->vertexIndex;
        }
        curr = curr->next;
    }
    return -1;
}

const char* SpatialGraph::getAreaName(const char* code) const {
    int idx = getAreaIndex(code);
    if (idx < 0 || idx >= vertexCount) return "Khu vuc khong xac dinh";
    return vertices[idx].areaName;
}

int SpatialGraph::addArea(const char* code, const char* name) {
    if (!code || code[0] == '\0') return -1;

    int existingIdx = getAreaIndex(code);
    if (existingIdx != -1) {
        safeCopyStrGraph(vertices[existingIdx].areaName, name, MAX_NAME_LEN);
        return existingIdx;
    }

    if (vertexCount >= MAX_VERTICES) return -1;

    int newIdx = vertexCount++;
    safeCopyStrGraph(vertices[newIdx].areaCode, code, MAX_ID_LEN);
    safeCopyStrGraph(vertices[newIdx].areaName, name, MAX_NAME_LEN);
    vertices[newIdx].headEdge = nullptr;
    vertices[newIdx].unitCount = 0;

    int bucket = computeHash(code);
    hashTable[bucket] = new HashEntry(code, newIdx, hashTable[bucket]);

    return newIdx;
}

bool SpatialGraph::addRoad(const char* srcCode, const char* destCode, float travelMinutes,
                           const char* roadName, float trafficFactor, bool isTwoWay) {
    int u = getAreaIndex(srcCode);
    int v = getAreaIndex(destCode);
    if (u == -1 || v == -1 || u == v || travelMinutes <= 0.0f) return false;

    if (trafficFactor <= 0.0f) trafficFactor = 1.0f;

    vertices[u].headEdge = new RoadEdge(v, travelMinutes, roadName, trafficFactor, vertices[u].headEdge);
    if (isTwoWay) {
        vertices[v].headEdge = new RoadEdge(u, travelMinutes, roadName, trafficFactor, vertices[v].headEdge);
    }
    return true;
}

bool SpatialGraph::registerUnitToArea(RescueUnit* unit) {
    if (!unit) return false;
    int idx = getAreaIndex(unit->getStationLocationCode());
    if (idx == -1) return false;

    AreaNode& area = vertices[idx];
    if (area.unitCount >= MAX_UNITS_PER_AREA) return false;

    for (int i = 0; i < area.unitCount; ++i) {
        if (area.stationedUnits[i] == unit) return true;
    }

    area.stationedUnits[area.unitCount++] = unit;
    return true;
}

// ============================================================================
// 3. THUẬT TOÁN DIJKSTRA & ĐIỀU PHỐI TỰ ĐỘNG VỚI KHO THIẾT BỊ CHUNG (MODULE 2)
// ============================================================================
void SpatialGraph::computeShortestPaths(int startIdx, float dist[], int parent[]) const {
    bool visited[MAX_VERTICES];

    for (int i = 0; i < vertexCount; ++i) {
        dist[i] = INF_TIME;
        parent[i] = -1;
        visited[i] = false;
    }

    if (startIdx < 0 || startIdx >= vertexCount) return;
    dist[startIdx] = 0.0f;

    for (int count = 0; count < vertexCount; ++count) {
        int u = -1;
        float minDist = INF_TIME;
        for (int i = 0; i < vertexCount; ++i) {
            if (!visited[i] && dist[i] < minDist) {
                minDist = dist[i];
                u = i;
            }
        }

        if (u == -1 || minDist >= INF_TIME) break;
        visited[u] = true;

        RoadEdge* edge = vertices[u].headEdge;
        while (edge != nullptr) {
            int v = edge->destIndex;
            float weight = edge->getEffectiveTravelTime();
            if (!visited[v] && dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                parent[v] = u;
            }
            edge = edge->next;
        }
    }
}

static const char* findRoadNameBetween(const AreaNode& srcNode, int destIdx) {
    RoadEdge* edge = srcNode.headEdge;
    while (edge != nullptr) {
        if (edge->destIndex == destIdx) {
            return edge->roadName;
        }
        edge = edge->next;
    }
    return "Tuyen noi bo";
}

DispatchResult SpatialGraph::findBestRescueUnit(const EmergencyCase* emergencyCase) const {
    DispatchResult result;
    result.bestUnit = nullptr;
    result.estimatedTravelMinutes = INF_TIME;
    result.suitabilityScore = -1.0f;
    safeCopyStrGraph(result.routeDescription, "Tat ca cac doi phu hop hien dang ban lam nhiem vu.", MAX_DESC_LEN);

    if (!emergencyCase) return result;

    int incidentIdx = getAreaIndex(emergencyCase->getLocationCode());
    if (incidentIdx == -1) {
        safeCopyStrGraph(result.routeDescription, "Ma khu vuc khong ton tai tren ban do!", MAX_DESC_LEN);
        return result;
    }

    float dist[MAX_VERTICES];
    int parent[MAX_VERTICES];
    computeShortestPaths(incidentIdx, dist, parent);

    int bestStationIdx = -1;
    float bestCombinedMetric = -1e9f;

    IncidentType incType = emergencyCase->getIncidentType();
    const Victim* victim = emergencyCase->getVictim();

    // Duyệt toàn bộ các khu vực có đội cứu hộ đóng quân
    // LƯU Ý: Thiết bị chuyên dụng lấy từ KHO DÙNG CHUNG nên không ràng buộc cứng vào đội!
    for (int i = 0; i < vertexCount; ++i) {
        if (dist[i] >= INF_TIME) continue;

        const AreaNode& area = vertices[i];
        for (int j = 0; j < area.unitCount; ++j) {
            RescueUnit* unit = area.stationedUnits[j];
            if (!unit) continue;

            // Điều kiện 1: Đội phải đang RẢNH (UNIT_AVAILABLE)
            if (unit->getStatus() != UNIT_AVAILABLE) continue;

            // Điều kiện 2: Đánh giá điểm chuyên môn Đa hình (Nếu < 0 nghĩa là trái ngành hoàn toàn)
            float suitScore = unit->evaluateSuitabilityScore(incType, victim);
            if (suitScore < 0.0f) continue;

            float travelTime = dist[i];
            // Ưu tiên đội có chuyên môn cao nhất và thời gian di chuyển Dijkstra ngắn nhất
            float combinedMetric = suitScore - (travelTime * 4.5f);

            if (combinedMetric > bestCombinedMetric) {
                bestCombinedMetric = combinedMetric;
                result.bestUnit = unit;
                result.estimatedTravelMinutes = travelTime;
                result.suitabilityScore = suitScore;
                bestStationIdx = i;
            }
        }
    }

    if (result.bestUnit != nullptr && bestStationIdx != -1) {
        if (bestStationIdx == incidentIdx) {
            snprintf(result.routeDescription, MAX_DESC_LEN,
                     "Tai cho (%s) - Tiep can tuc thi (0.5 phut)",
                     vertices[incidentIdx].areaCode);
            result.estimatedTravelMinutes = 0.5f;
        } else {
            char routeBuf[MAX_DESC_LEN];
            snprintf(routeBuf, sizeof(routeBuf), "%s", vertices[bestStationIdx].areaCode);

            int curr = bestStationIdx;
            while (curr != incidentIdx && parent[curr] != -1) {
                int nextNode = parent[curr];
                const char* rName = findRoadNameBetween(vertices[curr], nextNode);

                char stepBuf[128];
                snprintf(stepBuf, sizeof(stepBuf), " --[%s]--> %s", rName, vertices[nextNode].areaCode);

                int currentLen = (int)strlen(routeBuf);
                if (currentLen + (int)strlen(stepBuf) < MAX_DESC_LEN - 1) {
                    strcat(routeBuf, stepBuf);
                }
                curr = nextNode;
            }
            safeCopyStrGraph(result.routeDescription, routeBuf, MAX_DESC_LEN);
        }
    }

    return result;
}

// ============================================================================
// 4. ĐỌC / GHI FILE BẢN ĐỒ ĐÀ NẴNG
// ============================================================================
bool SpatialGraph::saveMapToFile(const char* filePath) const {
    if (!filePath) return false;
    FILE* fp = fopen(filePath, "w");
    if (!fp) return false;

    fprintf(fp, "# BAN DO MANG LUOI GIAO THONG CUU HO TP. DA NANG\n");
    for (int i = 0; i < vertexCount; ++i) {
        fprintf(fp, "AREA|%s|%s\n", vertices[i].areaCode, vertices[i].areaName);
    }
    fprintf(fp, "\n");

    for (int u = 0; u < vertexCount; ++u) {
        RoadEdge* edge = vertices[u].headEdge;
        while (edge != nullptr) {
            int v = edge->destIndex;
            if (u < v) {
                fprintf(fp, "ROAD|%s|%s|%.1f|%.2f|%s\n",
                        vertices[u].areaCode, vertices[v].areaCode,
                        edge->travelTimeMinutes, edge->trafficFactor, edge->roadName);
            }
            edge = edge->next;
        }
    }

    fclose(fp);
    return true;
}

bool SpatialGraph::loadMapFromFile(const char* filePath) {
    // Luôn khởi tạo chuẩn bộ 15 khu vực Đà Nẵng với tên sạch đẹp
    generateDefaultDaNangMap(filePath);
    return vertexCount > 0;
}

void SpatialGraph::generateDefaultDaNangMap(const char* filePath) {
    clearGraph();

    // 15 Khu vực thực tế phân bổ theo địa lý tự nhiên của TP. Đà Nẵng (Tây Bắc - Trung Tâm - Đông Sông Hàn - Nam)
    addArea("LC-01", "P. Hoa Khanh Bac, Q. Lien Chieu");
    addArea("TK-02", "P. Xuan Ha, Q. Thanh Khe");
    addArea("TK-01", "P. Thac Gian, Q. Thanh Khe");
    addArea("TK-03", "P. Chinh Gian (Nga Ba Hue), Q. Thanh Khe");

    addArea("HC-04", "P. Thanh Binh, Q. Hai Chau");
    addArea("HC-01", "P. Hai Chau (Trung Tam), Q. Hai Chau");
    addArea("HC-02", "P. Phuoc Ninh (Cau Rong), Q. Hai Chau");
    addArea("HC-03", "P. Binh Thuan, Q. Hai Chau");
    addArea("HC-05", "P. Hoa Cuong Bac, Q. Hai Chau");

    addArea("CL-01", "P. Khue Trung, Q. Cam Le");
    addArea("HV-01", "Xa Hoa Phong, H. Hoa Vang");

    addArea("ST-02", "P. An Hai Bac, Q. Son Tra");
    addArea("ST-01", "P. An Hai Nam, Q. Son Tra");
    addArea("ST-03", "P. Phuoc My (Bien My Khe), Q. Son Tra");
    addArea("NHS-01", "P. My An, Q. Ngu Hanh Son");

    // Các trục đường thực tế và cầu bắc qua Sông Hàn
    addRoad("HC-01", "HC-02", 3.0f, "Duong Nguyen Van Linh", 1.0f);
    addRoad("HC-01", "HC-04", 3.5f, "Duong Nguyen Tat Thanh", 1.0f);
    addRoad("HC-01", "TK-01", 4.0f, "Duong Hung Vuong", 1.1f);
    addRoad("HC-01", "ST-02", 3.5f, "Cau Song Han", 1.0f);

    addRoad("HC-02", "HC-03", 2.5f, "Duong Trung Nu Vuong", 1.0f);
    addRoad("HC-02", "ST-01", 3.0f, "Cau Rong", 1.1f);
    addRoad("HC-03", "HC-05", 3.5f, "Duong 2 Thang 9", 1.0f);
    addRoad("HC-05", "CL-01", 4.0f, "Duong Cach Mang Thang 8", 1.0f);
    addRoad("HC-05", "ST-01", 3.5f, "Cau Tran Thi Ly", 1.0f);

    addRoad("TK-01", "TK-02", 3.0f, "Duong Ha Huy Tap", 1.0f);
    addRoad("TK-01", "TK-03", 2.5f, "Duong Dien Bien Phu", 1.2f);
    addRoad("TK-03", "LC-01", 6.5f, "Truc Ton Duc Thang - Nga Ba Hue", 1.1f);
    addRoad("TK-03", "CL-01", 5.5f, "Duong Truong Chinh", 1.1f);

    addRoad("ST-01", "ST-02", 3.0f, "Duong Ngo Quyen", 1.0f);
    addRoad("ST-01", "ST-03", 2.5f, "Duong Vo Van Kiet", 1.0f);
    addRoad("ST-01", "NHS-01", 4.5f, "Duong Le Van Hien", 1.0f);

    addRoad("CL-01", "HV-01", 9.0f, "Quoc Lo 14B", 1.0f);
    addRoad("CL-01", "NHS-01", 5.0f, "Cau Tien Son", 1.0f);

    saveMapToFile(filePath);
}
