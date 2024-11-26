#ifndef LAYEREDCANVAS_H
#define LAYEREDCANVAS_H

#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QWheelEvent>
#include <QVector>
#include <QGraphicsScene>
#include "gdal_priv.h"
#include "ogrsf_frmts.h"

struct Layer {
    QImage image;
    QList<QGraphicsItem*> graphicsItems; // 存储与图像关联的绘制图形
    QList<QPointF> graphicsOriginalPositions; // 存储与图形项关联的原始位置
    QPointF position; // 图像的当前位置信息
    QPointF originalPosition; // 添加原始位置
};

class LayeredCanvas : public QGraphicsView
{
    Q_OBJECT

public:
    explicit LayeredCanvas(QWidget* parent = nullptr);

    // 添加图层
    void addLayer(const QImage& layer);

    // 删除图层
    void removeLayer(int index);

    // 设置图层可见性
    void setLayerVisible(int index, bool visible);

    //图层复位
    void restoreAllPositions(int layerIndex);

    //清空图形
    void clearLayerGraphics(int layerIndex);

    //清除上一个绘制的图形
    void clearLastGraphic(int layerIndex);

    enum DrawMode {
        None,
        Move,
        DrawPolyline,
        DrawEllipse,
        DrawRectangle,
    };

    void setPenColor(const QColor& color); // 添加设置画笔颜色的方法
    void setPenWidth(int width);
    DrawMode mDrawMode; // 当前绘制模式
    void setDrawMode(DrawMode mode) { mDrawMode = mode; }
    DrawMode getDrawMode() { return mDrawMode; }
   
private:
    // 更新图层可见性
    void updateLayerVisibility();
    QGraphicsScene* mpScene; // 指向图形场景的指针
    QVector<QGraphicsPixmapItem*> mvLayerItems; // 存储图层项
    QVector<bool> mvLayerVisibility; // 图层可见性
    double mdScaleFactor; // 缩放因子
    QPointF mPanOffset; // 平移偏移
    bool mbIsDragging; // 是否正在拖动
    QPointF mLastMousePos; // 上一个鼠标位置
    QPointF mStartPoint; // 鼠标按下时的位置
    QGraphicsItem* mCurrentItem; // 当前正在绘制的项
    QList<QPointF> mvPoints;
    QGraphicsPixmapItem* mSelectedItem;
    QColor mPenColor = Qt::black; // 存储画笔颜色
    int mnPenwidth = 1;
    QList<Layer> mvLayers; // 存储所有图层
    QList<QGraphicsEllipseItem*> mvPointItems;
    QGraphicsLineItem* mCurrentLine = nullptr; // 当前绘制的临时线段

protected:
    // 鼠标滚轮事件
    void wheelEvent(QWheelEvent* event) override;

    // 鼠标按下事件
    void mousePressEvent(QMouseEvent* event) override;

    // 鼠标移动事件
    void mouseMoveEvent(QMouseEvent* event) override;

    // 鼠标释放事件
    void mouseReleaseEvent(QMouseEvent* event) override;
};

#endif // LAYEREDCANVAS_H
