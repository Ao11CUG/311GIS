#include"WKTInput.h"
#include <QApplication>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QWidget>
#include <QLabel>
#include <gdal.h>
#include <ogrsf_frmts.h>
#include <cpl_error.h>

//接受文件地址并渲染至QImage
QImage WKTInput::visualizeWkt(const QString& filePath) {
    GDALAllRegister();

    QImage image(800, 600, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    GDALDataset* poDS = (GDALDataset*)GDALOpenEx(filePath.toStdString().c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
    if (poDS == nullptr) {
        std::cerr << "Failed to open the file. GDAL Error: " << CPLGetLastErrorMsg() << std::endl;
        return image;
    }

    OGRLayer* poLayer = poDS->GetLayer(0);
    if (!poLayer) {
        std::cerr << "Failed to get the layer." << std::endl;
        GDALClose(poDS);
        return image;
    }

    QList<OGRGeometry*> geometries;
    OGRFeature* poFeature;
    poLayer->ResetReading();

    while ((poFeature = poLayer->GetNextFeature()) != nullptr) {
        OGRGeometry* poGeometry = poFeature->GetGeometryRef();
        if (poGeometry != nullptr) {
            geometries.push_back(poGeometry->clone());
        }
        OGRFeature::DestroyFeature(poFeature);
    }

    GDALClose(poDS);

    if (geometries.isEmpty()) {
        std::cerr << "No geometries to draw." << std::endl;
        return image;
    }

    double minX, minY, maxX, maxY;
    bool first = true;

    // 计算几何图形的总边界框
    for (const auto& geom : geometries) {
        if (geom == nullptr) {
            continue;
        }

        OGREnvelope envelope;
        geom->getEnvelope(&envelope);

        if (first) {
            minX = envelope.MinX;
            minY = envelope.MinY;
            maxX = envelope.MaxX;
            maxY = envelope.MaxY;
            first = false;
        }
        else {
            if (envelope.MinX < minX) minX = envelope.MinX;
            if (envelope.MinY < minY) minY = envelope.MinY;
            if (envelope.MaxX > maxX) maxX = envelope.MaxX;
            if (envelope.MaxY > maxY) maxY = envelope.MaxY;
        }
    }

    // 计算缩放和偏移
    double scaleX = (maxX - minX) > 0 ? image.width() / (maxX - minX) : 1.0;
    double scaleY = (maxY - minY) > 0 ? image.height() / (maxY - minY) : 1.0;
    double scale = std::min(scaleX, scaleY);
    double offsetX = -minX * scale;
    double offsetY = image.height() + minY * scale;

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制几何图形
    for (const auto& geom : geometries) {
        if (geom == nullptr) continue;

        if (geom->getGeometryType() == wkbPoint || geom->getGeometryType() == wkbPoint25D) {
            OGRPoint* point = (OGRPoint*)geom;
            painter.setPen(Qt::black);
            painter.drawEllipse(offsetX + point->getX() * scale - 2.0, offsetY - point->getY() * scale - 2.0, 4.0, 4.0);
        }
        else if (geom->getGeometryType() == wkbLineString || geom->getGeometryType() == wkbLineString25D) {
            OGRLineString* line = (OGRLineString*)geom;
            painter.setPen(Qt::black);
            painter.drawLine(offsetX + line->getX(0) * scale, offsetY - line->getY(0) * scale, offsetX + line->getX(line->getNumPoints() - 1) * scale, offsetY - line->getY(line->getNumPoints() - 1) * scale);
        }
        else if (geom->getGeometryType() == wkbPolygon || geom->getGeometryType() == wkbPolygon25D) {
            OGRPolygon* polygon = (OGRPolygon*)geom;
            drawPolygon(polygon, painter, scale, offsetX, offsetY);
        }
        else if (geom->getGeometryType() == wkbMultiPolygon) {
            OGRMultiPolygon* multiPolygon = (OGRMultiPolygon*)geom;
            for (int i = 0; i < multiPolygon->getNumGeometries(); ++i) {
                OGRPolygon* polygon = (OGRPolygon*)multiPolygon->getGeometryRef(i);
                drawPolygon(polygon, painter, scale, offsetX, offsetY);
            }
        }
    }

    return image;
}

//绘制
void WKTInput::drawPolygon(OGRPolygon* polygon, QPainter& painter, double scale, double offsetX, double offsetY) {
    QPainterPath path;
    OGRLinearRing* ring = polygon->getExteriorRing();
    if (ring == nullptr) return;

    path.moveTo(offsetX + ring->getX(0) * scale, offsetY - ring->getY(0) * scale);
    for (int i = 1; i < ring->getNumPoints(); ++i) {
        path.lineTo(offsetX + ring->getX(i) * scale, offsetY - ring->getY(i) * scale);
    }
    path.closeSubpath();
    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(QBrush(Qt::blue));
    painter.drawPath(path);
}