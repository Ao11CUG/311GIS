#include "SpotFour.h"

SpotFour::SpotFour(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	setWindowTitle("Qu Yuan's Temple");
	setWindowIcon(QIcon(":/login/res/travel.png"));
}

SpotFour::~SpotFour()
{}
