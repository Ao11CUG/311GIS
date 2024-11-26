#pragma once

#include <QtWidgets/QWidget>
#include "ui_login.h"
#include <QString>
#include <Qvector>
#include "mainwindow.h"

class Login : public QWidget
{
    Q_OBJECT

public:
    Login(QWidget *parent = nullptr);
    ~Login();

    void addAccount(const QString& account, const QString& password);

private slots:
    void on_login_clicked();

private:
    bool verifyAccount(const QString& account, const QString& password);
    Ui::loginClass mUi;
    QVector<QPair<QString, QString>> mvAccounts;
};



