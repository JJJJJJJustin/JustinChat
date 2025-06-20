#ifndef TIMERBTN_H
#define TIMERBTN_H

#include <QPushButton>
#include <QTimer>

class TimerBtn : public QPushButton
{
public:
    TimerBtn(QWidget* parent = nullptr);
    ~TimerBtn();

    virtual void mouseReleaseEvent(QMouseEvent* event) override;
private:
    QTimer* m_Timer;
    int m_Counter;
};

#endif // TIMERBTN_H
