#pragma once

#include <QWidget>
#include "ui_SpotSeven.h"

class SpotSeven : public QWidget
{
	Q_OBJECT

public:
	SpotSeven(QWidget *parent = nullptr);
	~SpotSeven();

private:
	Ui::SpotSevenClass ui;
};
