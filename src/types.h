#ifndef TYPES_H
#define TYPES_H

#include <QPoint>

enum class ClickType {
    LeftClick,
    RightClick,
    MiddleClick,
    DoubleClick
};

enum class MouseMode {
    Unlocked,  // Normal clicking - mouse can move freely
    Locked     // Mouse position is locked - clicks at fixed position
};

#endif // TYPES_H 