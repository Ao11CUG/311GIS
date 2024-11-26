#pragma once

#include <QWidget>
#include "ui_SpotSix.h"

class SpotSix : public QWidget
{
	Q_OBJECT

public:
	SpotSix(QWidget *parent = nullptr);
	~SpotSix();

private:
	Ui::SpotSixClass ui;
};
