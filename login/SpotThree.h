#pragma once

#include <QWidget>
#include "ui_SpotThree.h"

class SpotThree : public QWidget
{
	Q_OBJECT

public:
	SpotThree(QWidget *parent = nullptr);
	~SpotThree();

private:
	Ui::SpotThreeClass ui;
};
