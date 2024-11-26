#pragma once

#include <QWidget>
#include "ui_SpotFour.h"

class SpotFour : public QWidget
{
	Q_OBJECT

public:
	SpotFour(QWidget *parent = nullptr);
	~SpotFour();

private:
	Ui::SpotFourClass ui;
};
