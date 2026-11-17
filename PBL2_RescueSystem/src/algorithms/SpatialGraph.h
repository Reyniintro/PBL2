#ifndef ALGORITHMS_SPATIAL_GRAPH_H
#define ALGORITHMS_SPATIAL_GRAPH_H

#include "DomainConstants.h"

class RescueUnit;
class EmergencyCase;

// Các hằng số kích thước cấu trúc dữ liệu (Không dùng std::vector hay std::unordered_map)
#define MAX_VERTICES 100       // Tối đa 100 khu vực/phường xã tại Đà Nẵng
#define HASH_TABLE_SIZE 127    // Số nguyên tố giúp phân bố khóa băm đều, giảm đụng độ (Collision)
#define MAX_UNITS_PER_AREA 10  // Số đội cứu hộ tối đa đóng quân tại 1 khu vực
#define INF_TIME 1e9f          // Giá trị vô cùng dùng cho thuật toán Dijkstra

// 1. Cấu trúc Cạnh đồ thị (Tuyến đường nối 2 khu vực lân cận - Danh sách kề)
struct RoadEdge {
    int destIndex;             // Chỉ số đỉnh đích (0 -> vertexCount - 1)
    float travelTimeMinutes;   // Trọng số cơ sở: Thời gian di chuyển dự kiến (phút)
    float trafficFactor;       // Hệ số ùn tắc giao thông (Mặc định = 1.0f, giờ cao điểm có thể > 1.0)
    char roadName[MAX_NAME_LEN]; // Tên tuyến đường chính (VD: "Cau Rong", "Dien Bien Phu")
    RoadEdge* next;            // Con trỏ nối sang tuyến đường tiếp theo cùng xuất phát từ đỉnh này

    RoadEdge(int dest, float time, const char* name, float traffic = 1.0f, RoadEdge* nxt = nullptr)
        : destIndex(dest), travelTimeMinutes(time), trafficFactor(traffic), next(nxt) {
        // Sao chép tên đường an toàn
        if (name) {
            int i = 0;
            while (name[i] != '\0' && i < MAX_NAME_LEN - 1) {
                roadName[i] = name[i];
                i++;
            }
            roadName[i] = '\0';
        } else {
            roadName[0] = '\0';
        }
    }

    // Trả về thời gian thực tế sau khi nhân hệ số ùn tắc
    float getEffectiveTravelTime() const {
        return travelTimeMinutes * trafficFactor;
    }
};

// 2. Cấu trúc Đỉnh đồ thị (Thông tin 1 Khu vực tại Đà Nẵng)
struct AreaNode {
    char areaCode[MAX_ID_LEN];   // Mã khu vực (Khóa chính: VD "HC-01", "ST-01")
    char areaName[MAX_NAME_LEN]; // Tên hành chính sau sáp nhập (VD: "Phuong Hai Chau, Q. Hai Chau")
    RoadEdge* headEdge;          // Con trỏ đầu Danh sách kề các đường đi từ khu vực này

    // Quản lý danh sách Đội cứu hộ đang túc trực tại khu vực này
    RescueUnit* stationedUnits[MAX_UNITS_PER_AREA];
    int unitCount;

    AreaNode();
};

// 3. Cấu trúc Node cho Bảng băm nối kết (Separate Chaining Hash Table)
struct HashEntry {
    char key[MAX_ID_LEN];        // Mã khu vực ("HC-01")
    int vertexIndex;             // Chỉ số ánh xạ vào mảng vertices[]
    HashEntry* next;             // Xử lý đụng độ (Collision) bằng danh sách liên kết đơn

    HashEntry(const char* k, int idx, HashEntry* nxt = nullptr);
};

// 4. Kết quả trả về của thuật toán Điều phối Tuyến đầu (Module 2)
struct DispatchResult {
    RescueUnit* bestUnit;              // Đội cứu hộ tối ưu nhất được chọn
    float estimatedTravelMinutes;      // Thời gian di chuyển ngắn nhất (ETA)
    float suitabilityScore;            // Điểm chuyên môn của đội
    char routeDescription[MAX_DESC_LEN]; // Chuỗi lưu lộ trình đường đi (VD: "TK-01 -> HC-02 -> HC-01")
};

// ============================================================================
// LỚP QUẢN LÝ KHÔNG GIAN & ĐỊNH TUYẾN DIJKSTRA (MODULE 1 & MODULE 2)
// ============================================================================
class SpatialGraph {
private:
    AreaNode vertices[MAX_VERTICES];       // Mảng lưu các đỉnh khu vực
    int vertexCount;                       // Số lượng khu vực hiện có

    HashEntry* hashTable[HASH_TABLE_SIZE]; // Bảng băm tự cài đặt tra cứu mã khu vực O(1)

    // Hàm băm chuỗi DJB2 (Bitwise Polynomial Rolling Hash) - Độ phức tạp O(K) với K <= 12
    int computeHash(const char* key) const;

    // Cấm sao chép nông để tránh lỗi bộ nhớ danh sách liên kết
    SpatialGraph(const SpatialGraph&) = delete;
    SpatialGraph& operator=(const SpatialGraph&) = delete;

public:
    SpatialGraph();
    ~SpatialGraph();

    // --- CÁC PHƯƠNG THỨC MODULE 1: BẢNG BĂM & ĐỒ THỊ KHU VỰC ---
    void clearGraph();
    int addArea(const char* code, const char* name);
    int getAreaIndex(const char* code) const; // Tra cứu O(1) qua Hash Table
    const char* getAreaName(const char* code) const;

    bool addRoad(const char* srcCode, const char* destCode, float travelMinutes, const char* roadName, float trafficFactor = 1.0f, bool isTwoWay = true);
    bool registerUnitToArea(RescueUnit* unit);

    // --- CÁC PHƯƠNG THỨC ĐỌC / GHI FILE BẢN ĐỒ ĐÀ NẴNG ---
    bool loadMapFromFile(const char* filePath);
    bool saveMapToFile(const char* filePath) const;
    void generateDefaultDaNangMap(const char* filePath); // Tự động khởi tạo bản đồ Đà Nẵng (cập nhật các phường sau sáp nhập)

    // --- CÁC PHƯƠNG THỨC MODULE 2: DIJKSTRA & ĐIỀU PHỐI TUYẾN ĐẦU ---
    // Chạy thuật toán Dijkstra tính khoảng cách từ khu vực gặp nạn tới mọi khu vực khác
    void computeShortestPaths(int startIdx, float dist[], int parent[]) const;

    // Tìm đội cứu hộ tối ưu nhất (Kết hợp: Đang rảnh + Đủ thiết bị Bitmask + Điểm chuyên môn + Đường đi ngắn nhất)
    DispatchResult findBestRescueUnit(const EmergencyCase* emergencyCase) const;

    // Truy xuất thông tin phục vụ hiển thị lên bảng Qt Creator sau này
    int getVertexCount() const { return vertexCount; }
    const AreaNode& getAreaByIndex(int idx) const { return vertices[idx]; }
};

#endif // ALGORITHMS_SPATIAL_GRAPH_H
