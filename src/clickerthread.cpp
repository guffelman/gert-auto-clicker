#include "clickerthread.h"
#include <QDebug>
#include <QApplication>
#include <QCursor>

#ifdef Q_OS_WIN
#include <windows.h>
#elif defined(Q_OS_MAC)
#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#else
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#endif

ClickerThread::ClickerThread(QObject *parent)
    : QThread(parent)
    , clickType(ClickType::LeftClick)
    , useCurrentPosition(true)
    , shouldClick(false)
    , running(false)
{
}

ClickerThread::~ClickerThread()
{
    running = false;
    condition.wakeAll();
    quit();
    wait();
}

void ClickerThread::setClickType(ClickType type)
{
    clickType = type;
}

void ClickerThread::setClickPosition(const QPoint &pos)
{
    clickPosition = pos;
}

void ClickerThread::setUseCurrentPosition(bool useCurrent)
{
    useCurrentPosition.store(useCurrent);
}

void ClickerThread::performClick()
{
    // Cache frequently accessed atomic value
    const bool useCurrent = useCurrentPosition.load(std::memory_order_relaxed);
    
    // Get position and type with minimal overhead
    const QPoint currentPos = useCurrent ? QCursor::pos() : clickPosition;
    const ClickType currentType = clickType;
    
    performMouseClick(currentType, currentPos);
}

void ClickerThread::run()
{
    running = true;
    
    while (running) {
        mutex.lock();
        while (!shouldClick && running) {
            condition.wait(&mutex);
        }
        
        if (!running) {
            mutex.unlock();
            break;
        }
        
        shouldClick = false;
        QPoint pos = useCurrentPosition ? QCursor::pos() : clickPosition;
        ClickType type = clickType;
        mutex.unlock();
        
        performMouseClick(type, pos);
    }
}

void ClickerThread::performMouseClick(ClickType type, const QPoint &pos)
{
#ifdef Q_OS_WIN
    performWindowsClick(type, pos);
#elif defined(Q_OS_MAC)
    performMacClick(type, pos);
#else
    performLinuxClick(type, pos);
#endif
}

#ifdef Q_OS_WIN
void ClickerThread::performWindowsClick(ClickType type, const QPoint &pos)
{
    INPUT input[3] = {};
    int inputCount = 0;
    
    // Set cursor position
    input[inputCount].type = INPUT_MOUSE;
    input[inputCount].mi.dx = pos.x() * (65535 / GetSystemMetrics(SM_CXSCREEN));
    input[inputCount].mi.dy = pos.y() * (65535 / GetSystemMetrics(SM_CYSCREEN));
    input[inputCount].mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
    inputCount++;
    
    // Mouse button events
    DWORD downFlag = 0, upFlag = 0;
    
    switch (type) {
        case ClickType::LeftClick:
            downFlag = MOUSEEVENTF_LEFTDOWN;
            upFlag = MOUSEEVENTF_LEFTUP;
            break;
        case ClickType::RightClick:
            downFlag = MOUSEEVENTF_RIGHTDOWN;
            upFlag = MOUSEEVENTF_RIGHTUP;
            break;
        case ClickType::MiddleClick:
            downFlag = MOUSEEVENTF_MIDDLEDOWN;
            upFlag = MOUSEEVENTF_MIDDLEUP;
            break;
        case ClickType::DoubleClick:
            downFlag = MOUSEEVENTF_LEFTDOWN;
            upFlag = MOUSEEVENTF_LEFTUP;
            break;
    }
    
    // Mouse down
    input[inputCount].type = INPUT_MOUSE;
    input[inputCount].mi.dwFlags = downFlag;
    inputCount++;
    
    // Mouse up
    input[inputCount].type = INPUT_MOUSE;
    input[inputCount].mi.dwFlags = upFlag;
    inputCount++;
    
    SendInput(inputCount, input, sizeof(INPUT));
    
    // For double click, add a second click
    if (type == ClickType::DoubleClick) {
        // Minimal delay for ultra-high speed double clicks
        input[0].type = INPUT_MOUSE;
        input[0].mi.dwFlags = downFlag;
        input[1].type = INPUT_MOUSE;
        input[1].mi.dwFlags = upFlag;
        
        SendInput(2, input, sizeof(INPUT));
    }
}
#endif

#ifdef Q_OS_MAC
void ClickerThread::performMacClick(ClickType type, const QPoint &pos)
{
    // Check if we have accessibility permissions
    if (!AXIsProcessTrusted()) {
        return;
    }
    
    CGPoint cgPos = CGPointMake(pos.x(), pos.y());
    
    // Move cursor to position
    CGWarpMouseCursorPosition(cgPos);
    
    // Determine click type
    CGEventType downType, upType;
    CGMouseButton button;
    
    switch (type) {
        case ClickType::LeftClick:
            downType = kCGEventLeftMouseDown;
            upType = kCGEventLeftMouseUp;
            button = kCGMouseButtonLeft;
            break;
        case ClickType::RightClick:
            downType = kCGEventRightMouseDown;
            upType = kCGEventRightMouseUp;
            button = kCGMouseButtonRight;
            break;
        case ClickType::MiddleClick:
            downType = kCGEventOtherMouseDown;
            upType = kCGEventOtherMouseUp;
            button = kCGMouseButtonCenter;
            break;
        case ClickType::DoubleClick:
            downType = kCGEventLeftMouseDown;
            upType = kCGEventLeftMouseUp;
            button = kCGMouseButtonLeft;
            break;
    }
    
    // Create and post mouse events
    CGEventRef downEvent = CGEventCreateMouseEvent(nullptr, downType, cgPos, button);
    CGEventRef upEvent = CGEventCreateMouseEvent(nullptr, upType, cgPos, button);
    
    if (!downEvent || !upEvent) {
        if (downEvent) CFRelease(downEvent);
        if (upEvent) CFRelease(upEvent);
        return;
    }
    
    // Post the events
    CGEventPost(kCGHIDEventTap, downEvent);
    // No delay for ultra-high speeds - let the system handle timing
    CGEventPost(kCGHIDEventTap, upEvent);
    
    // For double click, add a second click
    if (type == ClickType::DoubleClick) {
        // Minimal delay for double click - system will handle timing
        CGEventRef downEvent2 = CGEventCreateMouseEvent(nullptr, downType, cgPos, button);
        CGEventRef upEvent2 = CGEventCreateMouseEvent(nullptr, upType, cgPos, button);
        
        if (downEvent2 && upEvent2) {
            CGEventPost(kCGHIDEventTap, downEvent2);
            CGEventPost(kCGHIDEventTap, upEvent2);
        }
        
        if (downEvent2) CFRelease(downEvent2);
        if (upEvent2) CFRelease(upEvent2);
    }
    
    CFRelease(downEvent);
    CFRelease(upEvent);
}
#endif

#ifndef Q_OS_WIN
#ifndef Q_OS_MAC
void ClickerThread::performLinuxClick(ClickType type, const QPoint &pos)
{
    Display *display = XOpenDisplay(nullptr);
    if (!display) return;
    
    Window root = DefaultRootWindow(display);
    
    // Move cursor to position
    XWarpPointer(display, None, root, 0, 0, 0, 0, pos.x(), pos.y());
    XFlush(display);
    
    // Determine button
    int button;
    switch (type) {
        case ClickType::LeftClick:
            button = Button1;
            break;
        case ClickType::RightClick:
            button = Button3;
            break;
        case ClickType::MiddleClick:
            button = Button2;
            break;
        case ClickType::DoubleClick:
            button = Button1;
            break;
    }
    
    // Perform click
    XTestFakeButtonEvent(display, button, True, 0);
    XTestFakeButtonEvent(display, button, False, 0);
    
    // For double click, add a second click
    if (type == ClickType::DoubleClick) {
        // Minimal delay for ultra-high speed double clicks
        XTestFakeButtonEvent(display, button, True, 0);
        XTestFakeButtonEvent(display, button, False, 0);
    }
    
    XFlush(display);
    XCloseDisplay(display);
}
#endif
#endif 