#include "GeoJsonInput.h"
#include <QDebug>
#include <QWheelEvent>
#include <QPainter>

GeoJsonInput::GeoJsonInput(const QString& filePath, QWidget* parent)
    : QLabel(parent), mstrFilePath(filePath), mstrScaleFactor(1.0), mTranslation(0, 0), mbPanMode(false)
{
    CPLSetErrorHandler([](CPLErr eErrClass, int err_no, const char* msg) {
        qDebug() << "GDAL Error:" << CPLGetLastErrorMsg();
        });

    GDALAllRegister();
}

GeoJsonInput::~GeoJsonInput()
{
}

void GeoJsonInput::resetTransform()
{
    mstrScaleFactor = 1.0;
    mTranslation = QPoint(0, 0);
    update();
}


void GeoJsonInput::renderToPixmap()
{
    QPixmap pixmap(size());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.translate(mTranslation);
    painter.scale(mstrScaleFactor, mstrScaleFactor);

    GDALDataset* poDS = (GDALDataset*)GDALOpenEx(mstrFilePath.toStdString().c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
    if (poDS == nullptr) {
        qDebug() << "Failed to open vector file:" << mstrFilePath;
        return;
    }

    OGRLayer* poLayer = poDS->GetLayer(0);
    if (poLayer == nullptr) {
        qDebug() << "Failed to get layer from vector file:" << mstrFilePath;
        GDALClose(poDS);
        return;
    }

    poLayer->GetExtent(&mEnv);
    mdMinX = mEnv.MinX;
    mdMinY = mEnv.MinY;
    mdScaleX = width() / (mEnv.MaxX - mEnv.MinX);
    mdScaleY = height() / (mEnv.MaxY - mEnv.MinY);

    OGRFeature* poFeature;
    poLayer->ResetReading();
    while ((poFeature = poLayer->GetNextFeature()) != NULL) {
        OGRGeometry* poGeometry = poFeature->GetGeometryRef();
        if (poGeometry != NULL) {
            if (wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon ||
                wkbFlatten(poGeometry->getGeometryType()) == wkbMultiPolygon) {
                renderGeometry(&painter, poGeometry);
            }
        }
        OGRFeature::DestroyFeature(poFeature);
    }

    GDALClose(poDS);
    setPixmap(pixmap);
}

void GeoJsonInput::renderGeometry(QPainter* painter, OGRGeometry* geometry)
{
    if (wkbFlatten(geometry->getGeometryType()) == wkbPolygon) {
        OGRPolygon* polygon = (OGRPolygon*)geometry;
        painter->setPen(Qt::black);
        painter->setBrush(Qt::yellow);

        QPolygonF polygonF;
        OGRLinearRing* exteriorRing = polygon->getExteriorRing();
        for (int i = 0; i < exteriorRing->getNumPoints(); ++i) {
            QPointF point(transformX(exteriorRing->getX(i)),
                transformY(exteriorRing->getY(i)));
            polygonF << point;
        }
        painter->drawPolygon(polygonF);
    }
    else if (wkbFlatten(geometry->getGeometryType()) == wkbMultiPolygon) {
        OGRMultiPolygon* multiPolygon = (OGRMultiPolygon*)geometry;
        painter->setPen(Qt::black);
        painter->setBrush(Qt::yellow);

        for (int i = 0; i < multiPolygon->getNumGeometries(); ++i) {
            OGRPolygon* polygon = (OGRPolygon*)multiPolygon->getGeometryRef(i);
            QPolygonF polygonF;
            OGRLinearRing* exteriorRing = polygon->getExteriorRing();
            for (int j = 0; j < exteriorRing->getNumPoints(); ++j) {
                QPointF point(transformX(exteriorRing->getX(j)),
                    transformY(exteriorRing->getY(j)));
                polygonF << point;
            }
            painter->drawPolygon(polygonF);
        }
    }
}

double GeoJsonInput::transformX(double x) const
{
    return (x - mdMinX) * mdScaleX;
}

double GeoJsonInput::transformY(double y) const
{
    return height() - (y - mdMinY) * mdScaleY;
}

QImage GeoJsonInput::renderToImage() {
    QImage image(size(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.translate(mTranslation);
    painter.scale(mstrScaleFactor, mstrScaleFactor);

    GDALDataset* poDS = (GDALDataset*)GDALOpenEx(mstrFilePath.toStdString().c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
    if (poDS == nullptr) {
        qDebug() << "Failed to open vector file:" << mstrFilePath;
        return image;  // Return the blank image
    }

    OGRLayer* poLayer = poDS->GetLayer(0);
    if (poLayer == nullptr) {
        qDebug() << "Failed to get layer from vector file:" << mstrFilePath;
        GDALClose(poDS);
        return image;  // Return the blank image
    }

    poLayer->GetExtent(&mEnv);
    mdMinX = mEnv.MinX;
    mdMinY = mEnv.MinY;
    mdScaleX = width() / (mEnv.MaxX - mEnv.MinX);
    mdScaleY = height() / (mEnv.MaxY - mEnv.MinY);

    OGRFeature* poFeature;
    poLayer->ResetReading();
    while ((poFeature = poLayer->GetNextFeature()) != NULL) {
        OGRGeometry* poGeometry = poFeature->GetGeometryRef();
        if (poGeometry != NULL) {
            if (wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon ||
                wkbFlatten(poGeometry->getGeometryType()) == wkbMultiPolygon) {
                renderGeometry(&painter, poGeometry);
            }
        }
        OGRFeature::DestroyFeature(poFeature);
    }

    GDALClose(poDS);
    return image;
}
