#include "SpotTwo.h"

SpotTwo::SpotTwo(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	setWindowTitle("Qu Yuan Culture Museum");
	setWindowIcon(QIcon(":/login/res/travel.png"));
}

SpotTwo::~SpotTwo()
{}
