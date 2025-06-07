#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include "global.h"
#include <QDialog>

namespace Ui {
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
private:
    Ui::RegisterDialog *ui;
    QMap<ReqID, std::function<void(const QJsonObject&)>> m_HandlersMap;
};

#endif // REGISTERDIALOG_H
