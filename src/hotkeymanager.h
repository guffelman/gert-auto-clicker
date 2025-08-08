#ifndef HOTKEYMANAGER_H
#define HOTKEYMANAGER_H

#include <QObject>
#include <QString>
#include <QKeySequence>
#include <QTimer>

#ifdef Q_OS_WIN
#include <windows.h>
#include <winuser.h>
#elif defined(Q_OS_MAC)
#include <Carbon/Carbon.h>
#else
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#endif

class HotkeyManager : public QObject
{
    Q_OBJECT

public:
    explicit HotkeyManager(QObject *parent = nullptr);
    ~HotkeyManager();

    bool registerHotkey(const QString &keySequence);
    void unregisterHotkey();
    QString getCurrentHotkey() const;
    bool isHotkeyRegistered() const;

signals:
    void hotkeyPressed();

private:
    void setupGlobalHotkey();
    void cleanupGlobalHotkey();
    
#ifdef Q_OS_WIN
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    HWND hwnd;
    int hotkeyId;
#elif defined(Q_OS_MAC)
    EventHandlerRef eventHandler;
    EventHotKeyRef hotKeyRef;
    EventHotKeyID hotKeyID;
#else
    Display *display;
    Window root;
    int hotkeyKeycode;
    unsigned int hotkeyModifiers;
    QTimer *checkTimer;
#endif

    QString currentHotkey;
    bool registered;
};

#endif // HOTKEYMANAGER_H 