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
    mPenColor(QColor("#55ffff")) // 初始化画笔颜色
{
    // 设置窗口标题
    setWindowTitle("矢量数据可视化");
    // 启用鼠标跟踪
    setMouseTracking(true);

    // 加载用户输入的矢量数据集
    GDALAllRegister();
    GDALDataset* poDS = (GDALDataset*)GDALOpenEx(mstrFilePath.toStdString().c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
    if (poDS == NULL) {
        // 若打开文件失败，弹出错误消息框
        QMessageBox::critical(this, "Error", "Failed to open vector file.");
        return;
    }

    // 获取第一层
    OGRLayer* poLayer = poDS->GetLayer(0);
    // 获取层的范围用于缩放
    poLayer->GetExtent(&mEnv);

    // 计算初始缩放因子
    mdScaleX = width() / (mEnv.MaxX - mEnv.MinX);
    mdScaleY = height() / (mEnv.MaxY - mEnv.MinY);
    mdMinX = mEnv.MinX;
    mdMinY = mEnv.MinY;

    // 存储初始变换矩阵
    resetTransform();

    // 渲染到 QPixmap
    renderToPixmap();

    GDALClose(poDS);
}

//矩阵变换
void VectorLayerRenderer::resetTransform() {
    mLastMousePos = QPoint(0, 0);
    mdScaleFactor = 1.0;
    renderToPixmap();
}

//绘制事件
void VectorLayerRenderer::paintEvent(QPaintEvent* event) {
    QLabel::paintEvent(event);  // 调用基类的重绘函数来绘制 QPixmap
}

//渲染至QImage
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

//渲染至Pixmap
void VectorLayerRenderer::renderToPixmap() {
    QPixmap pixmap(size());
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // 应用变换
    painter.translate(mLastMousePos);
    painter.scale(mdScaleFactor, mdScaleFactor);

    // 循环处理特征并渲染
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

    // 将渲染结果设置为 QLabel 的图片
    setPixmap(pixmap);
}

//绘制
void VectorLayerRenderer::renderGeometry(QPainter* painter, OGRGeometry* geometry) {
    // 设置画笔颜色为黑色，用于点和线要素
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
        // 使用 mPenColor 进行多边形填充
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

// 新增设置颜色的方法
void VectorLayerRenderer::setPenColor(const QColor& color) {
    mPenColor = color;
}

//转换X
double VectorLayerRenderer::transformX(double x) const {
    return (x - mdMinX) * mdScaleX;
}

//转换Y
double VectorLayerRenderer::transformY(double y) const {
    return height() - (y - mdMinY) * mdScaleY;
}

//设置缩放因子
void VectorLayerRenderer::setScaleFactors(double scaleX, double scaleY) {
    mdScaleX = scaleX;
    mdScaleY = scaleY;
}