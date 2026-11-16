#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QPainter>
#include <QPainterPath>
#include <QMessageBox>
#include <QFrame>
#include <QAbstractItemView>
#include <QDateTime>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>

static QString formatCurrencyVND(double amount) {
    if (amount <= 0.0) return "0 VNĐ (Miễn phí)";
    long val = (long)(amount + 0.5);
    QString s = QString::number(val);
    int pos = s.length() - 3;
    while (pos > 0) {
        s.insert(pos, '.');
        pos -= 3;
    }
    return s + " VNĐ";
}

// ============================================================================
// 0. HỘP THOẠI ĐĂNG NHẬP TỔNG ĐÀI VIÊN (ĐỌC/GHI FILE AdminAccounts.txt)
// ============================================================================
LoginDialog::LoginDialog(QWidget* parent)
    : QDialog(parent), staffCount(0), authenticatedAdmin(nullptr) {
    for (int i = 0; i < MAX_DISPATCHERS; ++i) staffList[i] = nullptr;
    loadOrGenerateAccounts("AdminAccounts.txt");

    setWindowTitle("Đăng Nhập Hệ Thống Điều Phối Cứu Hộ TP. Đà Nẵng");
    setFixedSize(460, 420);

    setStyleSheet(
        "QDialog { background-color: #F8FAFC; font-family: 'Segoe UI'; }"
        "QFrame#LoginCard { background-color: #FFFFFF; border: 1px solid #CBD5E1; border-radius: 12px; }"
        "QLabel { color: #1E293B; font-size: 12px; font-weight: 600; background: transparent; }"
        "QLineEdit, QComboBox { background-color: #F8FAFC; border: 1px solid #94A3B8; border-radius: 6px; "
        "padding: 7px 10px; color: #0F172A; font-size: 13px; font-weight: 600; }"
        "QLineEdit:focus, QComboBox:focus { border: 2px solid #0284C7; background-color: #FFFFFF; }"
        "QComboBox QAbstractItemView { background-color: #FFFFFF; color: #0F172A; selection-background-color: #0284C7; selection-color: #FFFFFF; }"
        "QPushButton#BtnLogin { background-color: #0284C7; color: #FFFFFF; border-radius: 8px; "
        "padding: 10px; font-size: 13px; font-weight: 800; border: none; }"
        "QPushButton#BtnLogin:hover { background-color: #0369A1; }"
        );

    QVBoxLayout* outerLay = new QVBoxLayout(this);
    outerLay->setContentsMargins(24, 24, 24, 24);

    QFrame* card = new QFrame();
    card->setObjectName("LoginCard");
    QVBoxLayout* cardLay = new QVBoxLayout(card);
    cardLay->setContentsMargins(26, 22, 26, 22);
    cardLay->setSpacing(10);

    QHBoxLayout* headerRow = new QHBoxLayout();
    QPixmap badgePix(44, 44);
    badgePix.fill(Qt::transparent);
    {
        QPainter p(&badgePix);
        p.setRenderHint(QPainter::Antialiasing);
        p.setBrush(QColor("#EF4444"));
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(0, 0, 44, 44, 10, 10);
        p.setBrush(Qt::white);
        p.drawRect(18, 9, 8, 26);
        p.drawRect(9, 18, 26, 8);
    }
    QLabel* lblIcon = new QLabel();
    lblIcon->setPixmap(badgePix);

    QVBoxLayout* titleCol = new QVBoxLayout();
    titleCol->setSpacing(2);
    QLabel* lblTitle = new QLabel("TRUNG TÂM ĐIỀU PHỐI 114/115");
    lblTitle->setStyleSheet("color: #0F172A; font-size: 16px; font-weight: 800;");
    QLabel* lblSub = new QLabel("Xác thực Tổng đài viên trực ban (Admin Portal)");
    lblSub->setStyleSheet("color: #64748B; font-size: 11px; font-weight: 600;");
    titleCol->addWidget(lblTitle);
    titleCol->addWidget(lblSub);

    headerRow->addWidget(lblIcon);
    headerRow->addSpacing(10);
    headerRow->addLayout(titleCol, 1);
    cardLay->addLayout(headerRow);
    cardLay->addSpacing(6);

    cardLay->addWidget(new QLabel("⚡ Chọn nhanh tài khoản Ca trực:"));
    cboQuickAccount = new QComboBox();
    for (int i = 0; i < staffCount; ++i) {
        cboQuickAccount->addItem(QString("[%1] %2 (%3)")
                                     .arg(staffList[i]->getStaffID(), staffList[i]->getFullName(), staffList[i]->getShiftCode()));
    }
    connect(cboQuickAccount, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LoginDialog::onQuickAccountSelected);
    cardLay->addWidget(cboQuickAccount);

    cardLay->addWidget(new QLabel("👤 Mã Nhân Viên (Staff ID):"));
    txtUsername = new QLineEdit();
    cardLay->addWidget(txtUsername);

    cardLay->addWidget(new QLabel("🔒 Mật khẩu (Password):"));
    txtPassword = new QLineEdit();
    txtPassword->setEchoMode(QLineEdit::Password);
    cardLay->addWidget(txtPassword);

    lblErrorMsg = new QLabel("");
    lblErrorMsg->setStyleSheet("color: #DC2626; font-size: 11px; font-weight: bold;");
    lblErrorMsg->setVisible(false);
    cardLay->addWidget(lblErrorMsg);

    cardLay->addStretch();

    QPushButton* btnLogin = new QPushButton("🔐 ĐĂNG NHẬP VÀO CA TRỰC");
    btnLogin->setObjectName("BtnLogin");
    btnLogin->setCursor(Qt::PointingHandCursor);
    connect(btnLogin, &QPushButton::clicked, this, &LoginDialog::onLoginButtonClicked);
    cardLay->addWidget(btnLogin);

    outerLay->addWidget(card);

    if (staffCount > 0) {
        onQuickAccountSelected(0);
    }
}

LoginDialog::~LoginDialog() {
    for (int i = 0; i < staffCount; ++i) {
        delete staffList[i];
    }
}

void LoginDialog::loadOrGenerateAccounts(const char* filePath) {
    FILE* fp = fopen(filePath, "r");
    if (!fp) {
        fp = fopen(filePath, "w");
        if (fp) {
            fprintf(fp, "# DANH SACH TAI KHOAN TONG DAI VIEN (ADMIN)\n");
            fprintf(fp, "ADMIN-01|123456|Nguyen Hai Nam|0905111222|CA SANG\n");
            fprintf(fp, "ADMIN-02|123456|Tran Thi Thu Ha|0905333444|CA CHIEU\n");
            fprintf(fp, "ADMIN-03|admin114|Le Hoang Minh|0905888999|CA DEM\n");
            fclose(fp);
        }
        fp = fopen(filePath, "r");
    }

    if (!fp) return;
    char line[256];
    while (fgets(line, sizeof(line), fp) && staffCount < MAX_DISPATCHERS) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0' || line[0] == '#') continue;

        char id[MAX_ID_LEN] = {0};
        char pass[32] = {0};
        char name[MAX_NAME_LEN] = {0};
        char phone[MAX_PHONE_LEN] = {0};
        char shift[MAX_ID_LEN] = {0};

        if (sscanf(line, "%13[^|]|%31[^|]|%63[^|]|%15[^|]|%13[^\n]", id, pass, name, phone, shift) == 5) {
            staffList[staffCount++] = new Dispatcher(id, pass, name, phone, 1, CustomDateTime(1, 1, 1995), shift);
        }
    }
    fclose(fp);
}

void LoginDialog::onQuickAccountSelected(int index) {
    if (index < 0 || index >= staffCount) return;
    txtUsername->setText(staffList[index]->getStaffID());
    if (strcmp(staffList[index]->getStaffID(), "ADMIN-03") == 0) {
        txtPassword->setText("admin114");
    } else {
        txtPassword->setText("123456");
    }
    lblErrorMsg->setVisible(false);
}

void LoginDialog::onLoginButtonClicked() {
    QByteArray userBytes = txtUsername->text().trimmed().toUtf8();
    QByteArray passBytes = txtPassword->text().toUtf8();

    for (int i = 0; i < staffCount; ++i) {
        if (strcmp(staffList[i]->getStaffID(), userBytes.constData()) == 0 &&
            staffList[i]->verifyPassword(passBytes.constData())) {
            authenticatedAdmin = staffList[i];
            accept();
            return;
        }
    }

    lblErrorMsg->setText("❌ Sai Mã nhân viên hoặc Mật khẩu! Vui lòng kiểm tra lại.");
    lblErrorMsg->setVisible(true);
}

// ============================================================================
// 1. CANVAS BẢN ĐỒ ĐÀ NẴNG (CÓ CĂN CỨ TỌA ĐỘ ĐỊA LÝ KM & THƯỚC TỶ LỆ RÕ RÀNG)
// ============================================================================
struct NodeCoord {
    const char* code;
    float normX; // Tương ứng tọa độ Đông-Tây (0.0 -> 10.0 km)
    float normY; // Tương ứng tọa độ Bắc-Nam (0.0 -> 10.0 km)
};

// Căn cứ tọa độ thực tế tại Đà Nẵng:
// - Phía Tây Bắc (X: 1-4km, Y: 1-4km): Liên Chiểu (LC-01) & Thanh Khê (TK-01, TK-02, TK-03)
// - Trung tâm Bờ Tây Sông Hàn (X: 4.5-6km): Hải Châu (HC-04, HC-01, HC-02, HC-03, HC-05)
// - Phía Tây Nam (X: 1-3.5km, Y: 7.5-9km): Cẩm Lệ (CL-01) & Hòa Vang (HV-01)
// - Bán đảo phía Đông Sông Hàn (X: 7.5-9km): Sơn Trà (ST-02, ST-01, ST-03) & Ngũ Hành Sơn (NHS-01)
static const NodeCoord DA_NANG_COORDS[] = {
    {"LC-01",  0.15f, 0.18f},
    {"TK-02",  0.33f, 0.23f},
    {"TK-01",  0.40f, 0.38f},
    {"TK-03",  0.24f, 0.46f},
    {"HC-04",  0.53f, 0.18f},
    {"HC-01",  0.55f, 0.34f},
    {"HC-02",  0.56f, 0.51f},
    {"HC-03",  0.47f, 0.64f},
    {"HC-05",  0.56f, 0.78f},
    {"CL-01",  0.35f, 0.77f},
    {"HV-01",  0.15f, 0.85f},
    {"ST-02",  0.76f, 0.25f},
    {"ST-01",  0.75f, 0.50f},
    {"ST-03",  0.89f, 0.44f},
    {"NHS-01", 0.80f, 0.79f}
};

DaNangMapCanvas::DaNangMapCanvas(const SpatialGraph* g, QWidget* parent)
    : QWidget(parent), graph(g) {
    setMinimumSize(450, 350);
    clearHighlight();
}

void DaNangMapCanvas::setHighlightRoute(const char* routeDesc, const char* stationCode, const char* incidentCode) {
    if (routeDesc) strncpy(highlightedRoute, routeDesc, MAX_DESC_LEN - 1);
    if (stationCode) strncpy(activeStationArea, stationCode, MAX_ID_LEN - 1);
    if (incidentCode) strncpy(activeIncidentArea, incidentCode, MAX_ID_LEN - 1);
    update();
}

void DaNangMapCanvas::clearHighlight() {
    highlightedRoute[0] = '\0';
    activeIncidentArea[0] = '\0';
    activeStationArea[0] = '\0';
    update();
}

static QPointF getScreenPos(const char* code, int w, int h) {
    int count = sizeof(DA_NANG_COORDS) / sizeof(DA_NANG_COORDS[0]);
    for (int i = 0; i < count; ++i) {
        if (strcmp(DA_NANG_COORDS[i].code, code) == 0) {
            return QPointF(DA_NANG_COORDS[i].normX * w, DA_NANG_COORDS[i].normY * h);
        }
    }
    return QPointF(w * 0.5f, h * 0.5f);
}

void DaNangMapCanvas::paintEvent(QPaintEvent* event) {
    (void)event;
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    int w = width();
    int h = height();

    // 1. Nền bản đồ GIS Sáng
    QLinearGradient bgGrad(0, 0, w, h);
    bgGrad.setColorAt(0.0, QColor("#F8FAFC"));
    bgGrad.setColorAt(1.0, QColor("#E2E8F0"));
    p.fillRect(rect(), bgGrad);

    // 2. Vịnh Đà Nẵng (Phía Bắc) & Biển Mỹ Khê (Phía Đông)
    QPainterPath seaPath;
    seaPath.moveTo(w * 0.30, 0);
    seaPath.quadTo(w * 0.58, h * 0.11, w * 0.80, 0);
    seaPath.lineTo(w, 0);
    seaPath.lineTo(w, h);
    seaPath.lineTo(w * 0.93, h);
    seaPath.quadTo(w * 0.91, h * 0.50, w * 0.80, 0);
    p.fillPath(seaPath, QColor(186, 230, 253, 120));

    // 3. Hệ trục lưới tọa độ Km (Căn cứ địa lý rõ ràng: Mỗi ô lưới = 2 km)
    p.setPen(QPen(QColor("#CBD5E1"), 1, Qt::DotLine));
    p.setFont(QFont("Segoe UI", 7));
    for (int i = 1; i <= 4; ++i) {
        int x = (w * i) / 5;
        int y = (h * i) / 5;
        p.drawLine(x, 0, x, h);
        p.drawLine(0, y, w, y);
        p.setPen(QColor("#94A3B8"));
        p.drawText(x + 3, 11, QString("%1km").arg(i * 2));
        p.drawText(3, y - 3, QString("%1km").arg(i * 2));
        p.setPen(QPen(QColor("#CBD5E1"), 1, Qt::DotLine));
    }

    // 4. Dòng Sông Hàn phân giới tự nhiên Tây (Hải Châu) và Đông (Sơn Trà - Ngũ Hành Sơn)
    QPainterPath hanRiver;
    hanRiver.moveTo(w * 0.65, 0);
    hanRiver.cubicTo(w * 0.63, h * 0.35, w * 0.66, h * 0.60, w * 0.63, h);
    p.setPen(QPen(QColor(56, 189, 248, 130), 16, Qt::SolidLine, Qt::RoundCap));
    p.drawPath(hanRiver);

    // Nhãn phân vùng địa lý chìm dễ hiểu
    p.setFont(QFont("Segoe UI", 8, QFont::Bold));
    p.setPen(QColor(100, 116, 139, 130));
    p.drawText(w * 0.10, h * 0.08, "KHU VỰC TÂY BẮC (LIÊN CHIỂU - THANH KHÊ)");
    p.drawText(w * 0.67, h * 0.14, "BÁN ĐẢO SƠN TRÀ & BIỂN ĐÔNG");
    p.drawText(w * 0.61, h * 0.92, "SÔNG HÀN");

    // Chú giải màu (Legend) & Thước tỷ lệ Scale Bar ở đáy bản đồ
    p.setFont(QFont("Segoe UI", 8, QFont::DemiBold));
    p.setBrush(QColor("#0284C7")); p.setPen(Qt::NoPen); p.drawEllipse(12, h - 22, 9, 9);
    p.setPen(QColor("#1E293B")); p.drawText(25, h - 14, "Trạm có đội rảnh");

    p.setBrush(QColor("#16A34A")); p.setPen(Qt::NoPen); p.drawEllipse(125, h - 22, 9, 9);
    p.setPen(QColor("#1E293B")); p.drawText(138, h - 14, "Trạm xuất phát");

    p.setBrush(QColor("#DC2626")); p.setPen(Qt::NoPen); p.drawEllipse(235, h - 22, 9, 9);
    p.setPen(QColor("#1E293B")); p.drawText(248, h - 14, "Hiện trường SOS");

    // Vẽ thước tỷ lệ (Scale bar: 1 ô = 2km)
    int scaleW = w / 5;
    p.setPen(QPen(QColor("#334155"), 2));
    p.drawLine(w - scaleW - 20, h - 16, w - 20, h - 16);
    p.drawLine(w - scaleW - 20, h - 20, w - scaleW - 20, h - 12);
    p.drawLine(w - 20, h - 20, w - 20, h - 12);
    p.setFont(QFont("Segoe UI", 7, QFont::Bold));
    p.drawText(w - scaleW / 2 - 35, h - 20, "Tỷ lệ: 2 km");

    if (!graph) return;

    // 5. Vẽ các Tuyến đường (Edges)
    int vCount = graph->getVertexCount();
    for (int u = 0; u < vCount; ++u) {
        const AreaNode& nodeU = graph->getAreaByIndex(u);
        QPointF posU = getScreenPos(nodeU.areaCode, w, h);

        RoadEdge* edge = nodeU.headEdge;
        while (edge != nullptr) {
            int v = edge->destIndex;
            if (u < v) {
                const AreaNode& nodeV = graph->getAreaByIndex(v);
                QPointF posV = getScreenPos(nodeV.areaCode, w, h);

                bool isHighlighted = (strstr(highlightedRoute, nodeU.areaCode) != nullptr &&
                                      strstr(highlightedRoute, nodeV.areaCode) != nullptr &&
                                      strstr(highlightedRoute, edge->roadName) != nullptr);

                if (isHighlighted) {
                    p.setPen(QPen(QColor(22, 163, 74, 85), 9, Qt::SolidLine, Qt::RoundCap));
                    p.drawLine(posU, posV);
                    p.setPen(QPen(QColor("#16A34A"), 3, Qt::SolidLine, Qt::RoundCap));
                    p.drawLine(posU, posV);
                } else {
                    p.setPen(QPen(QColor("#94A3B8"), 1.8, Qt::SolidLine));
                    p.drawLine(posU, posV);
                }

                QPointF mid = (posU + posV) * 0.5f;
                p.setFont(QFont("Segoe UI", 7, isHighlighted ? QFont::Bold : QFont::Normal));
                p.setPen(isHighlighted ? QColor("#15803D") : QColor("#64748B"));
                char weightStr[16];
                snprintf(weightStr, sizeof(weightStr), "%.0fp", edge->getEffectiveTravelTime());
                p.drawText(mid + QPointF(3, -2), weightStr);
            }
            edge = edge->next;
        }
    }

    // 6. Vẽ các Đỉnh Khu vực dưới dạng Pill Badge
    for (int i = 0; i < vCount; ++i) {
        const AreaNode& node = graph->getAreaByIndex(i);
        QPointF pos = getScreenPos(node.areaCode, w, h);

        bool isIncidentNode = (strcmp(node.areaCode, activeIncidentArea) == 0);
        bool isStationNode  = (strcmp(node.areaCode, activeStationArea) == 0);

        int availUnits = 0;
        for (int j = 0; j < node.unitCount; ++j) {
            if (node.stationedUnits[j] && node.stationedUnits[j]->getStatus() == UNIT_AVAILABLE) {
                availUnits++;
            }
        }

        QColor fillColor("#FFFFFF");
        QColor borderColor("#64748B");
        QColor textColor("#0F172A");

        if (isIncidentNode) {
            fillColor = QColor("#DC2626");
            borderColor = QColor("#991B1B");
            textColor = Qt::white;
            p.setBrush(QColor(220, 38, 38, 55));
            p.setPen(Qt::NoPen);
            p.drawEllipse(pos, 25, 25);
        } else if (isStationNode) {
            fillColor = QColor("#16A34A");
            borderColor = QColor("#14532D");
            textColor = Qt::white;
        } else if (availUnits > 0) {
            fillColor = QColor("#0284C7");
            borderColor = QColor("#0369A1");
            textColor = Qt::white;
        }

        QRectF pillRect(pos.x() - 22, pos.y() - 10, 44, 20);
        p.setBrush(fillColor);
        p.setPen(QPen(borderColor, 1.8));
        p.drawRoundedRect(pillRect, 9, 9);

        p.setPen(textColor);
        p.setFont(QFont("Segoe UI", 7, QFont::Bold));
        p.drawText(pillRect, Qt::AlignCenter, node.areaCode);
    }
}

// ============================================================================
// 2. WIDGET BIỂU ĐỒ THỐNG KÊ TỔNG HỢP
// ============================================================================
AnalyticsChartWidget::AnalyticsChartWidget(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(210);
    memset(&currentStats, 0, sizeof(currentStats));
}

void AnalyticsChartWidget::updateChartData(const SystemStatistics& stats) {
    currentStats = stats;
    update();
}

void AnalyticsChartWidget::paintEvent(QPaintEvent* event) {
    (void)event;
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    int w = width();
    int h = height();

    p.fillRect(rect(), QColor("#FFFFFF"));
    p.setPen(QPen(QColor("#CBD5E1"), 1));
    p.drawRoundedRect(1, 1, w - 2, h - 2, 8, 8);

    p.setPen(QColor("#0F172A"));
    p.setFont(QFont("Segoe UI", 10, QFont::Bold));
    p.drawText(18, 25, "BIỂU ĐỒ KẾT QUẢ ĐIỀU PHỐI, DỊCH VỤ & TÌNH TRẠNG NẠN NHÂN");

    int maxVal = (currentStats.totalCasesReported > 5) ? currentStats.totalCasesReported : 5;

    struct BarItem {
        QString label;
        int value;
        QString unit;
        QColor color;
    };

    BarItem items[5] = {
        {"Tổng số sự cố & dịch vụ tiếp nhận", currentStats.totalCasesReported, "hồ sơ", QColor("#0284C7")},
        {"Số ca đã giải cứu / phục vụ hoàn tất", currentStats.resolvedCasesCount, "ca", QColor("#4F46E5")},
        {"Số ca Dịch vụ có phí (Kéo xe/Thay lốp)", currentStats.paidServiceCasesCount, "ca", QColor("#0D9488")},
        {"Tình trạng: An toàn / Đã khắc phục", currentStats.outcomeSafeCount, "khách/nạn nhân", QColor("#16A34A")},
        {"Tình trạng: Chuyển viện / Đưa về Gara", currentStats.outcomeHospitalizedCount, "khách/nạn nhân", QColor("#D97706")}
    };

    int startY = 44;
    int rowHeight = 31;
    int labelWidth = 250;
    int barMaxWidth = w - labelWidth - 130;
    if (barMaxWidth < 80) barMaxWidth = 80;

    for (int i = 0; i < 5; ++i) {
        int y = startY + i * rowHeight;

        p.setPen(QColor("#1E293B"));
        p.setFont(QFont("Segoe UI", 9, QFont::DemiBold));
        p.drawText(18, y + 15, items[i].label);

        p.setBrush(QColor("#F1F5F9"));
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(labelWidth, y + 2, barMaxWidth, 17, 5, 5);

        int fillW = (maxVal > 0) ? (items[i].value * barMaxWidth) / maxVal : 0;
        if (items[i].value > 0 && fillW < 10) fillW = 10;

        if (fillW > 0) {
            p.setBrush(items[i].color);
            p.drawRoundedRect(labelWidth, y + 2, fillW, 17, 5, 5);
        }

        p.setPen(QColor("#0F172A"));
        p.setFont(QFont("Segoe UI", 9, QFont::Bold));
        p.drawText(labelWidth + barMaxWidth + 10, y + 16,
                   QString("%1 %2").arg(items[i].value).arg(items[i].unit));
    }
}

// ============================================================================
// 3. KHỞI TẠO CỬA SỔ CHÍNH & 5 TAB NGHIỆP VỤ
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), simulatedOffsetMinutes(0) {
    strncpy(currentAdminID, "ADMIN-01", MAX_ID_LEN);
    strncpy(currentAdminName, "Nguyen Hai Nam", MAX_NAME_LEN);
    strncpy(currentAdminShift, "CA SANG", MAX_ID_LEN);

    manager.initializeSystem("DaNangMap.txt", "RescueUnits.txt", "CasesHistory.txt");

    setupEnterpriseLightUI();
    applyLightDesignSystemStylesheet();

    realTimeClockTimer = new QTimer(this);
    connect(realTimeClockTimer, &QTimer::timeout, this, &MainWindow::onRealTimeTick);
    realTimeClockTimer->start(1000);

    syncRealTimeClockToManager();
    refreshAllViews();
    if (tblRescueUnits->rowCount() > 0) {
        tblRescueUnits->selectRow(0);
    }
    appendLogMessage("Hệ thống điều phối tự động TP. Đà Nẵng (16 đội cứu hộ & Kho thiết bị dùng chung) đã sẵn sàng.", "#16A34A");
}

MainWindow::~MainWindow() {}

void MainWindow::syncRealTimeClockToManager() {
    QDateTime now = QDateTime::currentDateTime().addSecs(simulatedOffsetMinutes * 60);
    if (lblSystemClock) {
        lblSystemClock->setText(QString("🕒 %1").arg(now.toString("dd/MM/yyyy HH:mm:ss")));
    }
}

void MainWindow::onRealTimeTick() {
    syncRealTimeClockToManager();
}

static QFrame* createLightKpiCard(const QString& title, QLabel*& outValLbl, const QString& accentHex, const QString& subHint) {
    QFrame* card = new QFrame();
    card->setObjectName("LightKpiCard");
    card->setStyleSheet(QString(
                            "QFrame#LightKpiCard { background-color: #FFFFFF; border: 1px solid #CBD5E1; "
                            "border-top: 4px solid %1; border-radius: 8px; }"
                            ).arg(accentHex));

    QVBoxLayout* lay = new QVBoxLayout(card);
    lay->setContentsMargins(12, 6, 12, 6);
    lay->setSpacing(2);

    QLabel* lblTitle = new QLabel(title);
    lblTitle->setStyleSheet("color: #475569; font-size: 11px; font-weight: 700; background: transparent;");

    QHBoxLayout* valRow = new QHBoxLayout();
    outValLbl = new QLabel("0");
    outValLbl->setStyleSheet(QString("color: %1; font-size: 19px; font-weight: 800; background: transparent;").arg(accentHex));

    QLabel* lblSub = new QLabel(subHint);
    lblSub->setStyleSheet("color: #64748B; font-size: 11px; background: transparent;");
    valRow->addWidget(outValLbl);
    valRow->addStretch();
    valRow->addWidget(lblSub);

    lay->addWidget(lblTitle);
    lay->addLayout(valRow);
    return card;
}

void MainWindow::setupEnterpriseLightUI() {
    setWindowTitle("Hệ Thống Điều Hành Cứu Hộ & Dịch Vụ Giao Thông TP. Đà Nẵng");
    resize(1260, 740);
    setMinimumSize(1020, 640);

    QWidget* rootWidget = new QWidget(this);
    setCentralWidget(rootWidget);
    QHBoxLayout* masterLayout = new QHBoxLayout(rootWidget);
    masterLayout->setContentsMargins(0, 0, 0, 0);
    masterLayout->setSpacing(0);

    // ------------------------------------------------------------------------
    // A. THANH SIDEBAR BÊN TRÁI (5 TAB NGHIỆP VỤ + THẺ ADMIN ĐỔI CA)
    // ------------------------------------------------------------------------
    QFrame* sidebar = new QFrame();
    sidebar->setObjectName("SidebarFrame");
    sidebar->setFixedWidth(230);
    QVBoxLayout* sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(12, 16, 12, 14);
    sideLayout->setSpacing(6);

    QHBoxLayout* brandBox = new QHBoxLayout();
    QPixmap logoPix(36, 36);
    logoPix.fill(Qt::transparent);
    {
        QPainter lp(&logoPix);
        lp.setRenderHint(QPainter::Antialiasing);
        lp.setBrush(QColor("#EF4444"));
        lp.setPen(Qt::NoPen);
        lp.drawRoundedRect(0, 0, 36, 36, 8, 8);
        lp.setBrush(Qt::white);
        lp.drawRect(14, 7, 8, 22);
        lp.drawRect(7, 14, 22, 8);
    }
    QLabel* lblLogo = new QLabel();
    lblLogo->setPixmap(logoPix);
    lblLogo->setStyleSheet("background: transparent;");

    QVBoxLayout* brandText = new QVBoxLayout();
    brandText->setSpacing(1);
    QLabel* lblAppName = new QLabel("DANANG SOS");
    lblAppName->setStyleSheet("color: #FFFFFF; font-size: 15px; font-weight: 800; background: transparent;");
    QLabel* lblAppTag = new QLabel("Auto-Dispatch & Service Center");
    lblAppTag->setStyleSheet("color: #94A3B8; font-size: 10px; background: transparent;");
    brandText->addWidget(lblAppName);
    brandText->addWidget(lblAppTag);

    brandBox->addWidget(lblLogo);
    brandBox->addSpacing(8);
    brandBox->addLayout(brandText);
    brandBox->addStretch();
    sideLayout->addLayout(brandBox);
    sideLayout->addSpacing(12);

    QLabel* lblMenuSection = new QLabel("DANH MỤC NGHIỆP VỤ");
    lblMenuSection->setStyleSheet("color: #94A3B8; font-size: 10px; font-weight: 800; padding-left: 6px; background: transparent;");
    sideLayout->addWidget(lblMenuSection);

    navBtnGroup = new QButtonGroup(this);
    navBtnGroup->setExclusive(true);

    auto createNavBtn = [&](const QString& text, int idx) -> QPushButton* {
        QPushButton* btn = new QPushButton(text);
        btn->setObjectName("NavButton");
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setMinimumHeight(40);
        navBtnGroup->addButton(btn, idx);
        sideLayout->addWidget(btn);
        return btn;
    };

    btnNavDashboard   = createNavBtn("🗺  1. Bản Đồ && Giám Sát", 0);
    btnNavIntake      = createNavBtn("📞  2. Tiếp Nhận && Điều Phối", 1);
    btnNavTeamManager = createNavBtn("🚑  3. Quản Lý Đội Cứu Hộ", 2);
    btnNavBilling     = createNavBtn("💳  4. Dịch Vụ && Hóa Đơn", 3);
    btnNavAnalytics   = createNavBtn("📊  5. Báo Cáo && Thống Kê", 4);
    btnNavDashboard->setChecked(true);

    connect(navBtnGroup, &QButtonGroup::idClicked, this, &MainWindow::onNavigationPageChanged);

    sideLayout->addStretch();

    // Thẻ Tổng đài viên trực ban & Nút Đổi Ca
    QFrame* adminCard = new QFrame();
    adminCard->setStyleSheet("background-color: #1E293B; border-radius: 8px; padding: 6px;");
    QVBoxLayout* adminLay = new QVBoxLayout(adminCard);
    adminLay->setContentsMargins(8, 6, 8, 6);
    adminLay->setSpacing(6);

    lblAdminBadge = new QLabel();
    lblAdminBadge->setWordWrap(true);
    lblAdminBadge->setStyleSheet("color: #F8FAFC; font-size: 11px; background: transparent;");
    lblAdminBadge->setText(QString(
                               "<span style='color:#38BDF8; font-weight:bold;'>👤 TRỰC BAN: %1</span><br>"
                               "<b>%2</b><br><span style='color:#94A3B8;'>Ca trực: %3</span>"
                               ).arg(currentAdminID, currentAdminName, currentAdminShift));

    QPushButton* btnLogout = new QPushButton("🔄 Đổi Ca / Đăng Xuất");
    btnLogout->setCursor(Qt::PointingHandCursor);
    btnLogout->setStyleSheet(
        "QPushButton { background-color: #334155; color: #F8FAFC; border-radius: 5px; padding: 5px; font-size: 11px; font-weight: bold; }"
        "QPushButton:hover { background-color: #DC2626; color: #FFFFFF; }"
        );
    connect(btnLogout, &QPushButton::clicked, this, &MainWindow::onSwitchShiftLogoutClicked);

    adminLay->addWidget(lblAdminBadge);
    adminLay->addWidget(btnLogout);
    sideLayout->addWidget(adminCard);

    masterLayout->addWidget(sidebar);

    // ------------------------------------------------------------------------
    // B. KHU VỰC NỘI DUNG BÊN PHẢI
    // ------------------------------------------------------------------------
    QWidget* rightContainer = new QWidget();
    rightContainer->setObjectName("RightWorkspace");
    QVBoxLayout* rightLayout = new QVBoxLayout(rightContainer);
    rightLayout->setContentsMargins(14, 10, 14, 10);
    rightLayout->setSpacing(10);

    // 1. Thanh Header Sáng
    QFrame* topBar = new QFrame();
    topBar->setObjectName("TopHeaderCard");
    QHBoxLayout* topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(14, 8, 14, 8);
    topBarLayout->setSpacing(10);

    QVBoxLayout* headerTitleBox = new QVBoxLayout();
    headerTitleBox->setSpacing(2);
    QLabel* lblHeaderMain = new QLabel("TRUNG TÂM ĐIỀU HÀNH CỨU HỘ & DỊCH VỤ GIAO THÔNG TP. ĐÀ NẴNG");
    lblHeaderMain->setStyleSheet("color: #0F172A; font-size: 14px; font-weight: 800; background: transparent;");
    QLabel* lblHeaderSub = new QLabel("Cơ chế Tự động Điều phối Tức thì (Zero-Wait Auto-Dispatch) • Kho Thiết bị Dùng chung 24/7");
    lblHeaderSub->setStyleSheet("color: #475569; font-size: 11px; background: transparent;");
    headerTitleBox->addWidget(lblHeaderMain);
    headerTitleBox->addWidget(lblHeaderSub);

    lblSystemClock = new QLabel();
    lblSystemClock->setStyleSheet(
        "background-color: #F1F5F9; color: #0F172A; font-family: 'Consolas'; "
        "font-size: 12px; font-weight: bold; padding: 5px 10px; border-radius: 6px; border: 1px solid #CBD5E1;"
        );

    QPushButton* btnAging = new QPushButton("⏱ +15 Phút Chờ");
    btnAging->setObjectName("BtnAmber");
    btnAging->setCursor(Qt::PointingHandCursor);
    connect(btnAging, &QPushButton::clicked, this, &MainWindow::onAdvanceAgingTimeClicked);

    QPushButton* btnNewCallTop = new QPushButton("➕ Tiếp Nhận Cuộc Gọi / Dịch Vụ Mới");
    btnNewCallTop->setObjectName("BtnPrimary");
    btnNewCallTop->setCursor(Qt::PointingHandCursor);
    connect(btnNewCallTop, &QPushButton::clicked, [this]() {
        btnNavIntake->setChecked(true);
        onNavigationPageChanged(1);
    });

    topBarLayout->addLayout(headerTitleBox, 1);
    topBarLayout->addWidget(lblSystemClock);
    topBarLayout->addWidget(btnAging);
    topBarLayout->addWidget(btnNewCallTop);
    rightLayout->addWidget(topBar);

    // 2. Dải 4 Thẻ KPI Sáng
    QHBoxLayout* kpiRow = new QHBoxLayout();
    kpiRow->setSpacing(10);
    kpiRow->addWidget(createLightKpiCard("CA ĐANG TRIỂN KHAI", lblKpiActive, "#0284C7", "Trên tổng 16 đội"));
    kpiRow->addWidget(createLightKpiCard("CA CHỜ ĐỘI RẢNH", lblKpiPending, "#D97706", "Tự động gán khi rảnh"));
    kpiRow->addWidget(createLightKpiCard("ĐÃ CỨU / PHỤC VỤ", lblKpiRescuedTotal, "#16A34A", "Người, thú & xe"));
    kpiRow->addWidget(createLightKpiCard("DOANH THU DỊCH VỤ", lblKpiRevenue, "#4F46E5", "Kéo xe, thay lốp..."));
    rightLayout->addLayout(kpiRow);

    // 3. Stacked Widget chứa 5 Trang
    mainStackedWidget = new QStackedWidget();
    mainStackedWidget->addWidget(createPage1Dashboard());
    mainStackedWidget->addWidget(createPage2SosIntake());
    mainStackedWidget->addWidget(createPage3TeamManagement());
    mainStackedWidget->addWidget(createPage4ServiceBilling());
    mainStackedWidget->addWidget(createPage5AnalyticsReport());

    rightLayout->addWidget(mainStackedWidget, 1);
    masterLayout->addWidget(rightContainer, 1);
}

// ============================================================================
// XÂY DỰNG 5 TRANG NGHIỆP VỤ
// ============================================================================

// --- TAB 1: BẢN ĐỒ ĐỊA LÝ ĐÀ NẴNG & GIÁM SÁT CA ĐANG CHẠY TỰ ĐỘNG ---
QWidget* MainWindow::createPage1Dashboard() {
    QWidget* page = new QWidget();
    QVBoxLayout* pageLay = new QVBoxLayout(page);
    pageLay->setContentsMargins(0, 0, 0, 0);
    pageLay->setSpacing(10);

    QHBoxLayout* splitRow = new QHBoxLayout();
    splitRow->setSpacing(12);

    // Trái: Bản đồ Đà Nẵng có lưới tọa độ km & thước tỷ lệ
    QGroupBox* grpMap = new QGroupBox("🗺 Bản Đồ Lưới Tọa Độ Địa Lý TP. Đà Nẵng (10x10 km) && Định Tuyến Tự Động");
    QVBoxLayout* mapLay = new QVBoxLayout(grpMap);
    mapLay->setContentsMargins(8, 12, 8, 8);
    mapCanvas = new DaNangMapCanvas(&manager.getCityGraph());
    mapLay->addWidget(mapCanvas);
    splitRow->addWidget(grpMap, 5);

    // Phải: Bảng các ca đang thực thi tại hiện trường & Hàng đợi dự phòng
    QVBoxLayout* rightOpsCol = new QVBoxLayout();
    rightOpsCol->setSpacing(8);

    QGroupBox* grpLive = new QGroupBox("🚑 Các Ca Đang Triển Khai Cứu Hộ Tại Hiện Trường (Đã Tự Động Điều Phối)");
    QVBoxLayout* liveLay = new QVBoxLayout(grpLive);
    liveLay->setContentsMargins(8, 10, 8, 8);

    tblLiveOperations = new QTableWidget(0, 6);
    tblLiveOperations->setHorizontalHeaderLabels({
        "Mã Sự Cố", "Khu Vực", "Loại Sự Cố / Dịch Vụ", "Đội Đang Xử Lý", "Thiết Bị Mang Theo", "Chi Phí"
    });
    tblLiveOperations->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    tblLiveOperations->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    tblLiveOperations->setColumnWidth(0, 78);
    tblLiveOperations->setColumnWidth(1, 65);
    tblLiveOperations->setColumnWidth(2, 130);
    tblLiveOperations->setColumnWidth(3, 95);
    tblLiveOperations->setColumnWidth(5, 110);
    tblLiveOperations->setSelectionBehavior(QAbstractItemView::SelectRows);
    tblLiveOperations->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tblLiveOperations->setAlternatingRowColors(true);

    QPushButton* btnCompleteNow = new QPushButton("✅ Xác Nhận Hoàn Thành Ca Đang Chọn (Trả Đội Về Trạm && Tự Động Nhận Ca Chờ)");
    btnCompleteNow->setObjectName("BtnEmerald");
    btnCompleteNow->setCursor(Qt::PointingHandCursor);
    connect(btnCompleteNow, &QPushButton::clicked, this, &MainWindow::onCompleteSelectedCaseClicked);

    liveLay->addWidget(tblLiveOperations, 1);
    liveLay->addWidget(btnCompleteNow);
    rightOpsCol->addWidget(grpLive, 3);

    // Bảng nhỏ phía dưới: Hàng đợi chờ khi toàn bộ đội cùng chuyên môn đang bận
    QGroupBox* grpQueue = new QGroupBox("⏳ Hàng Đợi Ưu Tiên Dự Phòng (Chỉ lưu khi tất cả đội cùng chuyên môn đều bận)");
    QVBoxLayout* qLay = new QVBoxLayout(grpQueue);
    qLay->setContentsMargins(8, 10, 8, 8);

    tblPendingQueue = new QTableWidget(0, 5);
    tblPendingQueue->setHorizontalHeaderLabels({
        "Mã Sự Cố", "Khu Vực", "Loại Sự Cố", "Thông Tin Khách / Nạn Nhân", "Điểm Ưu Tiên"
    });
    tblPendingQueue->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    tblPendingQueue->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    tblPendingQueue->setColumnWidth(0, 80);
    tblPendingQueue->setColumnWidth(1, 68);
    tblPendingQueue->setColumnWidth(2, 130);
    tblPendingQueue->setColumnWidth(4, 95);
    tblPendingQueue->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tblPendingQueue->setMaximumHeight(115);
    qLay->addWidget(tblPendingQueue);
    rightOpsCol->addWidget(grpQueue, 2);

    splitRow->addLayout(rightOpsCol, 6);
    pageLay->addLayout(splitRow, 1);

    QGroupBox* grpLog = new QGroupBox("🖥 Nhật Ký Điều Phối Tự Động && Lộ Trình Chi Tiết");
    QVBoxLayout* logLay = new QVBoxLayout(grpLog);
    logLay->setContentsMargins(10, 8, 10, 8);
    txtCommandLog = new QTextEdit();
    txtCommandLog->setReadOnly(true);
    txtCommandLog->setFixedHeight(78);
    logLay->addWidget(txtCommandLog);
    pageLay->addWidget(grpLog);

    return page;
}

// --- TAB 2: TIẾP NHẬN CUỘC GỌI & DỊCH VỤ (CÓ ID SỰ CỐ, ĐỂ TRỐNG TT CÁ NHÂN, KHO THIẾT BỊ CHUNG) ---
QWidget* MainWindow::createPage2SosIntake() {
    QWidget* page = new QWidget();
    QVBoxLayout* mainLay = new QVBoxLayout(page);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(10);

    // Thanh chọn nhanh kịch bản + Hiển thị Mã ID sự cố tự động
    QGroupBox* grpPreset = new QGroupBox("⚡ Mã Hồ Sơ Tự Động && Kịch Bản Thực Tế Mẫu");
    QHBoxLayout* presetLay = new QHBoxLayout(grpPreset);
    presetLay->setContentsMargins(12, 8, 12, 8);

    presetLay->addWidget(new QLabel("Mã ID Sự cố:"));
    txtAutoCaseID = new QLineEdit("SOS-0004");
    txtAutoCaseID->setReadOnly(true);
    txtAutoCaseID->setFixedWidth(105);
    txtAutoCaseID->setStyleSheet("background-color: #DBEAFE; color: #1E40AF; font-weight: 800; border: 1px solid #3B82F6;");
    presetLay->addWidget(txtAutoCaseID);

    presetLay->addSpacing(12);
    presetLay->addWidget(new QLabel("Chọn tình huống mẫu:"));
    QComboBox* cboPreset = new QComboBox();
    cboPreset->addItem("-- Tự nhập thông tin hoặc chọn nhanh kịch bản mẫu bên dưới --");
    cboPreset->addItem("Mẫu 1 [Dịch vụ Có phí]: Cứu hộ kéo xe ô tô chết máy tại Cầu Rồng (500.000đ + Phụ phí)");
    cboPreset->addItem("Mẫu 2 [Dịch vụ Có phí]: Thay lốp lưu động & Kích bình ắc-quy tại Ngũ Hành Sơn");
    cboPreset->addItem("Mẫu 3 [Khẩn cấp Miễn phí]: Người dân báo cháy/mắc kẹt ẩn danh (Để trống thông tin cá nhân)");
    cboPreset->addItem("Mẫu 4 [Khẩn cấp Miễn phí]: Giải cứu động vật hoang dã tại Bán đảo Sơn Trà");
    connect(cboPreset, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onIncidentPresetChanged);
    presetLay->addWidget(cboPreset, 1);
    mainLay->addWidget(grpPreset);

    QHBoxLayout* formColumns = new QHBoxLayout();
    formColumns->setSpacing(12);

    // Khối Trái: Vị trí & Phân loại Sự cố / Dịch vụ
    QGroupBox* grpScene = new QGroupBox("📍 1. Vị Trí Hiện Trường && Loại Yêu Cầu (Cứu Hộ / Dịch Vụ Có Phí)");
    QGridLayout* sceneGrid = new QGridLayout(grpScene);
    sceneGrid->setContentsMargins(12, 12, 12, 10);
    sceneGrid->setVerticalSpacing(9);
    sceneGrid->setHorizontalSpacing(10);

    cboAreaCode = new QComboBox();
    const SpatialGraph& g = manager.getCityGraph();
    for (int i = 0; i < g.getVertexCount(); ++i) {
        const AreaNode& node = g.getAreaByIndex(i);
        cboAreaCode->addItem(QString("%1 - %2").arg(node.areaCode, node.areaName), QString(node.areaCode));
    }

    txtDetailedAddr = new QLineEdit("Số 45 Nguyễn Văn Linh, Quận Hải Châu");
    txtSceneDesc    = new QLineEdit("Yêu cầu điều đội hỗ trợ ngay");

    cboIncidentType = new QComboBox();
    cboIncidentType->addItem("🚨 [Miễn phí] Cấp cứu Y tế / Đột quỵ / Tai nạn", INCIDENT_MEDICAL_EMERGENCY);
    cboIncidentType->addItem("🚨 [Miễn phí] Mắc kẹt song cửa / Lan can / Thang máy", INCIDENT_TRAPPED_FENCE);
    cboIncidentType->addItem("🚨 [Miễn phí] Sập hầm / Sập giàn giáo công trình", INCIDENT_COLLAPSED_TUNNEL);
    cboIncidentType->addItem("🚨 [Miễn phí] Rớt xuống giếng / Hố sâu", INCIDENT_FALLEN_INTO_WELL);
    cboIncidentType->addItem("🚨 [Miễn phí] Hỏa hoạn / Cháy nổ", INCIDENT_FIRE_EXPLOSION);
    cboIncidentType->addItem("🚨 [Miễn phí] Giải cứu / Khống chế Động vật", INCIDENT_ANIMAL_RESCUE);
    cboIncidentType->addItem("💳 [DỊCH VỤ CÓ PHÍ] Kéo xe / Cẩu xe tai nạn (500.000đ)", INCIDENT_SERVICE_TOWING);
    cboIncidentType->addItem("💳 [DỊCH VỤ CÓ PHÍ] Thay lốp lưu động / Kích bình (200.000đ)", INCIDENT_SERVICE_TIRE_BATT);
    cboIncidentType->addItem("💳 [DỊCH VỤ CÓ PHÍ] Mở khóa xe / Cứu hộ ngập nước (300.000đ)", INCIDENT_SERVICE_UNLOCK);
    connect(cboIncidentType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onIncidentOrEquipChanged);

    cboUrgency = new QComboBox();
    cboUrgency->addItem("Mức 1 - Hỗ trợ thông thường / Dịch vụ kỹ thuật", URGENCY_STANDARD);
    cboUrgency->addItem("Mức 2 - Khẩn cấp", URGENCY_URGENT);
    cboUrgency->addItem("Mức 3 - Nguy kịch tính mạng", URGENCY_CRITICAL);

    lblEstimatedFeePreview = new QLabel("0 VNĐ (Dịch vụ công cộng miễn phí)");
    lblEstimatedFeePreview->setStyleSheet("color: #16A34A; font-weight: 800; font-size: 13px;");

    sceneGrid->addWidget(new QLabel("Khu vực Đà Nẵng:"), 0, 0);
    sceneGrid->addWidget(cboAreaCode, 0, 1);
    sceneGrid->addWidget(new QLabel("Địa chỉ cụ thể:"), 1, 0);
    sceneGrid->addWidget(txtDetailedAddr, 1, 1);
    sceneGrid->addWidget(new QLabel("Loại sự cố / Dịch vụ:"), 2, 0);
    sceneGrid->addWidget(cboIncidentType, 2, 1);
    sceneGrid->addWidget(new QLabel("Mức độ khẩn cấp:"), 3, 0);
    sceneGrid->addWidget(cboUrgency, 3, 1);
    sceneGrid->addWidget(new QLabel("Mô tả hiện trường:"), 4, 0);
    sceneGrid->addWidget(txtSceneDesc, 4, 1);
    sceneGrid->addWidget(new QLabel("Chi phí dự kiến:"), 5, 0);
    sceneGrid->addWidget(lblEstimatedFeePreview, 5, 1);

    // Khối Phải: Thông tin Cá nhân (Cho phép để trống hoàn toàn)
    QGroupBox* grpVictim = new QGroupBox("👤 2. Thông Tin Người Gọi / Nạn Nhân (Có Thể Để Trống Khi Khẩn Cấp)");
    QGridLayout* vicGrid = new QGridLayout(grpVictim);
    vicGrid->setContentsMargins(12, 12, 12, 10);
    vicGrid->setVerticalSpacing(9);
    vicGrid->setHorizontalSpacing(10);

    cboVictimType = new QComboBox();
    cboVictimType->addItem("Con người / Chủ phương tiện");
    cboVictimType->addItem("Động vật gặp nạn");

    txtVictimNameOrSpecies = new QLineEdit();
    txtVictimNameOrSpecies->setPlaceholderText("Có thể để trống (Tự điền: Khách vãng lai / Chưa rõ tên)");

    txtVictimPhone = new QLineEdit();
    txtVictimPhone->setPlaceholderText("Có thể để trống (Tự điền: Chưa cung cấp)");

    txtMedHistOrFeatures = new QLineEdit();
    txtMedHistOrFeatures->setPlaceholderText("Có thể để trống (Tiền sử bệnh / Biển số xe / Đặc điểm)");

    cboConsciousness = new QComboBox();
    cboConsciousness->addItem("Tỉnh táo hoàn toàn / Bình thường", CONSCIOUS_ALERT);
    cboConsciousness->addItem("Hoảng loạn / Kích động", CONSCIOUS_PANICKED);
    cboConsciousness->addItem("Lơ mơ / Mất phương hướng", CONSCIOUS_CONFUSED);
    cboConsciousness->addItem("Bất tỉnh / Hôn mê", CONSCIOUS_UNCONSCIOUS);

    chkBreathing = new QCheckBox("Còn hô hấp");
    chkBreathing->setChecked(true);
    chkTrapped   = new QCheckBox("Đang bị mắc kẹt");

    spnVictimCount = new QSpinBox();
    spnVictimCount->setRange(1, 100);
    spnVictimCount->setValue(1);

    vicGrid->addWidget(new QLabel("Phân loại đối tượng:"), 0, 0);
    vicGrid->addWidget(cboVictimType, 0, 1);
    vicGrid->addWidget(new QLabel("Họ tên / Loài (Tùy chọn):"), 1, 0);
    vicGrid->addWidget(txtVictimNameOrSpecies, 1, 1);
    vicGrid->addWidget(new QLabel("Số điện thoại (Tùy chọn):"), 2, 0);
    vicGrid->addWidget(txtVictimPhone, 2, 1);
    vicGrid->addWidget(new QLabel("Ghi chú / Biển số xe:"), 3, 0);
    vicGrid->addWidget(txtMedHistOrFeatures, 3, 1);
    vicGrid->addWidget(new QLabel("Trạng thái hiện tại:"), 4, 0);
    vicGrid->addWidget(cboConsciousness, 4, 1);

    QHBoxLayout* flagsRow = new QHBoxLayout();
    flagsRow->addWidget(chkBreathing);
    flagsRow->addWidget(chkTrapped);
    flagsRow->addStretch();
    flagsRow->addWidget(new QLabel("Số lượng:"));
    flagsRow->addWidget(spnVictimCount);
    vicGrid->addLayout(flagsRow, 5, 0, 1, 2);

    formColumns->addWidget(grpScene, 1);
    formColumns->addWidget(grpVictim, 1);
    mainLay->addLayout(formColumns);

    // Khối Dưới: Kho thiết bị dùng chung luôn có sẵn
    QGroupBox* grpEquip = new QGroupBox("🛠 3. Kho Thiết Bị Dùng Chung Luôn Có Sẵn (Tích Chọn Để Đội Mang Đi Kèm)");
    QGridLayout* eqGrid = new QGridLayout(grpEquip);
    eqGrid->setContentsMargins(12, 10, 12, 8);
    eqGrid->setVerticalSpacing(8);
    eqGrid->setHorizontalSpacing(10);

    chkEquipFirstAid       = new QCheckBox("Bộ sơ cứu y tế");
    chkEquipAED            = new QCheckBox("Máy sốc tim (AED)");
    chkEquipOxygen         = new QCheckBox("Bình dưỡng khí Oxy");
    chkEquipCutter         = new QCheckBox("Kìm thủy lực (+100k nếu DV)");
    chkEquipTripod         = new QCheckBox("Giá 3 chân && Tời cáp");
    chkEquipShoring        = new QCheckBox("Cột chống sập hầm");
    chkEquipFirePump       = new QCheckBox("Vòi rồng && Bơm PCCC");
    chkEquipTowCrane       = new QCheckBox("Cáp cẩu kéo xe (+150k)");
    chkEquipAnimalCage     = new QCheckBox("Lồng sắt && Lưới thú");
    chkEquipTranquilizer   = new QCheckBox("Súng bắn thuốc mê");
    chkEquipSpareTire      = new QCheckBox("Bộ thay lốp lưu động (+80k)");
    chkEquipBatteryBooster = new QCheckBox("Máy kích bình ắc-quy (+70k)");

    QCheckBox* allChks[] = {
        chkEquipFirstAid, chkEquipAED, chkEquipOxygen, chkEquipCutter,
        chkEquipTripod, chkEquipShoring, chkEquipFirePump, chkEquipTowCrane,
        chkEquipAnimalCage, chkEquipTranquilizer, chkEquipSpareTire, chkEquipBatteryBooster
    };
    for (int i = 0; i < 12; ++i) {
        connect(allChks[i], &QCheckBox::toggled, this, &MainWindow::onIncidentOrEquipChanged);
        eqGrid->addWidget(allChks[i], i / 4, i % 4);
    }
    mainLay->addWidget(grpEquip);

    QPushButton* btnSubmitSos = new QPushButton("⚡ TIẾP NHẬN HỒ SƠ && TỰ ĐỘNG ĐIỀU PHỐI ĐỘI GẦN NHẤT NGAY LẬP TỨC");
    btnSubmitSos->setObjectName("BtnPrimaryLarge");
    btnSubmitSos->setMinimumHeight(44);
    btnSubmitSos->setCursor(Qt::PointingHandCursor);
    connect(btnSubmitSos, &QPushButton::clicked, this, &MainWindow::onReportAndAutoDispatchClicked);
    mainLay->addWidget(btnSubmitSos);
    mainLay->addStretch();

    return page;
}

// --- TAB 3: QUẢN LÝ ĐỘI CỨU HỘ (ĐỘI TRƯỞNG, THÀNH VIÊN & THÊM ĐỘI MỚI) ---
QWidget* MainWindow::createPage3TeamManagement() {
    QWidget* page = new QWidget();
    QHBoxLayout* pageLay = new QHBoxLayout(page);
    pageLay->setContentsMargins(0, 0, 0, 0);
    pageLay->setSpacing(12);

    // Cột Trái (60%): Danh sách 16 Đội cứu hộ + Form thêm đội mới
    QVBoxLayout* leftCol = new QVBoxLayout();
    leftCol->setSpacing(10);

    QGroupBox* grpUnits = new QGroupBox("🚑 Danh Sách Các Đội Cứu Hộ Thường Trực (Nhấp chọn 1 đội để xem Đội trưởng && Thành viên)");
    QVBoxLayout* uLay = new QVBoxLayout(grpUnits);
    tblRescueUnits = new QTableWidget(0, 6);
    tblRescueUnits->setHorizontalHeaderLabels({
        "Mã Đội", "Tên Phiên Hiệu Đội", "Chuyên Môn", "Vị Trí", "Đội Trưởng", "Trạng Thái"
    });
    tblRescueUnits->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    tblRescueUnits->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    tblRescueUnits->setColumnWidth(0, 88);
    tblRescueUnits->setColumnWidth(2, 145);
    tblRescueUnits->setColumnWidth(3, 65);
    tblRescueUnits->setColumnWidth(4, 135);
    tblRescueUnits->setColumnWidth(5, 115);
    tblRescueUnits->setSelectionBehavior(QAbstractItemView::SelectRows);
    tblRescueUnits->setSelectionMode(QAbstractItemView::SingleSelection);
    tblRescueUnits->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tblRescueUnits->setAlternatingRowColors(true);
    connect(tblRescueUnits, &QTableWidget::itemSelectionChanged, this, &MainWindow::onRescueUnitSelectionChanged);
    uLay->addWidget(tblRescueUnits);
    leftCol->addWidget(grpUnits, 1);

    // Khung thêm đội cứu hộ mới
    QGroupBox* grpAddUnit = new QGroupBox("➕ Thành Lập Thêm Đội Cứu Hộ Mới Vào Mạng Lưới Đà Nẵng");
    QGridLayout* addGrid = new QGridLayout(grpAddUnit);
    addGrid->setContentsMargins(10, 10, 10, 8);
    addGrid->setSpacing(8);

    cboNewUnitSpec = new QComboBox();
    cboNewUnitSpec->addItem("Y Tế Cấp Cứu 115", SPEC_MEDICAL);
    cboNewUnitSpec->addItem("PCCC & CNCH 114", SPEC_FIRE);
    cboNewUnitSpec->addItem("Cứu Hộ Giao Thông & Cơ Giới", SPEC_MECHANIC);
    cboNewUnitSpec->addItem("Cứu Hộ Động Vật", SPEC_ANIMAL);

    txtNewUnitID      = new QLineEdit("MECH-ST01");
    txtNewUnitName    = new QLineEdit("Đội Cứu Hộ Giao Thông Cầu Rồng");
    cboNewUnitArea    = new QComboBox();
    const SpatialGraph& g = manager.getCityGraph();
    for (int i = 0; i < g.getVertexCount(); ++i) {
        cboNewUnitArea->addItem(g.getAreaByIndex(i).areaCode);
    }
    txtNewUnitVehicle = new QLineEdit("Xe Cẩu Kéo Isuzu");
    txtNewUnitLeader  = new QLineEdit("KS. Lê Văn Khoa");
    txtNewUnitPhone   = new QLineEdit("0905666777");

    QPushButton* btnCreateUnit = new QPushButton("➕ Thêm Đội Cứu Hộ Mới");
    btnCreateUnit->setObjectName("BtnPrimary");
    btnCreateUnit->setCursor(Qt::PointingHandCursor);
    connect(btnCreateUnit, &QPushButton::clicked, this, &MainWindow::onCreateNewRescueUnitClicked);

    addGrid->addWidget(new QLabel("Chuyên môn:"), 0, 0);
    addGrid->addWidget(cboNewUnitSpec, 0, 1);
    addGrid->addWidget(new QLabel("Mã đội:"), 0, 2);
    addGrid->addWidget(txtNewUnitID, 0, 3);
    addGrid->addWidget(new QLabel("Tên đội:"), 1, 0);
    addGrid->addWidget(txtNewUnitName, 1, 1);
    addGrid->addWidget(new QLabel("Trạm trực:"), 1, 2);
    addGrid->addWidget(cboNewUnitArea, 1, 3);
    addGrid->addWidget(new QLabel("Phương tiện:"), 2, 0);
    addGrid->addWidget(txtNewUnitVehicle, 2, 1);
    addGrid->addWidget(new QLabel("Đội trưởng:"), 2, 2);
    addGrid->addWidget(txtNewUnitLeader, 2, 3);
    addGrid->addWidget(new QLabel("SĐT Đội trưởng:"), 3, 0);
    addGrid->addWidget(txtNewUnitPhone, 3, 1);
    addGrid->addWidget(btnCreateUnit, 3, 2, 1, 2);

    leftCol->addWidget(grpAddUnit);
    pageLay->addLayout(leftCol, 6);

    // Cột Phải (40%): Hồ sơ chi tiết Đội trưởng & Danh sách Thành viên của Đội đang chọn
    QGroupBox* grpDetail = new QGroupBox("👨‍✈️ Hồ Sơ Đội Trưởng && Danh Sách Thành Viên Trong Đội");
    QVBoxLayout* detLay = new QVBoxLayout(grpDetail);
    detLay->setContentsMargins(14, 14, 14, 12);
    detLay->setSpacing(10);

    lblSelectedTeamHeader = new QLabel("Chọn một đội cứu hộ ở bảng bên trái");
    lblSelectedTeamHeader->setWordWrap(true);
    lblSelectedTeamHeader->setStyleSheet(
        "background-color: #F1F5F9; color: #0F172A; padding: 10px; border-radius: 8px; border: 1px solid #CBD5E1;"
        );
    detLay->addWidget(lblSelectedTeamHeader);

    QGridLayout* leaderGrid = new QGridLayout();
    txtEditLeaderName  = new QLineEdit();
    txtEditLeaderPhone = new QLineEdit();
    QPushButton* btnSaveLeader = new QPushButton("💾 Cập Nhật Thông Tin Đội Trưởng");
    btnSaveLeader->setObjectName("BtnPrimary");
    btnSaveLeader->setCursor(Qt::PointingHandCursor);
    connect(btnSaveLeader, &QPushButton::clicked, this, &MainWindow::onUpdateLeaderInfoClicked);

    leaderGrid->addWidget(new QLabel("Họ tên Đội trưởng:"), 0, 0);
    leaderGrid->addWidget(txtEditLeaderName, 0, 1);
    leaderGrid->addWidget(new QLabel("SĐT Đội trưởng:"), 1, 0);
    leaderGrid->addWidget(txtEditLeaderPhone, 1, 1);
    leaderGrid->addWidget(btnSaveLeader, 2, 0, 1, 2);
    detLay->addLayout(leaderGrid);

    detLay->addWidget(new QLabel("👥 Danh sách các thành viên thuộc đội:"));
    lstTeamMembers = new QListWidget();
    lstTeamMembers->setStyleSheet(
        "QListWidget { background-color: #F8FAFC; border: 1px solid #CBD5E1; border-radius: 6px; padding: 6px; color: #0F172A; font-weight: 600; }"
        "QListWidget::item { padding: 6px; border-bottom: 1px solid #E2E8F0; }"
        );
    detLay->addWidget(lstTeamMembers, 1);

    QHBoxLayout* addMemRow = new QHBoxLayout();
    txtNewMemberName = new QLineEdit();
    txtNewMemberName->setPlaceholderText("Nhập họ tên & chức vụ thành viên mới...");
    QPushButton* btnAddMem = new QPushButton("➕ Thêm Thành Viên");
    btnAddMem->setObjectName("BtnEmerald");
    btnAddMem->setCursor(Qt::PointingHandCursor);
    connect(btnAddMem, &QPushButton::clicked, this, &MainWindow::onAddTeamMemberClicked);
    addMemRow->addWidget(txtNewMemberName, 1);
    addMemRow->addWidget(btnAddMem);
    detLay->addLayout(addMemRow);

    pageLay->addWidget(grpDetail, 4);
    return page;
}

// --- TAB 4: DỊCH VỤ CÓ PHÍ (KÉO XE, THAY LỐP) & THANH TOÁN HÓA ĐƠN CHO ĐỘI CỨU HỘ ---
QWidget* MainWindow::createPage4ServiceBilling() {
    QWidget* page = new QWidget();
    QHBoxLayout* pageLay = new QHBoxLayout(page);
    pageLay->setContentsMargins(0, 0, 0, 0);
    pageLay->setSpacing(12);

    // Bảng Trái: Danh sách Hóa đơn (Bills)
    QGroupBox* grpBills = new QGroupBox("💳 Danh Sách Hóa Đơn Dịch Vụ Cứu Hộ Giao Thông (Kéo Xe, Thay Lốp, Kích Bình)");
    QVBoxLayout* bLay = new QVBoxLayout(grpBills);

    tblServiceInvoices = new QTableWidget(0, 7);
    tblServiceInvoices->setHorizontalHeaderLabels({
        "Mã Hóa Đơn", "Mã Sự Cố", "Loại Dịch Vụ", "Đội Thực Hiện", "Đội Trưởng", "Tổng Tiền (VNĐ)", "Trạng Thái Thanh Toán"
    });
    tblServiceInvoices->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tblServiceInvoices->setSelectionBehavior(QAbstractItemView::SelectRows);
    tblServiceInvoices->setSelectionMode(QAbstractItemView::SingleSelection);
    tblServiceInvoices->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tblServiceInvoices->setAlternatingRowColors(true);
    connect(tblServiceInvoices, &QTableWidget::itemSelectionChanged, this, &MainWindow::onInvoiceRowSelected);
    bLay->addWidget(tblServiceInvoices);
    pageLay->addWidget(grpBills, 6);

    // Khung Phải: Phiếu tính tiền chi tiết (Invoice Receipt) & Nút Thanh toán
    QGroupBox* grpReceipt = new QGroupBox("🧾 Chi Tiết Phiếu Thu && Thanh Toán Cho Đội Cứu Hộ");
    QVBoxLayout* rLay = new QVBoxLayout(grpReceipt);
    rLay->setContentsMargins(14, 14, 14, 14);
    rLay->setSpacing(10);

    lblInvoiceDetailPreview = new QLabel("Hãy nhấp chọn một hóa đơn ở bảng bên trái để xem chi tiết biểu phí.");
    lblInvoiceDetailPreview->setWordWrap(true);
    lblInvoiceDetailPreview->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    lblInvoiceDetailPreview->setStyleSheet(
        "background-color: #F8FAFC; color: #0F172A; padding: 14px; border-radius: 8px; "
        "border: 1px solid #CBD5E1; font-size: 12px; line-height: 1.5;"
        );
    rLay->addWidget(lblInvoiceDetailPreview, 1);

    QPushButton* btnPayBill = new QPushButton("✅ Xác Nhận Khách Đã Thanh Toán (Cộng Doanh Thu Cho Đội Cứu Hộ)");
    btnPayBill->setObjectName("BtnEmerald");
    btnPayBill->setMinimumHeight(40);
    btnPayBill->setCursor(Qt::PointingHandCursor);
    connect(btnPayBill, &QPushButton::clicked, this, &MainWindow::onConfirmInvoicePaymentClicked);

    QPushButton* btnPrintInvoice = new QPushButton("🖨 Xuất && Mở Ngay Phiếu Hóa Đơn Ra File (.txt)");
    btnPrintInvoice->setObjectName("BtnPrimary");
    btnPrintInvoice->setMinimumHeight(38);
    btnPrintInvoice->setCursor(Qt::PointingHandCursor);
    connect(btnPrintInvoice, &QPushButton::clicked, this, &MainWindow::onExportSelectedInvoiceClicked);

    rLay->addWidget(btnPayBill);
    rLay->addWidget(btnPrintInvoice);
    pageLay->addWidget(grpReceipt, 4);

    return page;
}

// --- TAB 5: BÁO CÁO & THỐNG KÊ TỔNG HỢP ---
QWidget* MainWindow::createPage5AnalyticsReport() {
    QWidget* page = new QWidget();
    QVBoxLayout* pageLay = new QVBoxLayout(page);
    pageLay->setContentsMargins(0, 0, 0, 0);
    pageLay->setSpacing(10);

    QHBoxLayout* topAnalyticsRow = new QHBoxLayout();
    topAnalyticsRow->setSpacing(12);

    chartWidget = new AnalyticsChartWidget();
    topAnalyticsRow->addWidget(chartWidget, 6);

    QGroupBox* grpSummary = new QGroupBox("📋 Tổng Hợp Số Liệu Cứu Hộ && Doanh Thu Dịch Vụ");
    QVBoxLayout* sumLay = new QVBoxLayout(grpSummary);
    lblReportSummaryText = new QLabel();
    lblReportSummaryText->setWordWrap(true);
    lblReportSummaryText->setStyleSheet("color: #0F172A; font-size: 12px; line-height: 1.5; background: transparent;");

    QPushButton* btnExportFile = new QPushButton("💾 Xuất && Mở Ngay File Báo Cáo Tổng Hợp (FinalReport_DaNang.txt)");
    btnExportFile->setObjectName("BtnPrimary");
    btnExportFile->setCursor(Qt::PointingHandCursor);
    connect(btnExportFile, &QPushButton::clicked, this, &MainWindow::onExportReportToFileClicked);

    sumLay->addWidget(lblReportSummaryText, 1);
    sumLay->addWidget(btnExportFile);
    topAnalyticsRow->addWidget(grpSummary, 4);

    pageLay->addLayout(topAnalyticsRow);

    QHBoxLayout* bottomTablesRow = new QHBoxLayout();
    bottomTablesRow->setSpacing(12);

    QGroupBox* grpResolved = new QGroupBox("✅ 1. Danh Sách Các Ca Đã Hoàn Thành");
    QVBoxLayout* resLay = new QVBoxLayout(grpResolved);
    tblResolvedReport = new QTableWidget(0, 6);
    tblResolvedReport->setHorizontalHeaderLabels({
        "Mã Sự Cố", "Khu Vực", "Loại Sự Cố / Dịch Vụ", "Đội Thực Hiện", "Chi Phí", "Kết Quả Sau Cứu Hộ"
    });
    tblResolvedReport->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tblResolvedReport->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tblResolvedReport->setAlternatingRowColors(true);
    resLay->addWidget(tblResolvedReport);

    QGroupBox* grpUnfinished = new QGroupBox("⏳ 2. Danh Sách Các Ca Đang Triển Khai / Đang Chờ");
    QVBoxLayout* unfLay = new QVBoxLayout(grpUnfinished);
    tblUnfinishedReport = new QTableWidget(0, 5);
    tblUnfinishedReport->setHorizontalHeaderLabels({
        "Mã Sự Cố", "Khu Vực", "Loại Sự Cố / Dịch Vụ", "Đội Phụ Trách", "Trạng Thái"
    });
    tblUnfinishedReport->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tblUnfinishedReport->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tblUnfinishedReport->setAlternatingRowColors(true);
    unfLay->addWidget(tblUnfinishedReport);

    bottomTablesRow->addWidget(grpResolved, 6);
    bottomTablesRow->addWidget(grpUnfinished, 5);

    pageLay->addLayout(bottomTablesRow, 1);
    return page;
}

// ============================================================================
// BẢNG MÀU LIGHT ENTERPRISE DESIGN SYSTEM (KHÓA MÀU CHỐNG LỖI WINDOWS DARK MODE)
// ============================================================================
void MainWindow::applyLightDesignSystemStylesheet() {
    setStyleSheet(
        "QMainWindow, QWidget#RightWorkspace { background-color: #F8FAFC; color: #0F172A; font-family: 'Segoe UI'; font-size: 12px; }"
        "QFrame#SidebarFrame { background-color: #0F172A; border-right: 1px solid #1E293B; }"
        "QPushButton#NavButton { background-color: transparent; color: #CBD5E1; text-align: left; padding: 9px 12px; "
        "border-radius: 8px; font-size: 12px; font-weight: 600; border: none; }"
        "QPushButton#NavButton:hover { background-color: #1E293B; color: #FFFFFF; }"
        "QPushButton#NavButton:checked { background-color: #0284C7; color: #FFFFFF; font-weight: 700; }"
        "QFrame#TopHeaderCard { background-color: #FFFFFF; border: 1px solid #CBD5E1; border-radius: 10px; }"
        "QGroupBox { background-color: #FFFFFF; border: 1px solid #CBD5E1; border-radius: 10px; margin-top: 8px; "
        "padding-top: 14px; font-weight: 700; color: #0F172A; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 14px; padding: 0 6px; color: #0284C7; background-color: transparent; }"

        "QLabel { color: #1E293B; font-weight: 600; background-color: transparent; }"
        "QCheckBox { color: #0F172A; font-weight: 600; spacing: 6px; background-color: transparent; }"
        "QCheckBox::indicator { width: 16px; height: 16px; border: 2px solid #64748B; border-radius: 4px; background-color: #FFFFFF; }"
        "QCheckBox::indicator:checked { background-color: #0284C7; border-color: #0284C7; }"

        "QMessageBox, QDialog { background-color: #FFFFFF; color: #0F172A; }"
        "QMessageBox QLabel { color: #0F172A; font-size: 13px; font-weight: 600; background-color: transparent; }"
        "QMessageBox QPushButton { background-color: #0284C7; color: #FFFFFF; border: 1px solid #0369A1; "
        "border-radius: 6px; padding: 6px 14px; min-width: 90px; font-weight: 700; }"
        "QMessageBox QPushButton:hover { background-color: #0369A1; }"

        "QLineEdit, QComboBox, QSpinBox { background-color: #F8FAFC; border: 1px solid #94A3B8; border-radius: 6px; "
        "padding: 5px 10px; color: #0F172A; font-weight: 600; min-height: 20px; }"
        "QLineEdit:focus, QComboBox:focus, QSpinBox:focus { border: 2px solid #0284C7; background-color: #FFFFFF; }"
        "QComboBox QAbstractItemView { background-color: #FFFFFF; color: #0F172A; border: 1px solid #0284C7; "
        "selection-background-color: #0284C7; selection-color: #FFFFFF; outline: 0px; padding: 4px; }"
        "QComboBox QAbstractItemView::item { min-height: 26px; padding: 4px 8px; color: #0F172A; background-color: #FFFFFF; }"
        "QComboBox QAbstractItemView::item:selected { background-color: #0284C7; color: #FFFFFF; }"

        "QTableWidget { background-color: #FFFFFF; alternate-background-color: #F8FAFC; gridline-color: #E2E8F0; "
        "border: 1px solid #CBD5E1; border-radius: 6px; color: #0F172A; selection-background-color: #BAE6FD; selection-color: #0F172A; }"
        "QHeaderView::section { background-color: #F1F5F9; color: #1E293B; font-weight: 700; padding: 6px; "
        "border: none; border-bottom: 2px solid #CBD5E1; border-right: 1px solid #E2E8F0; }"
        "QTextEdit { background-color: #F8FAFC; border: 1px solid #CBD5E1; border-radius: 6px; color: #0F172A; font-family: 'Consolas'; }"
        "QPushButton { border-radius: 6px; padding: 6px 12px; font-weight: 700; font-size: 12px; }"
        "QPushButton#BtnPrimary { background-color: #0284C7; color: #FFFFFF; border: 1px solid #0369A1; }"
        "QPushButton#BtnPrimary:hover { background-color: #0369A1; }"
        "QPushButton#BtnPrimaryLarge { background-color: #0284C7; color: #FFFFFF; font-size: 13px; border-radius: 8px; }"
        "QPushButton#BtnPrimaryLarge:hover { background-color: #0369A1; }"
        "QPushButton#BtnCrimson { background-color: #DC2626; color: #FFFFFF; border: 1px solid #B91C1C; }"
        "QPushButton#BtnCrimson:hover { background-color: #B91C1C; }"
        "QPushButton#BtnEmerald { background-color: #16A34A; color: #FFFFFF; border: 1px solid #15803D; }"
        "QPushButton#BtnEmerald:hover { background-color: #15803D; }"
        "QPushButton#BtnAmber { background-color: #D97706; color: #FFFFFF; border: 1px solid #B45309; }"
        "QPushButton#BtnAmber:hover { background-color: #B45309; }"
        );
}

// ============================================================================
// 4. ĐỒNG BỘ DỮ LIỆU LÊN 5 TRANG & XỬ LÝ SỰ KIỆN
// ============================================================================
static QString incidentTypeToString(IncidentType t) {
    switch (t) {
    case INCIDENT_MEDICAL_EMERGENCY: return "Cấp cứu Y tế 115";
    case INCIDENT_TRAPPED_FENCE:     return "Kẹt song cửa/Thang máy";
    case INCIDENT_COLLAPSED_TUNNEL:  return "Sập hầm/Công trình";
    case INCIDENT_FALLEN_INTO_WELL:  return "Rớt giếng sâu";
    case INCIDENT_FIRE_EXPLOSION:    return "Hỏa hoạn/Cháy nổ";
    case INCIDENT_ANIMAL_RESCUE:     return "Cứu hộ Động vật";
    case INCIDENT_SERVICE_TOWING:    return "[DV] Kéo/Cẩu xe ô tô";
    case INCIDENT_SERVICE_TIRE_BATT: return "[DV] Thay lốp/Kích bình";
    case INCIDENT_SERVICE_UNLOCK:    return "[DV] Mở khóa/Ngập nước";
    default: return "Khác";
    }
}

void MainWindow::onNavigationPageChanged(int pageIndex) {
    mainStackedWidget->setCurrentIndex(pageIndex);
    refreshAllViews();
}

void MainWindow::updateFeePreviewInForm() {
    IncidentType incType = (IncidentType)cboIncidentType->currentData().toInt();
    if (incType != INCIDENT_SERVICE_TOWING &&
        incType != INCIDENT_SERVICE_TIRE_BATT &&
        incType != INCIDENT_SERVICE_UNLOCK) {
        lblEstimatedFeePreview->setText("0 VNĐ (Cứu hộ khẩn cấp công cộng - Miễn phí 100%)");
        lblEstimatedFeePreview->setStyleSheet("color: #16A34A; font-weight: 800; font-size: 12px;");
        return;
    }

    double baseFee = (incType == INCIDENT_SERVICE_TOWING) ? 500000.0
                     : (incType == INCIDENT_SERVICE_TIRE_BATT) ? 200000.0 : 300000.0;
    double equipFee = 0.0;
    if (chkEquipTowCrane->isChecked())       equipFee += 150000.0;
    if (chkEquipSpareTire->isChecked())      equipFee += 80000.0;
    if (chkEquipBatteryBooster->isChecked()) equipFee += 70000.0;
    if (chkEquipCutter->isChecked())         equipFee += 100000.0;

    lblEstimatedFeePreview->setText(QString("Phí cơ bản: %1 + Phụ phí thiết bị: %2 (+25.000đ/phút di chuyển)")
                                        .arg(formatCurrencyVND(baseFee), formatCurrencyVND(equipFee)));
    lblEstimatedFeePreview->setStyleSheet("color: #D97706; font-weight: 800; font-size: 12px;");
}

void MainWindow::onIncidentOrEquipChanged() {
    updateFeePreviewInForm();
}

void MainWindow::refreshAllViews() {
    syncRealTimeClockToManager();

    // Cập nhật mã ID sự cố tiếp theo trên Form Tab 2
    char nextIdBuf[MAX_ID_LEN];
    manager.peekNextCaseID(nextIdBuf, sizeof(nextIdBuf));
    if (txtAutoCaseID) txtAutoCaseID->setText(nextIdBuf);

    SystemStatistics stats = manager.computeStatistics();
    lblKpiActive->setText(QString::number(stats.activeDispatchedCount));
    lblKpiPending->setText(QString::number(stats.pendingCasesCount));
    lblKpiRescuedTotal->setText(QString::number(stats.totalHumansRescued + stats.totalAnimalsRescued));
    lblKpiRevenue->setText(formatCurrencyVND(stats.totalPaidRevenueVND + stats.totalUnpaidPendingVND));

    int hCount = manager.getHistoryCaseCount();
    int activeOnlyCount = 0;
    int resolvedOnlyCount = 0;
    int paidServiceCount = 0;

    for (int i = 0; i < hCount; ++i) {
        EmergencyCase* c = manager.getHistoryCaseAt(i);
        if (!c) continue;
        if (c->getStatus() == CASE_RESOLVED) resolvedOnlyCount++;
        else activeOnlyCount++;
        if (c->isPaidServiceCase()) paidServiceCount++;
    }

    // --- TAB 1: BẢNG CÁC CA ĐANG TRIỂN KHAI TẠI HIỆN TRƯỜNG ---
    tblLiveOperations->setRowCount(activeOnlyCount);
    int actRow = 0;
    for (int i = 0; i < hCount; ++i) {
        EmergencyCase* c = manager.getHistoryCaseAt(i);
        if (!c || c->getStatus() == CASE_RESOLVED) continue;

        QString unitStr = c->getAssignedUnit() ? c->getAssignedUnit()->getUnitID() : "N/A";
        char eqBuf[128];
        c->formatEquippedItemsText(eqBuf, sizeof(eqBuf));

        tblLiveOperations->setItem(actRow, 0, new QTableWidgetItem(c->getCaseID()));
        tblLiveOperations->setItem(actRow, 1, new QTableWidgetItem(c->getLocationCode()));
        tblLiveOperations->setItem(actRow, 2, new QTableWidgetItem(incidentTypeToString(c->getIncidentType())));

        QTableWidgetItem* uItem = new QTableWidgetItem(unitStr);
        uItem->setBackground(QColor("#DBEAFE"));
        uItem->setForeground(QColor("#1E40AF"));
        uItem->setFont(QFont("Segoe UI", 9, QFont::Bold));
        tblLiveOperations->setItem(actRow, 3, uItem);

        tblLiveOperations->setItem(actRow, 4, new QTableWidgetItem(QString::fromUtf8(eqBuf)));
        tblLiveOperations->setItem(actRow, 5, new QTableWidgetItem(formatCurrencyVND(c->getTotalAmountVND())));
        actRow++;
    }

    // --- TAB 1: BẢNG HÀNG ĐỢI DỰ PHÒNG ---
    const EmergencyPriorityQueue& pq = manager.getPendingQueue();
    int qSize = pq.size();
    tblPendingQueue->setRowCount(qSize);
    CustomDateTime now = manager.getSystemClock();

    for (int r = 0; r < qSize; ++r) {
        EmergencyCase* c = pq.getCaseAt(r);
        char vicDetails[256];
        c->getVictim()->formatDetails(vicDetails, sizeof(vicDetails));
        double score = c->calculateDynamicPriority(now);

        tblPendingQueue->setItem(r, 0, new QTableWidgetItem(c->getCaseID()));
        tblPendingQueue->setItem(r, 1, new QTableWidgetItem(c->getLocationCode()));
        tblPendingQueue->setItem(r, 2, new QTableWidgetItem(incidentTypeToString(c->getIncidentType())));
        tblPendingQueue->setItem(r, 3, new QTableWidgetItem(QString::fromUtf8(vicDetails)));
        tblPendingQueue->setItem(r, 4, new QTableWidgetItem(QString("%1 điểm").arg(score, 0, 'f', 1)));
    }

    // --- TAB 3: BẢNG 16 ĐỘI CỨU HỘ ---
    int selectedUnitRow = tblRescueUnits->currentRow();
    int uCount = manager.getTotalUnitCount();
    tblRescueUnits->setRowCount(uCount);
    for (int i = 0; i < uCount; ++i) {
        RescueUnit* u = manager.getUnitAt(i);
        bool isAvail = (u->getStatus() == UNIT_AVAILABLE);

        tblRescueUnits->setItem(i, 0, new QTableWidgetItem(u->getUnitID()));
        tblRescueUnits->setItem(i, 1, new QTableWidgetItem(QString::fromUtf8(u->getUnitName())));
        tblRescueUnits->setItem(i, 2, new QTableWidgetItem(QString::fromUtf8(u->getSpecialtyName())));
        tblRescueUnits->setItem(i, 3, new QTableWidgetItem(u->getCurrentLocationCode()));
        tblRescueUnits->setItem(i, 4, new QTableWidgetItem(QString::fromUtf8(u->getLeaderName())));

        QTableWidgetItem* stItem = new QTableWidgetItem(isAvail ? "Sẵn sàng tại trạm" : "Đang làm nhiệm vụ");
        stItem->setBackground(isAvail ? QColor("#DCFCE7") : QColor("#FEF3C7"));
        stItem->setForeground(isAvail ? QColor("#166534") : QColor("#92400E"));
        stItem->setFont(QFont("Segoe UI", 9, QFont::Bold));
        tblRescueUnits->setItem(i, 5, stItem);
    }
    if (selectedUnitRow >= 0 && selectedUnitRow < uCount) {
        tblRescueUnits->selectRow(selectedUnitRow);
    }

    // --- TAB 4: BẢNG HÓA ĐƠN DỊCH VỤ (BILLING) ---
    tblServiceInvoices->setRowCount(paidServiceCount);
    int billRow = 0;
    for (int i = 0; i < hCount; ++i) {
        EmergencyCase* c = manager.getHistoryCaseAt(i);
        if (!c || !c->isPaidServiceCase()) continue;

        RescueUnit* u = c->getAssignedUnit();
        bool isPaid = (c->getPaymentStatus() == PAYMENT_PAID);

        tblServiceInvoices->setItem(billRow, 0, new QTableWidgetItem(c->getInvoiceID()));
        tblServiceInvoices->setItem(billRow, 1, new QTableWidgetItem(c->getCaseID()));
        tblServiceInvoices->setItem(billRow, 2, new QTableWidgetItem(incidentTypeToString(c->getIncidentType())));
        tblServiceInvoices->setItem(billRow, 3, new QTableWidgetItem(u ? u->getUnitID() : "N/A"));
        tblServiceInvoices->setItem(billRow, 4, new QTableWidgetItem(u ? QString::fromUtf8(u->getLeaderName()) : "N/A"));
        tblServiceInvoices->setItem(billRow, 5, new QTableWidgetItem(formatCurrencyVND(c->getTotalAmountVND())));

        QTableWidgetItem* payItem = new QTableWidgetItem(isPaid ? "ĐÃ THANH TOÁN" : "CHƯA THANH TOÁN");
        payItem->setBackground(isPaid ? QColor("#DCFCE7") : QColor("#FEE2E2"));
        payItem->setForeground(isPaid ? QColor("#166534") : QColor("#991B1B"));
        payItem->setFont(QFont("Segoe UI", 9, QFont::Bold));
        tblServiceInvoices->setItem(billRow, 6, payItem);
        billRow++;
    }

    // --- TAB 5: BÁO CÁO & THỐNG KÊ ---
    tblResolvedReport->setRowCount(resolvedOnlyCount);
    int resRow = 0;
    for (int i = 0; i < hCount; ++i) {
        EmergencyCase* c = manager.getHistoryCaseAt(i);
        if (!c || c->getStatus() != CASE_RESOLVED) continue;

        QString unitStr = c->getAssignedUnit() ? c->getAssignedUnit()->getUnitID() : "N/A";
        const Victim* v = c->getVictim();
        RescueOutcome o = v ? v->getOutcome() : OUTCOME_SAFE_ON_SITE;
        QString outStr = (o == OUTCOME_SAFE_ON_SITE) ? "An toàn / Đã khắc phục"
                         : (o == OUTCOME_HOSPITALIZED) ? "Chuyển viện / Về Gara" : "Không qua khỏi";

        tblResolvedReport->setItem(resRow, 0, new QTableWidgetItem(c->getCaseID()));
        tblResolvedReport->setItem(resRow, 1, new QTableWidgetItem(c->getLocationCode()));
        tblResolvedReport->setItem(resRow, 2, new QTableWidgetItem(incidentTypeToString(c->getIncidentType())));
        tblResolvedReport->setItem(resRow, 3, new QTableWidgetItem(unitStr));
        tblResolvedReport->setItem(resRow, 4, new QTableWidgetItem(formatCurrencyVND(c->getTotalAmountVND())));
        tblResolvedReport->setItem(resRow, 5, new QTableWidgetItem(outStr));
        resRow++;
    }

    int totalUnfinished = activeOnlyCount + qSize;
    tblUnfinishedReport->setRowCount(totalUnfinished);
    int unfRow = 0;
    for (int i = 0; i < hCount; ++i) {
        EmergencyCase* c = manager.getHistoryCaseAt(i);
        if (!c || c->getStatus() == CASE_RESOLVED) continue;
        QString unitStr = c->getAssignedUnit() ? c->getAssignedUnit()->getUnitID() : "N/A";

        tblUnfinishedReport->setItem(unfRow, 0, new QTableWidgetItem(c->getCaseID()));
        tblUnfinishedReport->setItem(unfRow, 1, new QTableWidgetItem(c->getLocationCode()));
        tblUnfinishedReport->setItem(unfRow, 2, new QTableWidgetItem(incidentTypeToString(c->getIncidentType())));
        tblUnfinishedReport->setItem(unfRow, 3, new QTableWidgetItem(unitStr));
        tblUnfinishedReport->setItem(unfRow, 4, new QTableWidgetItem("Đang triển khai tại hiện trường"));
        unfRow++;
    }
    for (int r = 0; r < qSize; ++r) {
        EmergencyCase* c = pq.getCaseAt(r);
        tblUnfinishedReport->setItem(unfRow, 0, new QTableWidgetItem(c->getCaseID()));
        tblUnfinishedReport->setItem(unfRow, 1, new QTableWidgetItem(c->getLocationCode()));
        tblUnfinishedReport->setItem(unfRow, 2, new QTableWidgetItem(incidentTypeToString(c->getIncidentType())));
        tblUnfinishedReport->setItem(unfRow, 3, new QTableWidgetItem("Đang chờ đội rảnh"));
        tblUnfinishedReport->setItem(unfRow, 4, new QTableWidgetItem("Nằm trong Hàng đợi"));
        unfRow++;
    }

    chartWidget->updateChartData(stats);
    lblReportSummaryText->setText(QString(
                                      "<b>• Tổng số hồ sơ tiếp nhận:</b> %1 ca (Đã xong: <b style='color:#16A34A;'>%2</b> | Đang xử lý: <b style='color:#0284C7;'>%3</b>)<br>"
                                      "<b>• Con người được hỗ trợ:</b> <b style='color:#16A34A;'>%4 người</b> | <b>Động vật:</b> <b style='color:#0284C7;'>%5 cá thể</b><br><br>"
                                      "<b>💳 THỐNG KÊ DỊCH VỤ CỨU HỘ CÓ PHÍ:</b><br>"
                                      "  + Số ca dịch vụ (Kéo xe, Thay lốp, Kích bình): <b>%6 ca</b><br>"
                                      "  + Đã thu thanh toán: <b style='color:#16A34A;'>%7</b><br>"
                                      "  + Chờ thanh toán: <b style='color:#DC2626;'>%8</b>"
                                      ).arg(stats.totalCasesReported).arg(stats.resolvedCasesCount).arg(stats.activeDispatchedCount)
                                      .arg(stats.totalHumansRescued).arg(stats.totalAnimalsRescued).arg(stats.paidServiceCasesCount)
                                      .arg(formatCurrencyVND(stats.totalPaidRevenueVND), formatCurrencyVND(stats.totalUnpaidPendingVND)));

    mapCanvas->update();
}

void MainWindow::appendLogMessage(const QString& msg, const QString& badgeColor) {
    QString nowStr = QDateTime::currentDateTime().addSecs(simulatedOffsetMinutes * 60).toString("dd/MM/yyyy HH:mm:ss");
    txtCommandLog->append(QString("<span style='color:#64748B;'>[%1]</span> <b style='color:%2;'>%3</b>")
                              .arg(nowStr, badgeColor, msg));
}

// ============================================================================
// 5. XỬ LÝ SỰ KIỆN TIẾP NHẬN & TỰ ĐỘNG ĐIỀU PHỐI NGAY LẬP TỨC
// ============================================================================
void MainWindow::onIncidentPresetChanged(int index) {
    QCheckBox* allChks[] = {
        chkEquipFirstAid, chkEquipAED, chkEquipOxygen, chkEquipCutter,
        chkEquipTripod, chkEquipShoring, chkEquipFirePump, chkEquipTowCrane,
        chkEquipAnimalCage, chkEquipTranquilizer, chkEquipSpareTire, chkEquipBatteryBooster
    };
    for (int i = 0; i < 12; ++i) allChks[i]->setChecked(false);

    if (index == 1) { // Dịch vụ kéo xe Cầu Rồng
        cboAreaCode->setCurrentIndex(cboAreaCode->findData("HC-02"));
        txtDetailedAddr->setText("Đầu Cầu Rồng, Đường Nguyễn Văn Linh, Q. Hải Châu");
        txtSceneDesc->setText("Xe ô tô 5 chỗ chết máy giữa đường, cần xe cẩu kéo về gara");
        cboIncidentType->setCurrentIndex(6); // INCIDENT_SERVICE_TOWING
        cboUrgency->setCurrentIndex(0);
        cboVictimType->setCurrentIndex(0);
        txtVictimNameOrSpecies->setText("Anh Trần Quốc Bảo");
        txtVictimPhone->setText("0905222333");
        txtMedHistOrFeatures->setText("Biển số 43A-567.89");
        chkEquipTowCrane->setChecked(true);
    } else if (index == 2) { // Dịch vụ thay lốp & kích bình Ngũ Hành Sơn
        cboAreaCode->setCurrentIndex(cboAreaCode->findData("NHS-01"));
        txtDetailedAddr->setText("158 Lê Văn Hiến, P. Mỹ An, Q. Ngũ Hành Sơn");
        txtSceneDesc->setText("Xe thủng lốp sau và hết bình ắc-quy ban đêm");
        cboIncidentType->setCurrentIndex(7); // INCIDENT_SERVICE_TIRE_BATT
        cboUrgency->setCurrentIndex(0);
        cboVictimType->setCurrentIndex(0);
        txtVictimNameOrSpecies->setText("Chị Nguyễn Thị Hương");
        txtVictimPhone->setText("0914888999");
        txtMedHistOrFeatures->setText("Xe Mazda CX-5 màu trắng");
        chkEquipSpareTire->setChecked(true);
        chkEquipBatteryBooster->setChecked(true);
    } else if (index == 3) { // Báo cháy/mắc kẹt ẩn danh (Để trống thông tin cá nhân)
        cboAreaCode->setCurrentIndex(cboAreaCode->findData("LC-01"));
        txtDetailedAddr->setText("Khu công nghiệp Hòa Khánh, P. Hòa Khánh Bắc");
        txtSceneDesc->setText("Người đi đường báo có công nhân kẹt trong hầm công trình");
        cboIncidentType->setCurrentIndex(2); // INCIDENT_COLLAPSED_TUNNEL
        cboUrgency->setCurrentIndex(2);
        cboVictimType->setCurrentIndex(0);
        txtVictimNameOrSpecies->clear(); // Để trống hoàn toàn để kiểm chứng
        txtVictimPhone->clear();
        txtMedHistOrFeatures->clear();
        chkEquipShoring->setChecked(true);
        chkEquipOxygen->setChecked(true);
    } else if (index == 4) { // Giải cứu động vật Sơn Trà
        cboAreaCode->setCurrentIndex(cboAreaCode->findData("ST-02"));
        txtDetailedAddr->setText("Đường lên Bán đảo Sơn Trà, P. An Hải Bắc");
        txtSceneDesc->setText("Voọc chà vá chân nâu mắc bẫy dây thép cần gỡ và sơ cứu");
        cboIncidentType->setCurrentIndex(5); // INCIDENT_ANIMAL_RESCUE
        cboUrgency->setCurrentIndex(1);
        cboVictimType->setCurrentIndex(1);
        txtVictimNameOrSpecies->setText("Voọc Chà Vá Chân Nâu");
        txtMedHistOrFeatures->setText("Nặng khoảng 8.5kg");
        chkEquipAnimalCage->setChecked(true);
        chkEquipFirstAid->setChecked(true);
    }
    updateFeePreviewInForm();
}

void MainWindow::onReportAndAutoDispatchClicked() {
    QByteArray locBytes   = cboAreaCode->currentData().toString().toUtf8();
    QByteArray addrBytes  = txtDetailedAddr->text().trimmed().toUtf8();
    QByteArray descBytes  = txtSceneDesc->text().trimmed().toUtf8();
    QByteArray nameBytes  = txtVictimNameOrSpecies->text().trimmed().toUtf8();
    QByteArray phoneBytes = txtVictimPhone->text().trimmed().toUtf8();
    QByteArray histBytes  = txtMedHistOrFeatures->text().trimmed().toUtf8();

    IncidentType incType = (IncidentType)cboIncidentType->currentData().toInt();
    EmergencyUrgency urg = (EmergencyUrgency)cboUrgency->currentData().toInt();
    Consciousness cons   = (Consciousness)cboConsciousness->currentData().toInt();
    bool breathing       = chkBreathing->isChecked();
    bool trapped         = chkTrapped->isChecked();
    int count            = spnVictimCount->value();

    unsigned int mask = EQUIP_NONE;
    if (chkEquipFirstAid->isChecked())       mask |= EQUIP_FIRST_AID_KIT;
    if (chkEquipAED->isChecked())            mask |= EQUIP_AED_DEFIBRILLATOR;
    if (chkEquipOxygen->isChecked())         mask |= EQUIP_OXYGEN_TANK;
    if (chkEquipCutter->isChecked())         mask |= EQUIP_HYDRAULIC_CUTTER;
    if (chkEquipTripod->isChecked())         mask |= EQUIP_TRIPOD_WINCH;
    if (chkEquipShoring->isChecked())        mask |= EQUIP_SHORING_JACKS;
    if (chkEquipFirePump->isChecked())       mask |= EQUIP_FIRE_HOSE_PUMP;
    if (chkEquipTowCrane->isChecked())       mask |= EQUIP_HEAVY_TOW_CRANE;
    if (chkEquipAnimalCage->isChecked())     mask |= EQUIP_ANIMAL_CAGE_NET;
    if (chkEquipTranquilizer->isChecked())   mask |= EQUIP_TRANQUILIZER_GUN;
    if (chkEquipSpareTire->isChecked())      mask |= EQUIP_SPARE_TIRE_KIT;
    if (chkEquipBatteryBooster->isChecked()) mask |= EQUIP_BATTERY_BOOSTER;
    Victim* v = nullptr;
    if (cboVictimType->currentIndex() == 0) {
        v = new HumanVictim("VIC-NEW", nameBytes.constData(), phoneBytes.constData(), 1,
                            CustomDateTime(1, 1, 1990), addrBytes.constData(),
                            histBytes.constData(), cons, breathing, trapped, count);
    } else {
        v = new AnimalVictim("VIC-ANIM", nameBytes.constData(), histBytes.constData(),
                             phoneBytes.constData(), 15.0f, true, cons, breathing, trapped, count);
    }

    DispatchResult autoRes;
    char logBuf[512];
    EmergencyCase* created = manager.reportAndAutoDispatch(
        locBytes.constData(), addrBytes.constData(), descBytes.constData(),
        incType, urg, mask, v, autoRes, logBuf, sizeof(logBuf)
        );

    if (autoRes.bestUnit != nullptr) {
        mapCanvas->setHighlightRoute(autoRes.routeDescription,
                                     autoRes.bestUnit->getStationLocationCode(),
                                     created->getLocationCode());
        appendLogMessage(QString::fromUtf8(logBuf), "#16A34A");
    } else {
        appendLogMessage(QString::fromUtf8(logBuf), "#D97706");
    }

    // Chuyển ngay về Tab 1 (Bản đồ) để người điều khiển thấy lộ trình vừa tự động điều phối
    btnNavDashboard->setChecked(true);
    onNavigationPageChanged(0);
}

void MainWindow::onAdvanceAgingTimeClicked() {
    simulatedOffsetMinutes += 15;
    manager.advanceSystemTimeMinutes(15);
    refreshAllViews();
    appendLogMessage("⏱ Đã cộng thêm 15 phút chờ: Điểm ưu tiên của các ca trong hàng đợi đã được cập nhật.", "#D97706");
}

void MainWindow::onCompleteSelectedCaseClicked() {
    int row = tblLiveOperations->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Hướng dẫn", "Hãy nhấp chọn một ca đang triển khai trong bảng 'Các Ca Đang Triển Khai' trước khi bấm Hoàn thành!");
        return;
    }

    QString caseId = tblLiveOperations->item(row, 0)->text();
    QByteArray idBytes = caseId.toUtf8();

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Xác Nhận Hoàn Thành Nhiệm Vụ");
    msgBox.setText(QString("Chọn kết quả sau khi hoàn thành hồ sơ [%1]:").arg(caseId));
    QPushButton* btnSafe = msgBox.addButton("An toàn tại chỗ / Đã sửa xong", QMessageBox::AcceptRole);
    QPushButton* btnHosp = msgBox.addButton("Chuyển viện / Đưa xe về Gara", QMessageBox::YesRole);
    msgBox.addButton("Hủy", QMessageBox::RejectRole);
    msgBox.exec();

    RescueOutcome outcome = OUTCOME_SAFE_ON_SITE;
    if (msgBox.clickedButton() == btnHosp) {
        outcome = OUTCOME_HOSPITALIZED;
    } else if (msgBox.clickedButton() != btnSafe) {
        return;
    }

    char logBuf[512];
    if (manager.completeActiveCase(idBytes.constData(), outcome, logBuf, sizeof(logBuf))) {
        appendLogMessage(QString::fromUtf8(logBuf), "#16A34A");
        mapCanvas->clearHighlight();
        refreshAllViews();
    }
}

// ============================================================================
// 6. CÁC SLOTS CHO TAB 3: QUẢN LÝ ĐỘI TRƯỞNG & THÀNH VIÊN
// ============================================================================
void MainWindow::onRescueUnitSelectionChanged() {
    int row = tblRescueUnits->currentRow();
    if (row < 0 || row >= manager.getTotalUnitCount()) return;

    RescueUnit* u = manager.getUnitAt(row);
    if (!u) return;

    lblSelectedTeamHeader->setText(QString(
                                       "<b style='color:#0284C7; font-size:13px;'>[%1] %2</b><br>"
                                       "• <b>Chuyên môn:</b> %3 | <b>Trạm đóng quân:</b> %4<br>"
                                       "• <b>Phương tiện:</b> %5<br>"
                                       "• <b>Số ca hoàn thành:</b> %6 ca | <b>Tổng doanh thu đội:</b> <span style='color:#16A34A; font-weight:bold;'>%7</span>"
                                       ).arg(u->getUnitID(), QString::fromUtf8(u->getUnitName()),
                                            QString::fromUtf8(u->getSpecialtyName()), u->getStationLocationCode(),
                                            QString::fromUtf8(u->getVehicleType())).arg(u->getCompletedMissions())
                                       .arg(formatCurrencyVND(u->getTotalRevenueVND())));

    txtEditLeaderName->setText(QString::fromUtf8(u->getLeaderName()));
    txtEditLeaderPhone->setText(QString::fromUtf8(u->getLeaderPhone()));

    lstTeamMembers->clear();
    lstTeamMembers->addItem(QString("⭐ [ĐỘI TRƯỞNG] %1 - SĐT: %2")
                                .arg(QString::fromUtf8(u->getLeaderName()), QString::fromUtf8(u->getLeaderPhone())));
    for (int i = 0; i < u->getMemberCount(); ++i) {
        lstTeamMembers->addItem(QString("   %1. %2").arg(i + 1).arg(QString::fromUtf8(u->getMemberNameAt(i))));
    }
}

void MainWindow::onUpdateLeaderInfoClicked() {
    int row = tblRescueUnits->currentRow();
    if (row < 0 || row >= manager.getTotalUnitCount()) return;

    RescueUnit* u = manager.getUnitAt(row);
    QByteArray nameBytes  = txtEditLeaderName->text().trimmed().toUtf8();
    QByteArray phoneBytes = txtEditLeaderPhone->text().trimmed().toUtf8();

    if (nameBytes.isEmpty()) return;
    u->setLeaderInfo(nameBytes.constData(), phoneBytes.constData());
    manager.saveUnitsToFile("RescueUnits.txt");

    refreshAllViews();
    onRescueUnitSelectionChanged();
    appendLogMessage(QString("👨‍✈️ Đã cập nhật Đội trưởng mới cho đội [%1]: %2 (%3).")
                         .arg(u->getUnitID(), QString::fromUtf8(u->getLeaderName()), u->getLeaderPhone()), "#0284C7");
}

void MainWindow::onAddTeamMemberClicked() {
    int row = tblRescueUnits->currentRow();
    if (row < 0 || row >= manager.getTotalUnitCount()) return;

    RescueUnit* u = manager.getUnitAt(row);
    QByteArray memBytes = txtNewMemberName->text().trimmed().toUtf8();
    if (memBytes.isEmpty()) return;

    if (u->getMemberCount() >= MAX_TEAM_MEMBERS) {
        QMessageBox::warning(this, "Thông báo", "Đội này đã đạt số lượng thành viên tối đa (8 người)!");
        return;
    }

    u->addTeamMember(memBytes.constData());
    txtNewMemberName->clear();
    manager.saveUnitsToFile("RescueUnits.txt");
    onRescueUnitSelectionChanged();
}

void MainWindow::onCreateNewRescueUnitClicked() {
    QByteArray idBytes      = txtNewUnitID->text().trimmed().toUtf8();
    QByteArray nameBytes    = txtNewUnitName->text().trimmed().toUtf8();
    QByteArray areaBytes    = cboNewUnitArea->currentText().toUtf8();
    QByteArray vehicleBytes = txtNewUnitVehicle->text().trimmed().toUtf8();
    QByteArray leaderBytes  = txtNewUnitLeader->text().trimmed().toUtf8();
    QByteArray phoneBytes   = txtNewUnitPhone->text().trimmed().toUtf8();

    if (idBytes.isEmpty() || nameBytes.isEmpty()) {
        QMessageBox::warning(this, "Thông báo", "Vui lòng nhập Mã đội và Tên phiên hiệu đội!");
        return;
    }

    SpecialtyType spec = (SpecialtyType)cboNewUnitSpec->currentData().toInt();
    RescueUnit* added = manager.addNewRescueUnit(spec, idBytes.constData(), nameBytes.constData(),
                                                 areaBytes.constData(), vehicleBytes.constData(),
                                                 leaderBytes.constData(), phoneBytes.constData());
    if (added) {
        added->addTeamMember("KTV. Nguyễn Văn An");
        added->addTeamMember("LX. Trần Văn Bình");
        refreshAllViews();
        tblRescueUnits->selectRow(manager.getTotalUnitCount() - 1);
        appendLogMessage(QString("🚑 Đã thành lập đội cứu hộ mới [%1 - %2] đóng quân tại [%3].")
                             .arg(added->getUnitID(), QString::fromUtf8(added->getUnitName()), added->getStationLocationCode()), "#16A34A");
    }
}

// ============================================================================
// 7. CÁC SLOTS CHO TAB 4: LẬP HÓA ĐƠN & THANH TOÁN CHO ĐỘI CỨU HỘ
// ============================================================================
static EmergencyCase* findPaidCaseByCaseID(DispatchManager& manager, const QString& caseId) {
    QByteArray b = caseId.toUtf8();
    for (int i = 0; i < manager.getHistoryCaseCount(); ++i) {
        EmergencyCase* c = manager.getHistoryCaseAt(i);
        if (c && strcmp(c->getCaseID(), b.constData()) == 0) return c;
    }
    return nullptr;
}

void MainWindow::onInvoiceRowSelected() {
    int row = tblServiceInvoices->currentRow();
    if (row < 0) return;

    QString caseId = tblServiceInvoices->item(row, 1)->text();
    EmergencyCase* c = findPaidCaseByCaseID(manager, caseId);
    if (!c) return;

    RescueUnit* u = c->getAssignedUnit();
    char eqBuf[128];
    c->formatEquippedItemsText(eqBuf, sizeof(eqBuf));
    char vicBuf[256];
    c->getVictim()->formatDetails(vicBuf, sizeof(vicBuf));

    bool isPaid = (c->getPaymentStatus() == PAYMENT_PAID);

    lblInvoiceDetailPreview->setText(QString(
                                         "<b style='font-size:14px; color:#0284C7;'>PHIẾU HÓA ĐƠN DỊCH VỤ: %1</b><br>"
                                         "=========================================<br>"
                                         "• <b>Mã sự cố:</b> %2 | <b>Khu vực:</b> %3 (%4)<br>"
                                         "• <b>Khách hàng:</b> %5<br>"
                                         "• <b>Nội dung dịch vụ:</b> %6<br>"
                                         "• <b>Đội thực hiện:</b> %7 (%8)<br>"
                                         "• <b>Đội trưởng phụ trách:</b> %9 (%10)<br>"
                                         "• <b>Thiết bị sử dụng:</b> %11<br><br>"
                                         "<b>CHI TIẾT BIỂU PHÍ DỊCH VỤ:</b><br>"
                                         "  1. Phí dịch vụ cơ bản : <b>%12</b><br>"
                                         "  2. Phụ phí thiết bị   : <b>%13</b><br>"
                                         "  3. Phí quãng đường    : <b>%14</b><br>"
                                         "-----------------------------------------<br>"
                                         "👉 <b>TỔNG CỘNG THANH TOÁN: <span style='color:#DC2626; font-size:14px;'>%15</span></b><br>"
                                         "• <b>Trạng thái:</b> <b style='color:%16;'>%17</b>"
                                         ).arg(c->getInvoiceID(), c->getCaseID(), c->getLocationCode(), QString::fromUtf8(c->getDetailedAddress()),
                                              QString::fromUtf8(vicBuf), QString::fromUtf8(c->getSceneDescription()),
                                              u ? u->getUnitID() : "N/A", u ? QString::fromUtf8(u->getUnitName()) : "N/A",
                                              u ? QString::fromUtf8(u->getLeaderName()) : "N/A", u ? u->getLeaderPhone() : "N/A",
                                              QString::fromUtf8(eqBuf),
                                              formatCurrencyVND(c->getBaseServiceFeeVND()),
                                              formatCurrencyVND(c->getEquipmentSurchargeVND()),
                                              formatCurrencyVND(c->getDistanceFeeVND()),
                                              formatCurrencyVND(c->getTotalAmountVND()),
                                              isPaid ? "#16A34A" : "#DC2626",
                                              isPaid ? "ĐÃ THANH TOÁN CHO ĐỘI CỨU HỘ" : "CHƯA THANH TOÁN"));
}

void MainWindow::onConfirmInvoicePaymentClicked() {
    int row = tblServiceInvoices->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Hướng dẫn", "Hãy nhấp chọn một Hóa đơn trong bảng bên trái để xác nhận thanh toán!");
        return;
    }

    QString caseId = tblServiceInvoices->item(row, 1)->text();
    QByteArray idBytes = caseId.toUtf8();

    char logBuf[512];
    if (manager.payServiceInvoice(idBytes.constData(), logBuf, sizeof(logBuf))) {
        appendLogMessage(QString::fromUtf8(logBuf), "#16A34A");
        refreshAllViews();
        tblServiceInvoices->selectRow(row);
        onInvoiceRowSelected();
        QMessageBox::information(this, "Thanh Toán Thành Công", QString::fromUtf8(logBuf));
    } else {
        QMessageBox::warning(this, "Thông báo", QString::fromUtf8(logBuf));
    }
}

void MainWindow::onExportSelectedInvoiceClicked() {
    int row = tblServiceInvoices->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Hướng dẫn", "Hãy chọn một Hóa đơn trong bảng bên trái để xuất phiếu thu!");
        return;
    }

    QString caseId = tblServiceInvoices->item(row, 1)->text();
    EmergencyCase* c = findPaidCaseByCaseID(manager, caseId);
    if (!c) return;

    char fileName[64];
    snprintf(fileName, sizeof(fileName), "PhieuThu_%s.txt", c->getInvoiceID());

    FILE* fp = fopen(fileName, "w");
    if (fp) {
        RescueUnit* u = c->getAssignedUnit();
        char eqBuf[128];
        c->formatEquippedItemsText(eqBuf, sizeof(eqBuf));

        fprintf(fp, "===============================================================\n");
        fprintf(fp, "       PHIEU HOA DON DICH VU CUU HO GIAO THONG TP. DA NANG\n");
        fprintf(fp, "===============================================================\n");
        fprintf(fp, "Ma Hoa Don      : %s\n", c->getInvoiceID());
        fprintf(fp, "Ma Ho So Su Co  : %s\n", c->getCaseID());
        fprintf(fp, "Khu Vuc         : %s - %s\n", c->getLocationCode(), c->getDetailedAddress());
        fprintf(fp, "Noi Dung Dich Vu: %s\n", c->getSceneDescription());
        fprintf(fp, "Doi Thuc Hien   : %s (%s)\n", u ? u->getUnitID() : "N/A", u ? u->getUnitName() : "");
        fprintf(fp, "Doi Truong      : %s - SDT: %s\n", u ? u->getLeaderName() : "N/A", u ? u->getLeaderPhone() : "");
        fprintf(fp, "Thiet Bi Di Kem : %s\n", eqBuf);
        fprintf(fp, "---------------------------------------------------------------\n");
        fprintf(fp, "1. Phi dich vu co ban : %12.0f VND\n", c->getBaseServiceFeeVND());
        fprintf(fp, "2. Phu phi thiet bi   : %12.0f VND\n", c->getEquipmentSurchargeVND());
        fprintf(fp, "3. Phi quang duong    : %12.0f VND\n", c->getDistanceFeeVND());
        fprintf(fp, "---------------------------------------------------------------\n");
        fprintf(fp, "TONG CONG THANH TOAN  : %12.0f VND\n", c->getTotalAmountVND());
        fprintf(fp, "TRANG THAI            : %s\n",
                (c->getPaymentStatus() == PAYMENT_PAID) ? "DA THANH TOAN" : "CHUA THANH TOAN");
        fprintf(fp, "===============================================================\n");
        fclose(fp);

        QString absPath = QFileInfo(fileName).absoluteFilePath();
        QDesktopServices::openUrl(QUrl::fromLocalFile(absPath));
        appendLogMessage(QString("🖨 Đã xuất và mở trực tiếp phiếu hóa đơn tại: %1").arg(absPath), "#0284C7");
    }
}

// ============================================================================
// 8. XUẤT FILE BÁO CÁO TỔNG HỢP & ĐỔI CA TRỰC
// ============================================================================
void MainWindow::onExportReportToFileClicked() {
    const char* reportFileName = "FinalReport_DaNang.txt";
    SystemStatistics s = manager.computeStatistics();
    FILE* fp = fopen(reportFileName, "w");
    if (fp) {
        fprintf(fp, "=====================================================================\n");
        fprintf(fp, "     BAO CAO TONG HOP DIEU PHOI CUU HO & DICH VU TP. DA NANG\n");
        fprintf(fp, "=====================================================================\n");
        fprintf(fp, "Tong dai vien truc ban: [%s] %s (%s)\n\n",
                currentAdminID, currentAdminName, currentAdminShift);

        fprintf(fp, "1. TONG QUAN HO SO SU CO & DICH VU:\n");
        fprintf(fp, "   - Tong so ho so tiep nhan    : %d ca\n", s.totalCasesReported);
        fprintf(fp, "   - So ca da hoan thanh        : %d ca\n", s.resolvedCasesCount);
        fprintf(fp, "   - So ca dang trien khai      : %d ca\n", s.activeDispatchedCount);
        fprintf(fp, "   - So ca dich vu co phi       : %d ca\n", s.paidServiceCasesCount);
        fprintf(fp, "   - Tong doanh thu da thu      : %.0f VND\n\n", s.totalPaidRevenueVND);

        fprintf(fp, "2. DANH SACH 16 DOI CUU HO & DOANH THU:\n");
        for (int i = 0; i < manager.getTotalUnitCount(); ++i) {
            RescueUnit* u = manager.getUnitAt(i);
            fprintf(fp, "   [%s] %s | Doi truong: %s (%s) | So ca: %d | Doanh thu: %.0f VND\n",
                    u->getUnitID(), u->getUnitName(), u->getLeaderName(), u->getLeaderPhone(),
                    u->getCompletedMissions(), u->getTotalRevenueVND());
        }
        fclose(fp);

        QString absPath = QFileInfo(reportFileName).absoluteFilePath();
        QDesktopServices::openUrl(QUrl::fromLocalFile(absPath));
        appendLogMessage(QString("💾 Đã xuất và mở trực tiếp file báo cáo tại: %1").arg(absPath), "#0284C7");
    }
}

void MainWindow::setLoggedInDispatcher(const Dispatcher* admin) {
    if (!admin) return;
    strncpy(currentAdminID, admin->getStaffID(), MAX_ID_LEN - 1);
    strncpy(currentAdminName, admin->getFullName(), MAX_NAME_LEN - 1);
    strncpy(currentAdminShift, admin->getShiftCode(), MAX_ID_LEN - 1);

    if (lblAdminBadge) {
        lblAdminBadge->setText(QString(
                                   "<span style='color:#38BDF8; font-weight:bold;'>👤 TRỰC BAN: %1</span><br>"
                                   "<b>%2</b><br>"
                                   "<span style='color:#94A3B8;'>Ca trực: %3</span>"
                                   ).arg(currentAdminID, QString::fromUtf8(currentAdminName), currentAdminShift));
    }
    appendLogMessage(QString("🔐 Tổng đài viên [%1 - %2] đã nhận bàn giao [%3].")
                         .arg(currentAdminID, QString::fromUtf8(currentAdminName), currentAdminShift), "#0284C7");
}

void MainWindow::onSwitchShiftLogoutClicked() {
    LoginDialog loginDlg(this);
    if (loginDlg.exec() == QDialog::Accepted && loginDlg.getAuthenticatedAdmin() != nullptr) {
        setLoggedInDispatcher(loginDlg.getAuthenticatedAdmin());
    }
}