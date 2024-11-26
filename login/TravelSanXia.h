#pragma once

#include <QWidget>
#include "ui_TravelSanXia.h"

class TravelSanXia : public QWidget
{
	Q_OBJECT

public:
	TravelSanXia(QWidget *parent = nullptr);
	~TravelSanXia();

private:
	Ui::TravelSanXiaClass ui;
};
