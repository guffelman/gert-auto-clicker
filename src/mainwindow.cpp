#include "mainwindow.h"
#include <QApplication>
#include <QCloseEvent>
#include <QMessageBox>
#include <QIcon>
#include <QStyle>
#include <QDebug> // Added for qDebug

#ifdef Q_OS_MAC
#include <ApplicationServices/ApplicationServices.h>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , isClicking(false)
    , clickCount(0)
    , testClickCount(0)
{
    setWindowTitle("Gert Auto Clicker");
    setFixedSize(500, 400);  // Increased height for new controls
    setWindowIcon(style()->standardIcon(QStyle::SP_ComputerIcon));
    
    autoClicker = new AutoClicker(this);
    hotkeyManager = new HotkeyManager(this);
    
    setupUI();
    setupTrayIcon();
    loadSettings();
    
    // Check accessibility permissions on macOS
#ifdef Q_OS_MAC
    if (!AXIsProcessTrusted()) {
        QMessageBox::warning(this, "Accessibility Permissions Required",
            "This application requires accessibility permissions to perform mouse clicks.\n\n"
            "Please enable accessibility for this app in:\n"
            "System Preferences > Security & Privacy > Privacy > Accessibility\n\n"
            "After enabling permissions, restart the application.",
            QMessageBox::Ok);
    }
#endif
    
    // Connect signals

    

    
    connect(hotkeyManager, &HotkeyManager::hotkeyPressed, this, &MainWindow::toggleClicking);
    
    // Status update timer disabled for maximum performance
}

MainWindow::~MainWindow()
{
    saveSettings();
    if (autoClicker->isRunning()) {
        autoClicker->stop();
    }
    hotkeyManager->unregisterHotkey();
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget;
    setCentralWidget(centralWidget);
    
    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Click Settings Group
    clickSettingsGroup = new QGroupBox("Click Settings");
    clickSettingsLayout = new QGridLayout(clickSettingsGroup);
    
    // Click Mode (at the top)
    clickModeLabel = new QLabel("Click Mode:");
    clickModeCombo = new QComboBox;
    clickModeCombo->addItems({"Rapid Click", "Interval Click"});
    connect(clickModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::updateClickMode);
    
    // Rapid Click Settings
    cpsLabel = new QLabel("Clicks Per Second:");
    cpsSpinBox = new QSpinBox;
    cpsSpinBox->setRange(1, 10000); // 1 to 10,000 CPS
    cpsSpinBox->setValue(10); // Default 10 CPS
    cpsSpinBox->setSuffix(" CPS");
    connect(cpsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::updateCPS);
    
    // Interval Click Settings
    intervalDelayLabel = new QLabel("Interval (seconds):");
    intervalDelaySpinBox = new QSpinBox;
    intervalDelaySpinBox->setRange(1, 1000); // 1 second to 1000 seconds
    intervalDelaySpinBox->setValue(1);
    intervalDelaySpinBox->setSuffix(" s");
    connect(intervalDelaySpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::updateIntervalDelay);
    
    // Click type
    clickTypeLabel = new QLabel("Click Type:");
    clickTypeCombo = new QComboBox;
    clickTypeCombo->addItems({"Left Click", "Right Click", "Middle Click", "Double Click"});
    connect(clickTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::updateClickType);
    
    // Mouse mode
    mouseModeLabel = new QLabel("Mouse Mode:");
    mouseModeCombo = new QComboBox;
    mouseModeCombo->addItems({"Unlocked", "Locked"});
    connect(mouseModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::updateMouseMode);
    
    // Hotkey
    hotkeyLabel = new QLabel("Hotkey:");
    hotkeyButton = new QPushButton("F6");
    currentHotkey = "F6";
    connect(hotkeyButton, &QPushButton::clicked, this, &MainWindow::updateHotkey);
    
    // Add to layout
    clickSettingsLayout->addWidget(clickModeLabel, 0, 0);
    clickSettingsLayout->addWidget(clickModeCombo, 0, 1);
    clickSettingsLayout->addWidget(cpsLabel, 1, 0);
    clickSettingsLayout->addWidget(cpsSpinBox, 1, 1);
    clickSettingsLayout->addWidget(intervalDelayLabel, 2, 0);
    clickSettingsLayout->addWidget(intervalDelaySpinBox, 2, 1);
    clickSettingsLayout->addWidget(clickTypeLabel, 3, 0);
    clickSettingsLayout->addWidget(clickTypeCombo, 3, 1);
    clickSettingsLayout->addWidget(mouseModeLabel, 4, 0);
    clickSettingsLayout->addWidget(mouseModeCombo, 4, 1);
    clickSettingsLayout->addWidget(hotkeyLabel, 5, 0);
    clickSettingsLayout->addWidget(hotkeyButton, 5, 1);
    
    mainLayout->addWidget(clickSettingsGroup);
    
    // Control buttons
    controlLayout = new QHBoxLayout;
    
    startStopButton = new QPushButton("Start (F6)");
    startStopButton->setStyleSheet(
        "QPushButton { background-color: #2ecc71; color: white; border: none; padding: 10px; border-radius: 5px; }"
        "QPushButton:hover { background-color: #27ae60; }"
        "QPushButton:pressed { background-color: #229954; }"
    );
    connect(startStopButton, &QPushButton::clicked, this, &MainWindow::toggleClicking);
    
    minimizeButton = new QPushButton("Minimize to Tray");
    minimizeButton->setStyleSheet(
        "QPushButton { background-color: #3498db; color: white; border: none; padding: 10px; border-radius: 5px; }"
        "QPushButton:hover { background-color: #2980b9; }"
        "QPushButton:pressed { background-color: #21618c; }"
    );
    connect(minimizeButton, &QPushButton::clicked, this, &MainWindow::minimizeToTray);
    
    // Add test click button
    QPushButton *testClickButton = new QPushButton("Test Click");
    testClickButton->setStyleSheet(
        "QPushButton { background-color: #f39c12; color: white; border: none; padding: 10px; border-radius: 5px; }"
        "QPushButton:hover { background-color: #e67e22; }"
        "QPushButton:pressed { background-color: #d35400; }"
    );
    connect(testClickButton, &QPushButton::clicked, this, [this, testClickButton]() {
        testClickCount++;
        // No GUI updates for maximum performance
        qDebug() << "Test click button pressed - count:" << testClickCount;
    });
    
    controlLayout->addWidget(startStopButton);
    controlLayout->addWidget(minimizeButton);
    controlLayout->addWidget(testClickButton);
    
    mainLayout->addLayout(controlLayout);
    
    // Status
    statusLabel = new QLabel("Status: Ready");
    githubLink = new QLabel("https://github.com/guffelman/gert-auto-clicker");
    statusLabel->setStyleSheet("QLabel { color: #95a5a6; font-weight: bold; }");
    githubLink->setStyleSheet("QLabel { color: #95a5a6; font-weight: bold; }");
    mainLayout->addWidget(statusLabel);
    mainLayout->addWidget(githubLink);
    mainLayout->addStretch();
    
    // Register initial hotkey
    hotkeyManager->registerHotkey(currentHotkey);
}

void MainWindow::setupTrayIcon()
{
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(style()->standardIcon(QStyle::SP_ComputerIcon));
    trayIcon->setToolTip("Gert Auto Clicker");
    
    trayMenu = new QMenu;
    showAction = new QAction("Show", this);
    startStopAction = new QAction("Start", this);
    quitAction = new QAction("Quit", this);
    
    connect(showAction, &QAction::triggered, this, &MainWindow::restoreFromTray);
    connect(startStopAction, &QAction::triggered, this, &MainWindow::toggleClicking);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
    
    trayMenu->addAction(showAction);
    trayMenu->addSeparator();
    trayMenu->addAction(startStopAction);
    trayMenu->addSeparator();
    trayMenu->addAction(quitAction);
    
    trayIcon->setContextMenu(trayMenu);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::trayIconActivated);
    
    trayIcon->show();
}

void MainWindow::toggleClicking()
{
    if (isClicking) {
        autoClicker->stop();
        isClicking = false;
        startStopButton->setText("Start (F6)");
        startStopButton->setStyleSheet(
            "QPushButton { background-color: #2ecc71; color: white; border: none; padding: 10px; border-radius: 5px; }"
            "QPushButton:hover { background-color: #27ae60; }"
            "QPushButton:pressed { background-color: #229954; }"
        );
        startStopAction->setText("Start");
        // No GUI updates for maximum performance
    } else {
        autoClicker->start();
        isClicking = true;
        startStopButton->setText("Stop (F6)");
        startStopButton->setStyleSheet(
            "QPushButton { background-color: #e74c3c; color: white; border: none; padding: 10px; border-radius: 5px; }"
            "QPushButton:hover { background-color: #c0392b; }"
            "QPushButton:pressed { background-color: #a93226; }"
        );
        startStopAction->setText("Stop");
        // No GUI updates for maximum performance
    }
}

void MainWindow::updateClickMode()
{
    int mode = clickModeCombo->currentIndex();
    
    // Show/hide appropriate controls based on mode
    if (mode == 0) { // Rapid Click
        cpsLabel->setVisible(true);
        cpsSpinBox->setVisible(true);
        intervalDelayLabel->setVisible(false);
        intervalDelaySpinBox->setVisible(false);
        
        // Update auto clicker for rapid clicking
        autoClicker->setIntervalClick(false);
        updateCPS(); // Apply current CPS setting
    } else { // Interval Click
        cpsLabel->setVisible(false);
        cpsSpinBox->setVisible(false);
        intervalDelayLabel->setVisible(true);
        intervalDelaySpinBox->setVisible(true);
        
        // Update auto clicker for interval clicking
        updateIntervalDelay(); // Apply current interval setting
    }
}

void MainWindow::updateCPS()
{
    int cps = cpsSpinBox->value();
    autoClicker->setClicksPerSecond(cps);
}

void MainWindow::updateIntervalDelay()
{
    int seconds = intervalDelaySpinBox->value();
    autoClicker->setIntervalClick(true, seconds * 1000); // Convert to milliseconds
}

void MainWindow::updateMouseMode()
{
    MouseMode mode = static_cast<MouseMode>(mouseModeCombo->currentIndex());
    autoClicker->setMouseMode(mode);
}

void MainWindow::updateClickType()
{
    ClickType type = static_cast<ClickType>(clickTypeCombo->currentIndex());
    autoClicker->setClickType(type);
}

void MainWindow::updateHotkey()
{
    // Simple hotkey selection - in a real app you'd have a proper hotkey editor
    QStringList hotkeys = {"F6", "F7", "F8", "F9", "F10", "F11", "F12"};
    static int currentIndex = 0;
    currentIndex = (currentIndex + 1) % hotkeys.size();
    currentHotkey = hotkeys[currentIndex];
    hotkeyButton->setText(currentHotkey);
    hotkeyManager->unregisterHotkey();
    hotkeyManager->registerHotkey(currentHotkey);
}

void MainWindow::minimizeToTray()
{
    hide();
    trayIcon->showMessage("Gert Auto Clicker", "Minimized to system tray", QSystemTrayIcon::Information, 2000);
}

void MainWindow::restoreFromTray()
{
    show();
    raise();
    activateWindow();
}

void MainWindow::showTrayMenu()
{
    // This function is called when the tray icon context menu is shown
    // We can update the menu items here if needed
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        restoreFromTray();
    }
}

void MainWindow::loadSettings()
{
    QSettings settings;
    clickModeCombo->setCurrentIndex(settings.value("clickMode", 0).toInt());
    cpsSpinBox->setValue(settings.value("clicksPerSecond", 10).toInt()); // Load CPS with default of 10
    intervalDelaySpinBox->setValue(settings.value("intervalClickDelay", 1).toInt()); // Load as seconds
    clickTypeCombo->setCurrentIndex(settings.value("clickType", 0).toInt());
    mouseModeCombo->setCurrentIndex(settings.value("mouseMode", 0).toInt());
    currentHotkey = settings.value("hotkey", "F6").toString();
    hotkeyButton->setText(currentHotkey);
    
    // Apply the click mode to show/hide appropriate controls
    updateClickMode();
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.setValue("clickMode", clickModeCombo->currentIndex());
    settings.setValue("clicksPerSecond", cpsSpinBox->value()); // Save CPS
    settings.setValue("intervalClickDelay", intervalDelaySpinBox->value()); // Save as seconds
    settings.setValue("clickType", clickTypeCombo->currentIndex());
    settings.setValue("mouseMode", mouseModeCombo->currentIndex());
    settings.setValue("hotkey", currentHotkey);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (trayIcon->isVisible()) {
        // Save settings before closing
        saveSettings();
        
        // Stop auto clicker if running
        if (autoClicker->isRunning()) {
            autoClicker->stop();
        }
        
        // Clean up hotkey manager
        hotkeyManager->unregisterHotkey();
        
        // Hide tray icon
        trayIcon->hide();
        
        event->accept();
    } else {
        event->accept();
    }
} 