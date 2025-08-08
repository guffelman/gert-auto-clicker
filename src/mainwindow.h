#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QSlider>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QSettings>
#include "autoclicker.h"
#include "hotkeymanager.h"
#include "types.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void toggleClicking();
    void updateClickMode();
    void updateCPS();
    void updateMouseMode();
    void updateClickType();
    void updateIntervalDelay();
    void updateHotkey();
    void minimizeToTray();
    void restoreFromTray();
    void showTrayMenu();
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void loadSettings();
    void saveSettings();
    void closeEvent(QCloseEvent *event) override;

private:
    void setupUI();
    void setupTrayIcon();

    // UI Components
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    
    // Click Settings
    QGroupBox *clickSettingsGroup;
    QGridLayout *clickSettingsLayout;
    
    // Click Mode (at the top)
    QLabel *clickModeLabel;
    QComboBox *clickModeCombo;
    
    // Rapid Click Settings
    QLabel *cpsLabel;
    QSpinBox *cpsSpinBox;
    
    // Interval Click Settings
    QLabel *intervalDelayLabel;
    QSpinBox *intervalDelaySpinBox;
    
    // General Settings
    QLabel *clickTypeLabel;
    QComboBox *clickTypeCombo;
    QLabel *mouseModeLabel;
    QComboBox *mouseModeCombo;
    QLabel *hotkeyLabel;
    QPushButton *hotkeyButton;
    
    // Control Buttons
    QHBoxLayout *controlLayout;
    QPushButton *startStopButton;
    QPushButton *minimizeButton;
    
    // Status
    QLabel *statusLabel;
    QLabel *githubLink;
    
    // Core Components
    AutoClicker *autoClicker;
    HotkeyManager *hotkeyManager;
    
    // Tray
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QAction *showAction;
    QAction *startStopAction;
    QAction *quitAction;
    
    // State
    bool isClicking;
    int clickCount;
    int testClickCount; // Counter for test clicks
    QString currentHotkey;
};

#endif // MAINWINDOW_H 