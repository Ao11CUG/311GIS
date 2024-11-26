#include "SpotSix.h"

SpotSix::SpotSix(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	setWindowTitle("Qu Yuan's Ancestral Arch");
	setWindowIcon(QIcon(":/login/res/travel.png"));
}

SpotSix::~SpotSix()
{}
