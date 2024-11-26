#include "Login.h"
#include <QMessageBox>

Login::Login(QWidget* parent)
    : QWidget(parent)
{
    mUi.setupUi(this);

    setWindowTitle("311GIS");
    setWindowIcon(QIcon(":/login/res/311GIS.png"));

    addAccount("20221003498", "20040602");
    addAccount("20221003040", "20031210");
    addAccount("20221001778", "20041114");
    addAccount("20221003535", "20031106");
    addAccount("1", "1");

    mUi.passwords->setEchoMode(QLineEdit::Password);
}

Login::~Login()
{
}

void Login::addAccount(const QString& account, const QString& password)
{
    mvAccounts.append(qMakePair(account, password));
}

bool Login::verifyAccount(const QString& account, const QString& password)
{
    for (const auto& acc : mvAccounts) {
        if (acc.first == account && acc.second == password) {
            return true;
        }
    }
    return false;
}

void Login::on_login_clicked()
{
    QString account = mUi.accounts->text();
    QString password = mUi.passwords->text();

    if (verifyAccount(account, password)) {
        QMessageBox::information(this, "Success", "Login successfully!");

        // 创建并显示主窗口
        MainWindow* mainWindow = new MainWindow();
        mainWindow->show();

        // 隐藏登陆窗口
        this->hide();
    }
    else {
        QMessageBox::warning(this, "Failed", "Wrong accounts or passwords!");
    }
}