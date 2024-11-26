#include "SpotSeven.h"

SpotSeven::SpotSeven(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	setWindowTitle("Mixiu embroidery Display");
	setWindowIcon(QIcon(":/login/res/travel.png"));
}

SpotSeven::~SpotSeven()
{}
