#include "TravelQuYuan.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <qscrollbar.h>

TravelQuYuan::TravelQuYuan(QWidget* parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    setWindowTitle("Qu Yuan's Hometown");
    setWindowIcon(QIcon(":/login/res/travel.png"));

    connect(ui.one, &QPushButton::clicked, this, &TravelQuYuan::showSpotOne);
    connect(ui.two, &QPushButton::clicked, this, &TravelQuYuan::showSpotTwo);
    connect(ui.three, &QPushButton::clicked, this, &TravelQuYuan::showSpotThree);
    connect(ui.four, &QPushButton::clicked, this, &TravelQuYuan::showSpotFour);
    connect(ui.five, &QPushButton::clicked, this, &TravelQuYuan::showSpotFive);
    connect(ui.six, &QPushButton::clicked, this, &TravelQuYuan::showSpotSix);
    connect(ui.seven, &QPushButton::clicked, this, &TravelQuYuan::showSpotSeven);
    connect(ui.eight, &QPushButton::clicked, this, &TravelQuYuan::showSpotEight);
    connect(ui.nine, &QPushButton::clicked, this, &TravelQuYuan::showSpotNine);
}

TravelQuYuan::~TravelQuYuan()
{}

void TravelQuYuan::showSpotOne() {
    SpotOne* spotOne = new SpotOne();
    spotOne->show();
}

void TravelQuYuan::showSpotTwo() {
    SpotTwo* spotTwo = new SpotTwo();
    spotTwo->show();
}

void TravelQuYuan::showSpotThree() {
    SpotThree* spotThree = new SpotThree();
    spotThree->show();
}

void TravelQuYuan::showSpotFour() {
    SpotFour* spotFour = new SpotFour();
    spotFour->show();
}

void TravelQuYuan::showSpotFive() {
    SpotFive* spotFive = new SpotFive();
    spotFive->show();
}

void TravelQuYuan::showSpotSix() {
    SpotSix* spotSix = new SpotSix();
    spotSix->show();
}

void TravelQuYuan::showSpotSeven() {
    SpotSeven* spotSeven = new SpotSeven();
    spotSeven->show();
}

void TravelQuYuan::showSpotEight() {
    SpotEight* spotEight = new SpotEight();
    spotEight->show();
}

void TravelQuYuan::showSpotNine() {
    SpotNine* spotNine = new SpotNine();
    spotNine->show();
}

