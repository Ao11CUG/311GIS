#include "SpotThree.h"

SpotThree::SpotThree(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	setWindowTitle("Qu Yuan Art Group");
	setWindowIcon(QIcon(":/login/res/travel.png"));
}

SpotThree::~SpotThree()
{}
