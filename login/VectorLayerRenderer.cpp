#include "VectorLayerRenderer.h"
#include <QImage>
#include <QPainter>
#include <gdal.h>
#include <gdal_priv.h>
#include <ogr_spatialref.h>
#include <ogr_api.h>
#include <ogrsf_frmts.h>

VectorLayerRenderer::VectorLayerRenderer(const QString& filePath, QWidget* parent)
    : QLabel(parent),
    mstrFilePath(filePath),
    mPenColor(QColor("#55ffff")) // ��ʼ��������ɫ
{
    // ���ô��ڱ���
    setWindowTitle("ʸ�����ݿ��ӻ�");
    // ����������
    setMouseTracking(true);

    // �����û������ʸ�����ݼ�
    GDALAllRegister();
    GDALDataset* poDS = (GDALDataset*)GDALOpenEx(mstrFilePath.toStdString().c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
    if (poDS == NULL) {
        // �����ļ�ʧ�ܣ�����������Ϣ��
        QMessageBox::critical(this, "Error", "Failed to open vector file.");
        return;
    }

    // ��ȡ��һ��
    OGRLayer* poLayer = poDS->GetLayer(0);
    // ��ȡ��ķ�Χ��������
    poLayer->GetExtent(&mEnv);

    // �����ʼ��������
    mdScaleX = width() / (mEnv.MaxX - mEnv.MinX);
    mdScaleY = height() / (mEnv.MaxY - mEnv.MinY);
    mdMinX = mEnv.MinX;
    mdMinY = mEnv.MinY;

    // �洢��ʼ�任����
    resetTransform();

    // ��Ⱦ�� QPixmap
    renderToPixmap();

    GDALClose(poDS);
}

//����任
void VectorLayerRenderer::resetTransform() {
    mLastMousePos = QPoint(0, 0);
    mdScaleFactor = 1.0;
    renderToPixmap();
}

//�����¼�
void VectorLayerRenderer::paintEvent(QPaintEvent* event) {
    QLabel::paintEvent(event);  // ���û�����ػ溯�������� QPixmap
}

//��Ⱦ��QImage
QImage VectorLayerRenderer::renderToImage() {
    QImage image(size(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.translate(mLastMousePos);
    painter.scale(mdScaleFactor, mdScaleFactor);

    GDALDataset* poDS = (GDALDataset*)GDALOpenEx(mstrFilePath.toStdString().c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
    OGRLayer* poLayer = poDS->GetLayer(0);
    poLayer->ResetReading();
    OGRFeature* poFeature;
    while ((poFeature = poLayer->GetNextFeature()) != NULL) {
        OGRGeometry* poGeometry = poFeature->GetGeometryRef();
        if (poGeometry != NULL) {
            renderGeometry(&painter, poGeometry);
        }
        OGRFeature::DestroyFeature(poFeature);
    }

    GDALClose(poDS);
    return image;
}

//��Ⱦ��Pixmap
void VectorLayerRenderer::renderToPixmap() {
    QPixmap pixmap(size());
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Ӧ�ñ任
    painter.translate(mLastMousePos);
    painter.scale(mdScaleFactor, mdScaleFactor);

    // ѭ��������������Ⱦ
    GDALDataset* poDS = (GDALDataset*)GDALOpenEx(mstrFilePath.toStdString().c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
    OGRLayer* poLayer = poDS->GetLayer(0);
    poLayer->ResetReading();
    OGRFeature* poFeature;
    while ((poFeature = poLayer->GetNextFeature()) != NULL) {
        OGRGeometry* poGeometry = poFeature->GetGeometryRef();
        if (poGeometry != NULL) {
            renderGeometry(&painter, poGeometry);
        }
        OGRFeature::DestroyFeature(poFeature);
    }

    GDALClose(poDS);

    // ����Ⱦ�������Ϊ QLabel ��ͼƬ
    setPixmap(pixmap);
}

//����
void VectorLayerRenderer::renderGeometry(QPainter* painter, OGRGeometry* geometry) {
    // ���û�����ɫΪ��ɫ�����ڵ����Ҫ��
    painter->setPen(Qt::black);

    if (wkbFlatten(geometry->getGeometryType()) == wkbPoint) {
        OGRPoint* point = (OGRPoint*)geometry;
        painter->drawPoint(transformX(point->getX()), transformY(point->getY()));
    }
    else if (wkbFlatten(geometry->getGeometryType()) == wkbLineString) {
        OGRLineString* line = (OGRLineString*)geometry;
        QPolygonF polyline;
        for (int i = 0; i < line->getNumPoints(); ++i) {
            QPointF point(transformX(line->getX(i)), transformY(line->getY(i)));
            polyline << point;
        }
        painter->drawPolyline(polyline);
    }
    else if (wkbFlatten(geometry->getGeometryType()) == wkbPolygon) {
        // ʹ�� mPenColor ���ж�������
        painter->setBrush(mPenColor);

        OGRPolygon* polygon = (OGRPolygon*)geometry;
        QPolygonF polygonF;
        for (int i = 0; i < polygon->getExteriorRing()->getNumPoints(); ++i) {
            QPointF point(transformX(polygon->getExteriorRing()->getX(i)),
                transformY(polygon->getExteriorRing()->getY(i)));
            polygonF << point;
        }
        painter->drawPolygon(polygonF);
    }
}

// ����������ɫ�ķ���
void VectorLayerRenderer::setPenColor(const QColor& color) {
    mPenColor = color;
}

//ת��X
double VectorLayerRenderer::transformX(double x) const {
    return (x - mdMinX) * mdScaleX;
}

//ת��Y
double VectorLayerRenderer::transformY(double y) const {
    return height() - (y - mdMinY) * mdScaleY;
}

//������������
void VectorLayerRenderer::setScaleFactors(double scaleX, double scaleY) {
    mdScaleX = scaleX;
    mdScaleY = scaleY;
}