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
    QList<QGraphicsItem*> graphicsItems; // �洢��ͼ������Ļ���ͼ��
    QList<QPointF> graphicsOriginalPositions; // �洢��ͼ���������ԭʼλ��
    QPointF position; // ͼ��ĵ�ǰλ����Ϣ
    QPointF originalPosition; // ���ԭʼλ��
};

class LayeredCanvas : public QGraphicsView
{
    Q_OBJECT

public:
    explicit LayeredCanvas(QWidget* parent = nullptr);

    // ���ͼ��
    void addLayer(const QImage& layer);

    // ɾ��ͼ��
    void removeLayer(int index);

    // ����ͼ��ɼ���
    void setLayerVisible(int index, bool visible);

    //ͼ�㸴λ
    void restoreAllPositions(int layerIndex);

    //���ͼ��
    void clearLayerGraphics(int layerIndex);

    //�����һ�����Ƶ�ͼ��
    void clearLastGraphic(int layerIndex);

    enum DrawMode {
        None,
        Move,
        DrawPolyline,
        DrawEllipse,
        DrawRectangle,
    };

    void setPenColor(const QColor& color); // ������û�����ɫ�ķ���
    void setPenWidth(int width);
    DrawMode mDrawMode; // ��ǰ����ģʽ
    void setDrawMode(DrawMode mode) { mDrawMode = mode; }
    DrawMode getDrawMode() { return mDrawMode; }
   
private:
    // ����ͼ��ɼ���
    void updateLayerVisibility();
    QGraphicsScene* mpScene; // ָ��ͼ�γ�����ָ��
    QVector<QGraphicsPixmapItem*> mvLayerItems; // �洢ͼ����
    QVector<bool> mvLayerVisibility; // ͼ��ɼ���
    double mdScaleFactor; // ��������
    QPointF mPanOffset; // ƽ��ƫ��
    bool mbIsDragging; // �Ƿ������϶�
    QPointF mLastMousePos; // ��һ�����λ��
    QPointF mStartPoint; // ��갴��ʱ��λ��
    QGraphicsItem* mCurrentItem; // ��ǰ���ڻ��Ƶ���
    QList<QPointF> mvPoints;
    QGraphicsPixmapItem* mSelectedItem;
    QColor mPenColor = Qt::black; // �洢������ɫ
    int mnPenwidth = 1;
    QList<Layer> mvLayers; // �洢����ͼ��
    QList<QGraphicsEllipseItem*> mvPointItems;
    QGraphicsLineItem* mCurrentLine = nullptr; // ��ǰ���Ƶ���ʱ�߶�

protected:
    // �������¼�
    void wheelEvent(QWheelEvent* event) override;

    // ��갴���¼�
    void mousePressEvent(QMouseEvent* event) override;

    // ����ƶ��¼�
    void mouseMoveEvent(QMouseEvent* event) override;

    // ����ͷ��¼�
    void mouseReleaseEvent(QMouseEvent* event) override;
};

#endif // LAYEREDCANVAS_H
