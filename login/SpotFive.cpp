#include "SpotFive.h"

SpotFive::SpotFive(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	setWindowTitle("Butterfly House");
	setWindowIcon(QIcon(":/login/res/travel.png"));
}

SpotFive::~SpotFive()
{}
