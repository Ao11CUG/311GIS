#include "LayeredCanvas.h"
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QMouseEvent>
#include <QMessageBox>
#include <QFileInfo>
#include <QTimer>

LayeredCanvas::LayeredCanvas(QWidget* parent)
    : QGraphicsView(parent), mdScaleFactor(1.0), mPanOffset(0, 0), mbIsDragging(false), mDrawMode(None), mCurrentItem(nullptr), mSelectedItem(nullptr) {
    // ���� QGraphicsScene
    mpScene = new QGraphicsScene(this);
    setScene(mpScene);
    setRenderHint(QPainter::Antialiasing);
}

void LayeredCanvas::addLayer(const QImage& layer) {
    Layer newLayer;
    newLayer.image = layer;
    newLayer.position = QPointF(0, 0); // ��ʼλ��
    newLayer.originalPosition = newLayer.position; // �洢ԭʼλ��
    mvLayers.append(newLayer);

    // ���ͼ�㵽����
    QGraphicsPixmapItem* item = mpScene->addPixmap(QPixmap::fromImage(layer));
    item->setPos(newLayer.position); // ���ó�ʼλ��
    mvLayerItems.append(item);
    mvLayerVisibility.append(true);
    updateLayerVisibility();

    // ��ͼ����Ӧ���ӿ�
    fitInView(item, Qt::KeepAspectRatio);

    // �ӳٽ���ͼ�ƶ�����ͼ��λ��
    QTimer::singleShot(0, this, [this, item]() {
        centerOn(item);
        });
}

// ɾ��ͼ��
void LayeredCanvas::removeLayer(int index) {
    if (index >= 0 && index < mvLayerItems.size()) {
        // ɾ�����ͼ������Ļ���ͼ����
        for (auto& graphicsItem : mvLayers[index].graphicsItems) {
            mpScene->removeItem(graphicsItem); // �ӳ������Ƴ�
            delete graphicsItem; // �ͷ��ڴ�
        }

        // ��ո�ͼ���ͼ�����б�
        mvLayers[index].graphicsItems.clear();
        mvLayers[index].graphicsOriginalPositions.clear(); // ���ԭʼλ���б�

        // ɾ��ͼ��ͼ��
        mpScene->removeItem(mvLayerItems[index]);
        delete mvLayerItems[index]; // �ͷ��ڴ�
        mvLayerItems.removeAt(index);
        mvLayerVisibility.removeAt(index);
        mvLayers.removeAt(index); // ɾ��ͼ������
        updateLayerVisibility(); // ����ͼ��ɼ���
    }
}

// ����ͼ��ɼ���
void LayeredCanvas::setLayerVisible(int index, bool visible) {
    if (index >= 0 && index < mvLayerVisibility.size()) {
        mvLayerVisibility[index] = visible;
        updateLayerVisibility();
    }
}

// ����ͼ��ɼ���
void LayeredCanvas::updateLayerVisibility() {
    for (int i = 0; i < mvLayerItems.size(); ++i) {
        mvLayerItems[i]->setVisible(mvLayerVisibility[i]);
    }
}

//��λͼ��
void LayeredCanvas::restoreAllPositions(int layerIndex) {
    if (layerIndex >= 0 && layerIndex < mvLayers.size()) {
        // �ָ�ͼ��λ��
        mvLayers[layerIndex].position = mvLayers[layerIndex].originalPosition;
        mvLayerItems[layerIndex]->setPos(mvLayers[layerIndex].position); // �ָ�ͼ��λ��

        // �ָ�ͼ����λ��
        const auto& graphicsItems = mvLayers[layerIndex].graphicsItems;
        const auto& originalPositions = mvLayers[layerIndex].graphicsOriginalPositions;

        for (int i = 0; i < graphicsItems.size(); ++i) {
            if (i < originalPositions.size()) {
                // �ָ�ͼ�����λ�ã�ʹ��ԭʼλ�ú͵�ǰͼ��λ�õĲ�ֵ
                QPointF adjustedPosition = originalPositions[i]; // ʹ�ü�¼��ʵ��λ��
                graphicsItems[i]->setPos(adjustedPosition); // �ָ�ͼ�����λ��
            }
        }
        update(); // ������ͼ�Է�ӳλ�ñ仯
    }
}

//��ջ��Ƶ�ͼ��
void LayeredCanvas::clearLayerGraphics(int layerIndex) {
    if (layerIndex >= 0 && layerIndex < mvLayers.size()) {
        // �Ƴ�����ͼ����
        for (auto& graphicsItem : mvLayers[layerIndex].graphicsItems) {
            mpScene->removeItem(graphicsItem); // �ӳ������Ƴ�
            delete graphicsItem; // �ͷ��ڴ�
        }

        // ���ͼ�����б�
        mvLayers[layerIndex].graphicsItems.clear();
        mvLayers[layerIndex].graphicsOriginalPositions.clear(); // ���ԭʼλ��

        update(); // ������ͼ�Է�ӳ�仯
    }
}

//������һ�����Ƶ�ͼ��
void LayeredCanvas::clearLastGraphic(int layerIndex) {
    // ���ͼ�������Ƿ���Ч
    if (layerIndex >= 0 && layerIndex < mvLayers.size()) {
        // ��ȡָ��ͼ���ͼ�����б�
        auto& graphicsItems = mvLayers[layerIndex].graphicsItems;

        // ���ͼ�����Ƿ��л�ͼ��
        if (!graphicsItems.isEmpty()) {
            // ��ȡ���һ��ͼ����
            QGraphicsItem* lastItem = graphicsItems.last();

            // �ӳ������Ƴ����һ��ͼ����
            mpScene->removeItem(lastItem);

            // �ͷ��ڴ�
            delete lastItem;

            // ���б����Ƴ�
            graphicsItems.removeLast();

            // Ҳ�� `graphicsOriginalPositions` �б����Ƴ���Ӧ��λ��
            if (!mvLayers[layerIndex].graphicsOriginalPositions.isEmpty()) {
                mvLayers[layerIndex].graphicsOriginalPositions.removeLast();
            }

            // ������ͼ�Է�ӳ�仯
            update();
        }
    }
}

void LayeredCanvas::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        QPointF scenePos = mapToScene(event->pos());

        QGraphicsItem* item = mpScene->itemAt(scenePos, QTransform());

        if (item && item->type() == QGraphicsPixmapItem::Type) {
            mSelectedItem = dynamic_cast<QGraphicsPixmapItem*>(item);
            mbIsDragging = true;
            mLastMousePos = event->pos();
        }
        else {
            mSelectedItem = nullptr;
        }

        // ��ȡͼ������
        int layerIndex = mvLayerItems.indexOf(mSelectedItem);
        if (layerIndex == -1) {
            return; // û��ѡ����Ч��ͼ��
        }

        // ������ʼ��
        if (mDrawMode == DrawPolyline) {
            mStartPoint = scenePos; // ��¼��ʼ��
            mCurrentLine = mpScene->addLine(QLineF(mStartPoint, mStartPoint), QPen(mPenColor, mnPenwidth)); // ʹ���趨�Ļ��ʴ�ϸ
            mvLayers[layerIndex].graphicsItems.append(mCurrentLine);
        }
        else if (mDrawMode == DrawEllipse) {
            mStartPoint = scenePos;
            mCurrentItem = mpScene->addEllipse(QRectF(mStartPoint.x(), mStartPoint.y(), 0, 0), QPen(mPenColor, mnPenwidth), QBrush(Qt::transparent));
            mvLayers[layerIndex].graphicsItems.append(mCurrentItem);
            mvLayers[layerIndex].graphicsOriginalPositions.append(mStartPoint); // ��¼ʵ��λ��
        }
        else if (mDrawMode == DrawRectangle) {
            mStartPoint = scenePos;
            mCurrentItem = mpScene->addRect(QRectF(mStartPoint.x(), mStartPoint.y(), 0, 0), QPen(mPenColor, mnPenwidth), QBrush(Qt::transparent));
            mvLayers[layerIndex].graphicsItems.append(mCurrentItem);
            mvLayers[layerIndex].graphicsOriginalPositions.append(mStartPoint); // ��¼ʵ��λ��
        }
    }
}

void LayeredCanvas::mouseMoveEvent(QMouseEvent* event) {
    QPointF scenePos = mapToScene(event->pos()); // ��ȡ��������

    if (mbIsDragging && mSelectedItem) {
        // ���� Move ģʽ��ִ���ƶ�����
        if (mDrawMode == Move) {
            int layerIndex = mvLayerItems.indexOf(mSelectedItem);
            if (layerIndex != -1) {
                QPointF delta = (event->pos() - mLastMousePos) * 2; // �����ƶ�������
                Layer& layer = mvLayers[layerIndex];

                // �ƶ�ͼ���ڵ�����ͼ����
                for (auto& graphicsItem : layer.graphicsItems) {
                    graphicsItem->moveBy(delta.x() / mdScaleFactor, delta.y() / mdScaleFactor);
                }

                // �ƶ�ѡ�е���
                mSelectedItem->moveBy(delta.x() / mdScaleFactor, delta.y() / mdScaleFactor);
                layer.position += delta / mdScaleFactor; // ����ͼ��λ��

                mLastMousePos = event->pos(); // �����������λ��
            }
        }
        else if (mDrawMode == DrawPolyline && mCurrentLine) {
            // �����߶εĽ�����
            mCurrentLine->setLine(QLineF(mStartPoint, scenePos)); // ֱ�������߶�
        }
        else if (mDrawMode == DrawEllipse && mCurrentItem) {
            QPointF currentPoint = mapToScene(event->pos());
            dynamic_cast<QGraphicsEllipseItem*>(mCurrentItem)->setRect(QRectF(mStartPoint, currentPoint).normalized());
        }
        else if (mDrawMode == DrawRectangle && mCurrentItem) {
            QPointF currentPoint = mapToScene(event->pos());
            dynamic_cast<QGraphicsRectItem*>(mCurrentItem)->setRect(QRectF(mStartPoint, currentPoint).normalized());
        }
        else if (mDrawMode == None && mbIsDragging) {
            // �϶��������߼�
            QPointF delta = (event->pos() - mLastMousePos) / mdScaleFactor; // �������ű�������delta
            mPanOffset -= delta; // ʹ�ü�����ת�ƶ�����
            mLastMousePos = event->pos(); // �����������λ��
            mpScene->setSceneRect(mpScene->sceneRect().translated(-delta)); // ͬ����תdelta
        }
    }
}

void LayeredCanvas::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && mbIsDragging) {
        if (mDrawMode == DrawPolyline && mCurrentLine) {
            // ȷ���߶εĽ����㲢��ӵ�ͼ��
            QPointF endPoint = mapToScene(event->pos());
            mCurrentLine->setLine(QLineF(mStartPoint, endPoint)); // �����߶εĽ�����
            int layerIndex = mvLayerItems.indexOf(mSelectedItem);
            mvLayers[layerIndex].graphicsOriginalPositions.append(mStartPoint);
            mvLayers[layerIndex].graphicsOriginalPositions.append(endPoint);
        }

        // �����϶�״̬
        mbIsDragging = false;
        if (mCurrentLine) {
            // �߶δ�����ɺ������ʱ�߶�
            mCurrentLine = nullptr;
        }
    }
}

void LayeredCanvas::wheelEvent(QWheelEvent* event) {
    const double zoomFactor = 1.15;
    if (event->angleDelta().y() > 0) {
        mdScaleFactor *= zoomFactor;
    }
    else {
        mdScaleFactor /= zoomFactor;
    }
    setTransform(QTransform::fromScale(mdScaleFactor, mdScaleFactor)); // Ӧ������
}

void LayeredCanvas::setPenColor(const QColor& color) {
    mPenColor = color; // ���»�����ɫ
}

void LayeredCanvas::setPenWidth(int width) {
    mnPenwidth = width;
}



