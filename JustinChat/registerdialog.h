#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include "global.h"
#include <QDialog>
#include <type_traits>

// 这个宏填入的参数必须是 std::function<bool()> 类型的函数名称 ！！！
#define CHECK_VALID(func){\
    bool valid = func();\
    if(!valid)\
    {\
        return;\
    }\
}

namespace Ui
{
    class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();

private slots:
    void on_get_code_clicked();
    void on_confirm_button_clicked();

    void SlotRegModeFinish(QString res, ReqID reqID, ErrorCode errCode);

private:
    void ShowTip(QString str, bool state);
    void InitHandlersMap();

    void AddErrTip(ErrTipType, QString);
    void DeleteErrTip(ErrTipType);
    bool CheckUserValid();
    bool CheckEmailValid();
    bool CheckPassValid();
    bool CheckConfirmValid();
    bool CheckVerifyValid();
private:
    Ui::RegisterDialog *ui;
    QMap<ReqID, std::function<void(const QJsonObject&)>> m_HandlersMap;

    QMap<ErrTipType, QString> m_ErrTips;
};

#endif // REGISTERDIALOG_H
