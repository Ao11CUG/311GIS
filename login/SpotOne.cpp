#include "SpotOne.h"

SpotOne::SpotOne(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	setWindowTitle("JingXian Gate");
	setWindowIcon(QIcon(":/login/res/travel.png"));
}

SpotOne::~SpotOne()
{}
