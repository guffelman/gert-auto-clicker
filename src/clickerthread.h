#ifndef CLICKERTHREAD_H
#define CLICKERTHREAD_H

#include <QThread>
#include <QPoint>
#include <QMutex>
#include <QWaitCondition>
#include <atomic>
#include "types.h"

class ClickerThread : public QThread
{
    Q_OBJECT

public:
    explicit ClickerThread(QObject *parent = nullptr);
    ~ClickerThread();

    void setClickType(ClickType type);
    void setClickPosition(const QPoint &pos);
    void setUseCurrentPosition(bool useCurrent);
    void performClick();

protected:
    void run() override;

private:
    void performMouseClick(ClickType type, const QPoint &pos);
    
#ifdef Q_OS_WIN
    void performWindowsClick(ClickType type, const QPoint &pos);
#elif defined(Q_OS_MAC)
    void performMacClick(ClickType type, const QPoint &pos);
#else
    void performLinuxClick(ClickType type, const QPoint &pos);
#endif

    ClickType clickType;
    QPoint clickPosition;
    std::atomic<bool> useCurrentPosition;
    QMutex mutex;
    QWaitCondition condition;
    bool shouldClick;
    bool running;
};

#endif // CLICKERTHREAD_H 