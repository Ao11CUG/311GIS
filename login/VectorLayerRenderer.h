#ifndef VECTORLAYERRENDERER_H
#define VECTORLAYERRENDERER_H

#include <QLabel>
#include <QColor>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QMessageBox>
#include <gdal_priv.h>
#include <ogr_api.h>

class VectorLayerRenderer : public QLabel {
    Q_OBJECT

public:
    explicit VectorLayerRenderer(const QString& filePath, QWidget* parent = nullptr);

    void setPenColor(const QColor& color);
    void setScaleFactors(double scaleX, double scaleY);

    QImage renderToImage();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QString mstrFilePath;    // ʸ���ļ�·��
    QColor mPenColor;        // ������ɫ
    double mdScaleX;         // X����������
    double mdScaleY;         // Y����������
    double mdMinX;           // ��СX����
    double mdMinY;           // ��СY����            
    double mdScaleFactor;    // ͳһ��������
    QPoint mLastMousePos;    // ��һ�����λ��
    OGREnvelope mEnv;  
    void resetTransform();
    void renderToPixmap();
    void renderGeometry(QPainter* painter, OGRGeometry* geometry);
    double transformX(double x) const;
    double transformY(double y) const;
};

#endif // VECTORLAYERRENDERER_H
