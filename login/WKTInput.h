#include <QApplication>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QWidget>
#include <QLabel>
#include <iostream>
#include <gdal.h>
#include <ogrsf_frmts.h>
#include <cpl_error.h>

class WKTInput {
public:
    QImage visualizeWkt(const QString& filePath);

    void drawPolygon(OGRPolygon* polygon, QPainter& painter, double scale, double offsetX, double offsetY);
};