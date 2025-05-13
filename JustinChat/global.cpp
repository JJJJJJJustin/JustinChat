#include "global.h"

std::function<void(QWidget*)> refresh = [](QWidget* w)
{
    w->style()->unpolish(w);
    w->style()->polish(w);
};
