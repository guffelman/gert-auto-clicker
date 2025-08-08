#ifndef AUTOCLICKER_H
#define AUTOCLICKER_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QPoint>
#include <atomic>
#include "types.h"

class ClickerThread;

class AutoClicker : public QObject
{
    Q_OBJECT

public:
    explicit AutoClicker(QObject *parent = nullptr);
    ~AutoClicker();

    void setInterval(int value); // Legacy method
    void setClicksPerSecond(int clicksPerSecond);
    void setClickType(ClickType type);
    void setMouseMode(MouseMode mode);
    void setClickLimit(int limit);
    void setClickPosition(const QPoint &pos);
    void setUseCurrentPosition(bool useCurrent);
    void setIntervalClick(bool enabled, int delayMs = 1000);

    void start();
    void stop();
    bool isRunning() const;
    int getClickCount() const;
    void resetClickCount();
    void performTestClick(); // Add this method for testing
    


signals:
    void clickPerformed();
    void clickCountChanged(int count);
    void statusChanged(const QString &status);
    void performanceUpdate(double clicksPerSecond); // New signal for performance updates

private slots:
    void onClickPerformed();
    void ultraSpeedLoop();

private:
    ClickerThread *clickerThread;
    QTimer *clickTimer;
    

    
    // Ultra-speed thread for bypassing Qt event loop
    QThread *ultraSpeedThread;
    std::atomic<bool> ultraSpeedRunning;
    
    int clicksPerSecond;
    int intervalMs; // Calculated from clicks per second
    ClickType clickType;
    MouseMode mouseMode;
    int clickLimit;
    int clickCount;
    QPoint clickPosition;
    bool useCurrentPosition;
    bool running;
    
    // Interval click functionality
    bool intervalClickEnabled;
    int intervalClickDelayMs;
    QTimer *intervalClickTimer;
    bool firstClickPerformed;
    std::atomic<bool> stopRequested; // Atomic flag for immediate stop requests
    
    void updateTimerInterval();
    void startUltraSpeedThread();
    void stopUltraSpeedThread();
};

#endif // AUTOCLICKER_H 