#pragma once

#include <QWidget>
#include "ui_SpotEight.h"

class SpotEight : public QWidget
{
	Q_OBJECT

public:
	SpotEight(QWidget *parent = nullptr);
	~SpotEight();

private:
	Ui::SpotEightClass ui;
};
