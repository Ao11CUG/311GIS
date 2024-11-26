#ifndef GEOJSONINPUT_H
#define GEOJSONINPUT_H

#include <QLabel>
#include <QString>
#include <QPixmap>
#include <QPoint>
#include <QMouseEvent>
#include "gdal_priv.h"
#include "ogrsf_frmts.h"

class GeoJsonInput : public QLabel {
    Q_OBJECT

public:
    GeoJsonInput(const QString& filePath, QWidget* parent = nullptr);
    ~GeoJsonInput();

    void resetTransform();

    QImage renderToImage();

private:
    void renderToPixmap();
    void renderGeometry(QPainter* painter, OGRGeometry* geometry);
    double transformX(double x) const;
    double transformY(double y) const;

private:
    QString mstrFilePath;
    double mstrScaleFactor;
    QPoint mTranslation;
    double mdMinX, mdMinY, mdScaleX, mdScaleY;
    OGREnvelope mEnv;
    bool mbPanMode;
    QPoint mLastPanPos;
};

#endif // GEOJSONINPUT_H