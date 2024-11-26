#pragma once

#include <QWidget>
#include "ui_SpotTwo.h"

class SpotTwo : public QWidget
{
	Q_OBJECT

public:
	SpotTwo(QWidget *parent = nullptr);
	~SpotTwo();

private:
	Ui::SpotTwoClass ui;
};
