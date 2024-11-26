#pragma once

#include <QWidget>
#include "ui_SpotOne.h"

class SpotOne : public QWidget
{
	Q_OBJECT

public:
	SpotOne(QWidget *parent = nullptr);
	~SpotOne();

private:
	Ui::SpotOneClass ui;
};
