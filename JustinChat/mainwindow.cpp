#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":resources/images/chat4.png"));          // 将资源中的 Icon 设置在窗口上

    m_LoginDialog = new LoginDialog(this);
    setCentralWidget(m_LoginDialog);                                    // 将 m_LoginDialog 嵌入 MainWindow，启动时以 m_LoginDialog 开始
    //m_LoginDialog->show();

    m_RegisterDialog = new RegisterDialog(this);

    connect(m_LoginDialog, &LoginDialog::SwitchRegister, this, &MainWindow::SlotSwitchReg); // 创建"注册"按钮的窗口事件触发（创建连接）

    m_LoginDialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    m_RegisterDialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    //m_RegisterDialog->hide();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SlotSwitchReg()
{
    setCentralWidget(m_RegisterDialog);
    m_LoginDialog->hide();
    m_RegisterDialog->show();
}
