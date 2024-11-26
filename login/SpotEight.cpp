#include "SpotEight.h"

SpotEight::SpotEight(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	setWindowTitle("Qingtan Folk Museum");
	setWindowIcon(QIcon(":/login/res/travel.png"));
}

SpotEight::~SpotEight()
{}
