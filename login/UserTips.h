#pragma once

#include <QWidget>
#include "ui_UserTips.h"
#include <QFile>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>

class UserTips : public QWidget
{
	Q_OBJECT

public:
	UserTips(QWidget *parent = nullptr);
	~UserTips();

public slots:
	void openPdf();

private:
	Ui::UserTipsClass mUi;
};
