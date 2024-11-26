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
    // 创建 QGraphicsScene
    mpScene = new QGraphicsScene(this);
    setScene(mpScene);
    setRenderHint(QPainter::Antialiasing);
}

void LayeredCanvas::addLayer(const QImage& layer) {
    Layer newLayer;
    newLayer.image = layer;
    newLayer.position = QPointF(0, 0); // 初始位置
    newLayer.originalPosition = newLayer.position; // 存储原始位置
    mvLayers.append(newLayer);

    // 添加图层到场景
    QGraphicsPixmapItem* item = mpScene->addPixmap(QPixmap::fromImage(layer));
    item->setPos(newLayer.position); // 设置初始位置
    mvLayerItems.append(item);
    mvLayerVisibility.append(true);
    updateLayerVisibility();

    // 将图层适应到视口
    fitInView(item, Qt::KeepAspectRatio);

    // 延迟将视图移动到新图层位置
    QTimer::singleShot(0, this, [this, item]() {
        centerOn(item);
        });
}

// 删除图层
void LayeredCanvas::removeLayer(int index) {
    if (index >= 0 && index < mvLayerItems.size()) {
        // 删除与该图层关联的绘制图形项
        for (auto& graphicsItem : mvLayers[index].graphicsItems) {
            mpScene->removeItem(graphicsItem); // 从场景中移除
            delete graphicsItem; // 释放内存
        }

        // 清空该图层的图形项列表
        mvLayers[index].graphicsItems.clear();
        mvLayers[index].graphicsOriginalPositions.clear(); // 清空原始位置列表

        // 删除图像图层
        mpScene->removeItem(mvLayerItems[index]);
        delete mvLayerItems[index]; // 释放内存
        mvLayerItems.removeAt(index);
        mvLayerVisibility.removeAt(index);
        mvLayers.removeAt(index); // 删除图层数据
        updateLayerVisibility(); // 更新图层可见性
    }
}

// 设置图层可见性
void LayeredCanvas::setLayerVisible(int index, bool visible) {
    if (index >= 0 && index < mvLayerVisibility.size()) {
        mvLayerVisibility[index] = visible;
        updateLayerVisibility();
    }
}

// 更新图层可见性
void LayeredCanvas::updateLayerVisibility() {
    for (int i = 0; i < mvLayerItems.size(); ++i) {
        mvLayerItems[i]->setVisible(mvLayerVisibility[i]);
    }
}

//复位图层
void LayeredCanvas::restoreAllPositions(int layerIndex) {
    if (layerIndex >= 0 && layerIndex < mvLayers.size()) {
        // 恢复图层位置
        mvLayers[layerIndex].position = mvLayers[layerIndex].originalPosition;
        mvLayerItems[layerIndex]->setPos(mvLayers[layerIndex].position); // 恢复图层位置

        // 恢复图形项位置
        const auto& graphicsItems = mvLayers[layerIndex].graphicsItems;
        const auto& originalPositions = mvLayers[layerIndex].graphicsOriginalPositions;

        for (int i = 0; i < graphicsItems.size(); ++i) {
            if (i < originalPositions.size()) {
                // 恢复图形项的位置，使用原始位置和当前图层位置的差值
                QPointF adjustedPosition = originalPositions[i]; // 使用记录的实际位置
                graphicsItems[i]->setPos(adjustedPosition); // 恢复图形项的位置
            }
        }
        update(); // 更新视图以反映位置变化
    }
}

//清空绘制的图形
void LayeredCanvas::clearLayerGraphics(int layerIndex) {
    if (layerIndex >= 0 && layerIndex < mvLayers.size()) {
        // 移除所有图形项
        for (auto& graphicsItem : mvLayers[layerIndex].graphicsItems) {
            mpScene->removeItem(graphicsItem); // 从场景中移除
            delete graphicsItem; // 释放内存
        }

        // 清空图形项列表
        mvLayers[layerIndex].graphicsItems.clear();
        mvLayers[layerIndex].graphicsOriginalPositions.clear(); // 清空原始位置

        update(); // 更新视图以反映变化
    }
}

//清除最后一个绘制的图形
void LayeredCanvas::clearLastGraphic(int layerIndex) {
    // 检查图层索引是否有效
    if (layerIndex >= 0 && layerIndex < mvLayers.size()) {
        // 获取指定图层的图形项列表
        auto& graphicsItems = mvLayers[layerIndex].graphicsItems;

        // 检查图层中是否有绘图项
        if (!graphicsItems.isEmpty()) {
            // 获取最后一个图形项
            QGraphicsItem* lastItem = graphicsItems.last();

            // 从场景中移除最后一个图形项
            mpScene->removeItem(lastItem);

            // 释放内存
            delete lastItem;

            // 从列表中移除
            graphicsItems.removeLast();

            // 也从 `graphicsOriginalPositions` 列表中移除对应的位置
            if (!mvLayers[layerIndex].graphicsOriginalPositions.isEmpty()) {
                mvLayers[layerIndex].graphicsOriginalPositions.removeLast();
            }

            // 更新视图以反映变化
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

        // 获取图层索引
        int layerIndex = mvLayerItems.indexOf(mSelectedItem);
        if (layerIndex == -1) {
            return; // 没有选择有效的图层
        }

        // 设置起始点
        if (mDrawMode == DrawPolyline) {
            mStartPoint = scenePos; // 记录起始点
            mCurrentLine = mpScene->addLine(QLineF(mStartPoint, mStartPoint), QPen(mPenColor, mnPenwidth)); // 使用设定的画笔粗细
            mvLayers[layerIndex].graphicsItems.append(mCurrentLine);
        }
        else if (mDrawMode == DrawEllipse) {
            mStartPoint = scenePos;
            mCurrentItem = mpScene->addEllipse(QRectF(mStartPoint.x(), mStartPoint.y(), 0, 0), QPen(mPenColor, mnPenwidth), QBrush(Qt::transparent));
            mvLayers[layerIndex].graphicsItems.append(mCurrentItem);
            mvLayers[layerIndex].graphicsOriginalPositions.append(mStartPoint); // 记录实际位置
        }
        else if (mDrawMode == DrawRectangle) {
            mStartPoint = scenePos;
            mCurrentItem = mpScene->addRect(QRectF(mStartPoint.x(), mStartPoint.y(), 0, 0), QPen(mPenColor, mnPenwidth), QBrush(Qt::transparent));
            mvLayers[layerIndex].graphicsItems.append(mCurrentItem);
            mvLayers[layerIndex].graphicsOriginalPositions.append(mStartPoint); // 记录实际位置
        }
    }
}

void LayeredCanvas::mouseMoveEvent(QMouseEvent* event) {
    QPointF scenePos = mapToScene(event->pos()); // 获取场景坐标

    if (mbIsDragging && mSelectedItem) {
        // 仅在 Move 模式下执行移动操作
        if (mDrawMode == Move) {
            int layerIndex = mvLayerItems.indexOf(mSelectedItem);
            if (layerIndex != -1) {
                QPointF delta = (event->pos() - mLastMousePos) * 2; // 增加移动灵敏度
                Layer& layer = mvLayers[layerIndex];

                // 移动图层内的所有图形项
                for (auto& graphicsItem : layer.graphicsItems) {
                    graphicsItem->moveBy(delta.x() / mdScaleFactor, delta.y() / mdScaleFactor);
                }

                // 移动选中的项
                mSelectedItem->moveBy(delta.x() / mdScaleFactor, delta.y() / mdScaleFactor);
                layer.position += delta / mdScaleFactor; // 更新图层位置

                mLastMousePos = event->pos(); // 更新最后的鼠标位置
            }
        }
        else if (mDrawMode == DrawPolyline && mCurrentLine) {
            // 更新线段的结束点
            mCurrentLine->setLine(QLineF(mStartPoint, scenePos)); // 直接设置线段
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
            // 拖动场景的逻辑
            QPointF delta = (event->pos() - mLastMousePos) / mdScaleFactor; // 根据缩放比例调整delta
            mPanOffset -= delta; // 使用减法反转移动方向
            mLastMousePos = event->pos(); // 更新最后的鼠标位置
            mpScene->setSceneRect(mpScene->sceneRect().translated(-delta)); // 同样反转delta
        }
    }
}

void LayeredCanvas::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && mbIsDragging) {
        if (mDrawMode == DrawPolyline && mCurrentLine) {
            // 确定线段的结束点并添加到图层
            QPointF endPoint = mapToScene(event->pos());
            mCurrentLine->setLine(QLineF(mStartPoint, endPoint)); // 更新线段的结束点
            int layerIndex = mvLayerItems.indexOf(mSelectedItem);
            mvLayers[layerIndex].graphicsOriginalPositions.append(mStartPoint);
            mvLayers[layerIndex].graphicsOriginalPositions.append(endPoint);
        }

        // 结束拖动状态
        mbIsDragging = false;
        if (mCurrentLine) {
            // 线段创建完成后，清除临时线段
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
    setTransform(QTransform::fromScale(mdScaleFactor, mdScaleFactor)); // 应用缩放
}

void LayeredCanvas::setPenColor(const QColor& color) {
    mPenColor = color; // 更新画笔颜色
}

void LayeredCanvas::setPenWidth(int width) {
    mnPenwidth = width;
}



