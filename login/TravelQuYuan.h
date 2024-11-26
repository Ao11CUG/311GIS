#pragma once

#include "ui_TravelQuYuan.h"
#include <QWidget>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollArea>
#include "SpotOne.h"
#include "SpotTwo.h"
#include "SpotThree.h"
#include "SpotFour.h"
#include "SpotFive.h"
#include "SpotSix.h"
#include "SpotSeven.h"
#include "SpotEight.h"
#include "SpotNine.h"

class TravelQuYuan : public QWidget
{
	Q_OBJECT

public:
	TravelQuYuan(QWidget *parent = nullptr);
	~TravelQuYuan();

public slots:
	void showSpotOne();
	void showSpotTwo();
	void showSpotThree();
	void showSpotFour();
	void showSpotFive();
	void showSpotSix();
	void showSpotSeven();
	void showSpotEight();
	void showSpotNine();

private:
	Ui::TravelQuYuanClass ui;
};
