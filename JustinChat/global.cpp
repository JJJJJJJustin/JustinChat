#include "global.h"

QString gateUrlPrefix = "";

std::function<void(QWidget*)> refresh = [](QWidget* w)
{
    w->style()->unpolish(w);
    w->style()->polish(w);
};
