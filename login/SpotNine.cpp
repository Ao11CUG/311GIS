#include "SpotNine.h"

SpotNine::SpotNine(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	setWindowTitle("Yinghe Gate");
	setWindowIcon(QIcon(":/login/res/travel.png"));
}

SpotNine::~SpotNine()
{}
