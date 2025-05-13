#ifndef GLOBAL_H
#define GLOBAL_H

#include <QWidget>
#include <QRegularExpression>
#include <functional>
#include "QStyle"

extern std::function<void(QWidget*)> refresh;

#endif // GLOBAL_H
