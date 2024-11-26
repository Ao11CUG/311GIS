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
    QString mstrFilePath;    // 矢量文件路径
    QColor mPenColor;        // 画笔颜色
    double mdScaleX;         // X轴缩放因子
    double mdScaleY;         // Y轴缩放因子
    double mdMinX;           // 最小X坐标
    double mdMinY;           // 最小Y坐标            
    double mdScaleFactor;    // 统一缩放因子
    QPoint mLastMousePos;    // 上一次鼠标位置
    OGREnvelope mEnv;  
    void resetTransform();
    void renderToPixmap();
    void renderGeometry(QPainter* painter, OGRGeometry* geometry);
    double transformX(double x) const;
    double transformY(double y) const;
};

#endif // VECTORLAYERRENDERER_H
