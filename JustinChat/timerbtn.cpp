#include "timerbtn.h"
#include <QMouseEvent>

TimerBtn::TimerBtn(QWidget* parent)
    :QPushButton(parent), m_Counter(10)
{
    m_Timer = new QTimer(this);

    connect(m_Timer, &QTimer::timeout, [this]()
        {
            m_Counter--;
            if(m_Counter <= 0)
            {
                m_Timer->stop();
                m_Counter = 10;          // 若十秒计时完成，则终止 Timer 并刷新 m_Counter 的值
                this->setText("获取");    // 这里的 this 指的是 QPushButton 的子类 TimerBtn，代指当前操作的 QPushButton。（具体的 button 信息通过参数 parent 填入）
                this->setEnabled(true);
                return;
            }
            this->setText(QString::number(m_Counter));
        }
    );
}

TimerBtn::~TimerBtn()
{
    m_Timer->stop();

}

void TimerBtn::mouseReleaseEvent(QMouseEvent* event)
{
    qDebug("Mouse release event triggered!");
    if(event->button() == Qt::LeftButton)
    {
        qDebug("Code verify button is released!");

        m_Counter = 10;
        this->setEnabled(false);
        this->setText(QString::number(m_Counter));
        m_Timer->start(1000);

        emit clicked();
    }

    // 在此处调用基类的 mouseReleaseEvent()，确保 Timer 的效果可以正确运行
    QPushButton::mouseReleaseEvent(event);
}
