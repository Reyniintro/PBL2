#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QStackedWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QListWidget>
#include <QLabel>
#include <QTextEdit>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QWidget>
#include <QButtonGroup>
#include <QTimer>
#include "DispatchManager.h"

#define MAX_DISPATCHERS 10

class LoginDialog : public QDialog {
    Q_OBJECT
private:
    Dispatcher* staffList[MAX_DISPATCHERS];
    int staffCount;
    Dispatcher* authenticatedAdmin;

    QLineEdit* txtUsername;
    QLineEdit* txtPassword;
    QComboBox* cboQuickAccount;
    QLabel* lblErrorMsg;

    void loadOrGenerateAccounts(const char* filePath);

private slots:
    void onLoginButtonClicked();
    void onQuickAccountSelected(int index);

public:
    explicit LoginDialog(QWidget* parent = nullptr);
    ~LoginDialog();

    Dispatcher* getAuthenticatedAdmin() const { return authenticatedAdmin; }
};

class DaNangMapCanvas : public QWidget {
    Q_OBJECT
private:
    const SpatialGraph* graph;
    char highlightedRoute[MAX_DESC_LEN];
    char activeIncidentArea[MAX_ID_LEN];
    char activeStationArea[MAX_ID_LEN];

public:
    explicit DaNangMapCanvas(const SpatialGraph* g, QWidget* parent = nullptr);
    void setHighlightRoute(const char* routeDesc, const char* stationCode, const char* incidentCode);
    void clearHighlight();

protected:
    void paintEvent(QPaintEvent* event) override;
};

class AnalyticsChartWidget : public QWidget {
    Q_OBJECT
private:
    SystemStatistics currentStats;

public:
    explicit AnalyticsChartWidget(QWidget* parent = nullptr);
    void updateChartData(const SystemStatistics& stats);

protected:
    void paintEvent(QPaintEvent* event) override;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    DispatchManager manager;
    QTimer* realTimeClockTimer;
    int simulatedOffsetMinutes;

    char currentAdminID[MAX_ID_LEN];
    char currentAdminName[MAX_NAME_LEN];
    char currentAdminShift[MAX_ID_LEN];
    QLabel* lblAdminBadge;

    QStackedWidget* mainStackedWidget;
    QButtonGroup* navBtnGroup;
    QPushButton* btnNavDashboard;
    QPushButton* btnNavIntake;
    QPushButton* btnNavTeamManager;
    QPushButton* btnNavBilling;
    QPushButton* btnNavAnalytics;

    QLabel* lblSystemClock;
    QLabel* lblKpiActive;
    QLabel* lblKpiPending;
    QLabel* lblKpiRescuedTotal;
    QLabel* lblKpiRevenue;

    DaNangMapCanvas* mapCanvas;
    QTableWidget* tblLiveOperations;
    QTableWidget* tblPendingQueue;
    QTextEdit* txtCommandLog;

    QLineEdit* txtAutoCaseID;
    QComboBox* cboAreaCode;
    QLineEdit* txtDetailedAddr;
    QLineEdit* txtSceneDesc;
    QComboBox* cboIncidentType;
    QComboBox* cboUrgency;
    QLabel* lblEstimatedFeePreview;

    QComboBox* cboVictimType;
    QLineEdit* txtVictimNameOrSpecies;
    QLineEdit* txtVictimPhone;
    QLineEdit* txtMedHistOrFeatures;
    QComboBox* cboConsciousness;
    QCheckBox* chkBreathing;
    QCheckBox* chkTrapped;
    QSpinBox* spnVictimCount;

    QCheckBox* chkEquipFirstAid;
    QCheckBox* chkEquipAED;
    QCheckBox* chkEquipOxygen;
    QCheckBox* chkEquipCutter;
    QCheckBox* chkEquipTripod;
    QCheckBox* chkEquipShoring;
    QCheckBox* chkEquipFirePump;
    QCheckBox* chkEquipTowCrane;
    QCheckBox* chkEquipAnimalCage;
    QCheckBox* chkEquipTranquilizer;
    QCheckBox* chkEquipSpareTire;
    QCheckBox* chkEquipBatteryBooster;

    QTableWidget* tblRescueUnits;
    QLabel* lblSelectedTeamHeader;
    QLineEdit* txtEditLeaderName;
    QLineEdit* txtEditLeaderPhone;
    QListWidget* lstTeamMembers;
    QLineEdit* txtNewMemberName;

    QComboBox* cboNewUnitSpec;
    QLineEdit* txtNewUnitID;
    QLineEdit* txtNewUnitName;
    QComboBox* cboNewUnitArea;
    QLineEdit* txtNewUnitVehicle;
    QLineEdit* txtNewUnitLeader;
    QLineEdit* txtNewUnitPhone;

    QTableWidget* tblServiceInvoices;
    QLabel* lblInvoiceDetailPreview;

    AnalyticsChartWidget* chartWidget;
    QTableWidget* tblResolvedReport;
    QTableWidget* tblUnfinishedReport;
    QLabel* lblReportSummaryText;

    void setupEnterpriseLightUI();
    QWidget* createPage1Dashboard();
    QWidget* createPage2SosIntake();
    QWidget* createPage3TeamManagement();
    QWidget* createPage4ServiceBilling();
    QWidget* createPage5AnalyticsReport();
    void applyLightDesignSystemStylesheet();

    void syncRealTimeClockToManager();
    void refreshAllViews();
    void updateFeePreviewInForm();
    void appendLogMessage(const QString& msg, const QString& badgeColor = "#0284C7");

private slots:
    void onRealTimeTick();
    void onNavigationPageChanged(int pageIndex);
    void onReportAndAutoDispatchClicked();
    void onAdvanceAgingTimeClicked();
    void onCompleteSelectedCaseClicked();
    void onIncidentPresetChanged(int index);
    void onIncidentOrEquipChanged();

    void onRescueUnitSelectionChanged();
    void onUpdateLeaderInfoClicked();
    void onAddTeamMemberClicked();
    void onCreateNewRescueUnitClicked();

    void onInvoiceRowSelected();
    void onConfirmInvoicePaymentClicked();
    void onExportSelectedInvoiceClicked();

    void onExportReportToFileClicked();
    void onSwitchShiftLogoutClicked();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setLoggedInDispatcher(const Dispatcher* admin);
};

#endif // MAINWINDOW_H