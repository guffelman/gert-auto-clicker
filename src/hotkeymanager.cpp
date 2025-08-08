#include "hotkeymanager.h"
#include <QDebug>
#include <QKeySequence>

#ifdef Q_OS_WIN
#include <QWidget>
#include <QApplication>
#endif

HotkeyManager::HotkeyManager(QObject *parent)
    : QObject(parent)
    , registered(false)
#ifdef Q_OS_WIN
    , hwnd(nullptr)
    , hotkeyId(1)
#elif defined(Q_OS_MAC)
    , eventHandler(nullptr)
    , hotKeyRef(nullptr)
#else
    , display(nullptr)
    , root(0)
    , hotkeyKeycode(0)
    , hotkeyModifiers(0)
    , checkTimer(nullptr)
#endif
{
#ifdef Q_OS_WIN
    // Create a hidden window for receiving hotkey messages
    hwnd = CreateWindowEx(
        0, L"STATIC", L"HotkeyWindow",
        WS_OVERLAPPED, 0, 0, 0, 0,
        nullptr, nullptr, GetModuleHandle(nullptr), nullptr
    );
    
    if (hwnd) {
        SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)windowProc);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
    }
#elif !defined(Q_OS_MAC)
    checkTimer = new QTimer(this);
    connect(checkTimer, &QTimer::timeout, this, [this]() {
        // Check for hotkey press on Linux
        if (display && hotkeyKeycode) {
            char keys[32];
            XQueryKeymap(display, keys);
            if (keys[hotkeyKeycode / 8] & (1 << (hotkeyKeycode % 8))) {
                emit hotkeyPressed();
            }
        }
    });
#endif
}

HotkeyManager::~HotkeyManager()
{
    unregisterHotkey();
    
#ifdef Q_OS_WIN
    if (hwnd) {
        DestroyWindow(hwnd);
        hwnd = nullptr;
    }
#elif defined(Q_OS_MAC)
    cleanupGlobalHotkey();
#else
    if (display) {
        XCloseDisplay(display);
        display = nullptr;
    }
#endif
}

bool HotkeyManager::registerHotkey(const QString &keySequence)
{
    unregisterHotkey();
    
    currentHotkey = keySequence;
    
#ifdef Q_OS_WIN
    // Parse key sequence for Windows
    QKeySequence seq(keySequence);
    if (seq.isEmpty()) return false;
    
    int key = seq[0].toCombined() & 0xFFFF;
    int modifiers = 0;
    
    if (seq[0].keyboardModifiers() & Qt::ControlModifier) modifiers |= MOD_CONTROL;
    if (seq[0].keyboardModifiers() & Qt::AltModifier) modifiers |= MOD_ALT;
    if (seq[0].keyboardModifiers() & Qt::ShiftModifier) modifiers |= MOD_SHIFT;
    if (seq[0].keyboardModifiers() & Qt::MetaModifier) modifiers |= MOD_WIN;
    
    // Convert Qt key to Windows virtual key
    if (key >= Qt::Key_F1 && key <= Qt::Key_F12) {
        key = VK_F1 + (key - Qt::Key_F1);
    }
    
    if (RegisterHotKey(hwnd, hotkeyId, modifiers, key)) {
        registered = true;
        return true;
    }
    
#elif defined(Q_OS_MAC)
    // Parse key sequence for macOS
    QKeySequence seq(keySequence);
    if (seq.isEmpty()) return false;
    
    int qtKey = seq[0].key();
    int modifiers = 0;
    
    if (seq[0].keyboardModifiers() & Qt::ControlModifier) modifiers |= cmdKey;
    if (seq[0].keyboardModifiers() & Qt::AltModifier) modifiers |= optionKey;
    if (seq[0].keyboardModifiers() & Qt::ShiftModifier) modifiers |= shiftKey;
    if (seq[0].keyboardModifiers() & Qt::MetaModifier) modifiers |= cmdKey;
    
    // Convert Qt key to macOS key code
    int key = 0;
    if (qtKey >= Qt::Key_F1 && qtKey <= Qt::Key_F12) {
        // Map F keys to correct macOS virtual key codes
        const int fKeyMap[] = {
            0x7A, // F1 - kVK_F1
            0x78, // F2 - kVK_F2
            0x63, // F3 - kVK_F3
            0x76, // F4 - kVK_F4
            0x60, // F5 - kVK_F5
            0x61, // F6 - kVK_F6
            0x62, // F7 - kVK_F7
            0x64, // F8 - kVK_F8
            0x65, // F9 - kVK_F9
            0x6D, // F10 - kVK_F10
            0x67, // F11 - kVK_F11
            0x6F  // F12 - kVK_F12
        };
        int fIndex = qtKey - Qt::Key_F1;
        if (fIndex >= 0 && fIndex < 12) {
            key = fKeyMap[fIndex];
        }
    } else {
        key = qtKey; // For other keys, use Qt key directly
    }
    
    EventTypeSpec eventType;
    eventType.eventClass = kEventClassKeyboard;
    eventType.eventKind = kEventHotKeyPressed;
    
    // Create event handler function
    auto eventHandlerFunc = [](EventHandlerCallRef nextHandler, EventRef theEvent, void *userData) -> OSStatus {
        HotkeyManager *manager = static_cast<HotkeyManager*>(userData);
        emit manager->hotkeyPressed();
        return noErr;
    };
    
    InstallEventHandler(GetApplicationEventTarget(), 
                       NewEventHandlerUPP(eventHandlerFunc), 
                       1, &eventType, this, &eventHandler);
    
    hotKeyID.signature = 'htk1';
    hotKeyID.id = 1;
    
    if (RegisterEventHotKey(key, modifiers, hotKeyID, GetApplicationEventTarget(), 0, &hotKeyRef) == noErr) {
        registered = true;
        return true;
    }
    
#else
    // Parse key sequence for Linux/X11
    display = XOpenDisplay(nullptr);
    if (!display) return false;
    
    root = DefaultRootWindow(display);
    
    QKeySequence seq(keySequence);
    if (seq.isEmpty()) return false;
    
    int key = seq[0].toCombined() & 0xFFFF;
    int modifiers = 0;
    
    if (seq[0].keyboardModifiers() & Qt::ControlModifier) modifiers |= ControlMask;
    if (seq[0].keyboardModifiers() & Qt::AltModifier) modifiers |= Mod1Mask;
    if (seq[0].keyboardModifiers() & Qt::ShiftModifier) modifiers |= ShiftMask;
    if (seq[0].keyboardModifiers() & Qt::MetaModifier) modifiers |= Mod4Mask;
    
    // Convert Qt key to X11 keycode
    if (key >= Qt::Key_F1 && key <= Qt::Key_F12) {
        hotkeyKeycode = XKeysymToKeycode(display, XK_F1 + (key - Qt::Key_F1));
    } else {
        hotkeyKeycode = XKeysymToKeycode(display, key);
    }
    
    hotkeyModifiers = modifiers;
    registered = true;
    checkTimer->start(50); // Check every 50ms
    return true;
#endif
    
    return false;
}

void HotkeyManager::unregisterHotkey()
{
    if (!registered) return;
    
#ifdef Q_OS_WIN
    if (hwnd) {
        UnregisterHotKey(hwnd, hotkeyId);
    }
#elif defined(Q_OS_MAC)
    cleanupGlobalHotkey();
#else
    if (checkTimer) {
        checkTimer->stop();
    }
    if (display) {
        XCloseDisplay(display);
        display = nullptr;
    }
#endif
    
    registered = false;
}

QString HotkeyManager::getCurrentHotkey() const
{
    return currentHotkey;
}

bool HotkeyManager::isHotkeyRegistered() const
{
    return registered;
}

#ifdef Q_OS_WIN
LRESULT CALLBACK HotkeyManager::windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_HOTKEY) {
        HotkeyManager *manager = reinterpret_cast<HotkeyManager*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (manager) {
            emit manager->hotkeyPressed();
        }
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
#endif

#ifdef Q_OS_MAC
void HotkeyManager::cleanupGlobalHotkey()
{
    if (hotKeyRef) {
        UnregisterEventHotKey(hotKeyRef);
        hotKeyRef = nullptr;
    }
    if (eventHandler) {
        RemoveEventHandler(eventHandler);
        eventHandler = nullptr;
    }
}
#endif 