#pragma once

#include <QWidget>
#include "ui_SpotFive.h"

class SpotFive : public QWidget
{
	Q_OBJECT

public:
	SpotFive(QWidget *parent = nullptr);
	~SpotFive();

private:
	Ui::SpotFiveClass ui;
};
