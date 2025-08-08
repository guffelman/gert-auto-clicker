#include "autoclicker.h"
#include "clickerthread.h"
#include <QDebug>
#include <QDateTime>
#include <QApplication>
#ifdef Q_OS_WIN
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <chrono>
#include <thread>

AutoClicker::AutoClicker(QObject *parent)
    : QObject(parent)
    , clicksPerSecond(10)
    , intervalMs(100)
    , clickType(ClickType::LeftClick)
    , mouseMode(MouseMode::Unlocked)
    , clickLimit(999999)
    , clickCount(0)
    , useCurrentPosition(true)
    , running(false)
    , intervalClickEnabled(false)
    , intervalClickDelayMs(1000)
    , firstClickPerformed(false)
    , stopRequested(false)
    , ultraSpeedRunning(false)
{
    clickerThread = new ClickerThread(this);
    clickerThread->start();
    
    clickTimer = new QTimer(this);
    clickTimer->setSingleShot(false);
    
    // Initialize ultra-speed thread
    ultraSpeedThread = new QThread(this);
    
    // Initialize interval click timer
    intervalClickTimer = new QTimer(this);
    intervalClickTimer->setSingleShot(true);
    connect(intervalClickTimer, &QTimer::timeout, this, [this]() {
        if (intervalClickEnabled && running && !stopRequested) {
            // Perform the interval click
            clickerThread->performClick();
            clickCount++;
            
            // Process events to keep hotkeys responsive
            QApplication::processEvents();
            
            // Restart the interval timer
            intervalClickTimer->start(intervalClickDelayMs);
        }
    });
    
    // Performance monitoring disabled for maximum speed
    
    connect(clickTimer, &QTimer::timeout, this, &AutoClicker::onClickPerformed);
    connect(clickerThread, &ClickerThread::finished, this, [this]() {
        // ClickerThread finished - let it run forever
    });
}

AutoClicker::~AutoClicker()
{
    stop();
    if (clickerThread) {
        clickerThread->quit();
        clickerThread->wait();
    }
}

void AutoClicker::setInterval(int value)
{
    // Legacy method - convert to clicks per second
    int cps = 1000 / value; // Convert milliseconds to clicks per second
    setClicksPerSecond(cps);
}

void AutoClicker::setClicksPerSecond(int cps)
{
    clicksPerSecond = qBound(1, cps, 10000);
    intervalMs = 1000 / clicksPerSecond;
    
    if (running) {
        updateTimerInterval();
    }
}

void AutoClicker::setClickType(ClickType type)
{
    clickType = type;
    clickerThread->setClickType(type);
}

void AutoClicker::setMouseMode(MouseMode mode)
{
    mouseMode = mode;
}

void AutoClicker::setIntervalClick(bool enabled, int delayMs)
{
    intervalClickEnabled = enabled;
    intervalClickDelayMs = delayMs;
    
    if (enabled && running) {
        // Start interval timer
        intervalClickTimer->start(delayMs);
    } else if (!enabled) {
        // Stop interval timer
        intervalClickTimer->stop();
    }
}

void AutoClicker::setClickLimit(int limit)
{
    clickLimit = limit;
}

void AutoClicker::setClickPosition(const QPoint &pos)
{
    clickPosition = pos;
    clickerThread->setClickPosition(pos);
}

void AutoClicker::setUseCurrentPosition(bool useCurrent)
{
    useCurrentPosition = useCurrent;
    clickerThread->setUseCurrentPosition(useCurrent);
}

void AutoClicker::start()
{
    if (running) return;
    
    running = true;
    stopRequested.store(false); // Reset stop flag
    clickCount = 0;
    firstClickPerformed = false;
    
    // Capture position only on start based on current mouse mode
    if (mouseMode == MouseMode::Locked) {
        // Lock to current position when starting
        clickPosition = QCursor::pos();
        useCurrentPosition = false;
    } else {
        // Unlocked mode: always use current position
        useCurrentPosition = true;
    }
    
    // Set up clicker thread
    clickerThread->setClickType(clickType);
    clickerThread->setClickPosition(clickPosition);
    clickerThread->setUseCurrentPosition(useCurrentPosition);
    
    // Performance monitoring disabled for maximum speed
    
    // Start the timer or ultra-speed thread
    updateTimerInterval();
    
    // Start interval click timer if enabled
    if (intervalClickEnabled) {
        intervalClickTimer->start(intervalClickDelayMs);
    }
    
    emit statusChanged("Auto-clicker started");
}

void AutoClicker::stop()
{
    if (!running) return;
    
    stopRequested.store(true);
    running = false;
    clickTimer->stop();
    intervalClickTimer->stop();
    
    // Clear captured position on stop
    clickPosition = QPoint(0, 0);
    useCurrentPosition = true;
    
    // Stop ultra-speed thread with timeout
    if (ultraSpeedRunning.load()) {
        ultraSpeedRunning.store(false);
        
        if (ultraSpeedThread) {
            ultraSpeedThread->quit();
            
            // Wait up to 50ms for graceful shutdown
            if (!ultraSpeedThread->wait(50)) {
                ultraSpeedThread->terminate();
                ultraSpeedThread->wait();
            }
        }
    }
    
    emit statusChanged("Auto-clicker stopped");
}

bool AutoClicker::isRunning() const
{
    return running;
}

int AutoClicker::getClickCount() const
{
    return clickCount;
}

void AutoClicker::resetClickCount()
{
    clickCount = 0;
    emit clickCountChanged(clickCount);
}

void AutoClicker::performTestClick()
{
    if (clickerThread) {
        clickerThread->performClick();
    }
}

void AutoClicker::updateTimerInterval()
{
    // For interval click mode, don't start ultra-speed thread
    if (intervalClickEnabled) {
        stopUltraSpeedThread();
        return;
    }
    
    // Use dedicated thread for rapid click mode
    stopUltraSpeedThread();
    startUltraSpeedThread();
}

void AutoClicker::onClickPerformed()
{
    // This method is now only used for interval click mode
    // Regular clicking is handled by the dedicated thread
    if (!running || stopRequested) return;
    
    // Handle interval click mode
    if (intervalClickEnabled) {
        if (!firstClickPerformed) {
            // First click - perform immediately
            clickerThread->performClick();
            clickCount++;
            firstClickPerformed = true;
            
            emit clickPerformed();
            emit clickCountChanged(clickCount);
            
            // Stop the regular timer and let interval timer handle the rest
            clickTimer->stop();
        }
        // Subsequent clicks are handled by intervalClickTimer
        return;
    }
    
    // For regular mode, this should not be called anymore since we use dedicated thread
}

 

void AutoClicker::startUltraSpeedThread()
{
    if (ultraSpeedRunning.load()) return;
    
    ultraSpeedRunning.store(true);
    
    // Stop the regular timer to avoid conflicts
    clickTimer->stop();
    
    // Create a lambda function for the thread
    auto threadFunc = [this]() {
        this->ultraSpeedLoop();
    };
    
    // Start the thread with the lambda
    ultraSpeedThread = QThread::create(threadFunc);
    ultraSpeedThread->start();
}

void AutoClicker::stopUltraSpeedThread()
{
    if (!ultraSpeedRunning.load()) return;
    
    ultraSpeedRunning.store(false);
    
    if (ultraSpeedThread) {
        ultraSpeedThread->quit();
        if (!ultraSpeedThread->wait(100)) {
            ultraSpeedThread->terminate();
            ultraSpeedThread->wait();
        }
    }
}

void AutoClicker::ultraSpeedLoop()
{
    // Calculate interval in microseconds
    const int intervalUs = 1000000 / clicksPerSecond;
    
    // Cache frequently accessed values
    const bool highSpeedMode = clicksPerSecond >= 1000;
    const int stopCheckInterval = highSpeedMode ? 10000 : 500;
    
    int stopCheckCounter = 0;
    
    // Use high-resolution clock for precise timing
    auto lastClickTime = std::chrono::high_resolution_clock::now();
    
    while (ultraSpeedRunning.load(std::memory_order_relaxed) && 
           running && 
           !stopRequested.load(std::memory_order_relaxed)) {
        
        const auto currentTime = std::chrono::high_resolution_clock::now();
        const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastClickTime);
        
        // Only perform click if enough time has passed
        if (elapsed.count() >= intervalUs) {
            // Perform the click
            clickerThread->performClick();
            ++clickCount;
            ++stopCheckCounter;
            
            // Update last click time
            lastClickTime = currentTime;
            
            // Reduce stop flag checking frequency for ultra-high speeds
            if (stopCheckCounter >= stopCheckInterval) {
                stopCheckCounter = 0;
                // Only yield CPU for lower speeds to prevent system lockup
                if (!highSpeedMode) {
                    std::this_thread::yield();
                }
            }
        } else if (!highSpeedMode) {
            // Only sleep for lower speeds - ultra-high speed mode stays in tight loop
            std::this_thread::yield();
        }
    }
} 