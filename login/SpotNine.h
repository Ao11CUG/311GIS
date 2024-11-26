#pragma once

#include <QWidget>
#include "ui_SpotNine.h"

class SpotNine : public QWidget
{
	Q_OBJECT

public:
	SpotNine(QWidget *parent = nullptr);
	~SpotNine();

private:
	Ui::SpotNineClass ui;
};
