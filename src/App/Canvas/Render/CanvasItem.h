#pragma once

#include "App/Canvas/Object/BaseObject.h"
#include "App/Canvas/Object/CanvasObject.h"
#include "Common/QmlProperty.hpp"

#include <QColor>
#include <QPointF>
#include <QQuickFramebufferObject>
#include <QSizeF>
#include <QTransform>
#include <QUrl>

#include <memory>
#include <vector>

class QMouseEvent;
class QWheelEvent;

class CanvasItem : public QQuickFramebufferObject
{
    Q_OBJECT

public:
    explicit CanvasItem(QQuickItem *parent = nullptr);

    Renderer *createRenderer() const override;
    const CanvasObject &canvasObject() const;
    const QTransform &canvasTransform() const;
    std::vector<std::unique_ptr<BaseObject>> cloneObjects() const;

    PIXELFORGE_QML_QT_PROPERTY(QSizeF, documentSize, DocumentSize)
    PIXELFORGE_QML_VALUE_PROPERTY(qreal, zoom, Zoom)
    PIXELFORGE_QML_QT_PROPERTY(QPointF, contentOffset, ContentOffset)
    PIXELFORGE_QML_QT_PROPERTY(QColor, backgroundColor, BackgroundColor)
    PIXELFORGE_QML_QT_PROPERTY(QColor, canvasColor, CanvasColor)
    PIXELFORGE_QML_QT_PROPERTY(QColor, checkerColorA, CheckerColorA)
    PIXELFORGE_QML_QT_PROPERTY(QColor, checkerColorB, CheckerColorB)
    PIXELFORGE_QML_QT_PROPERTY(QColor, borderColor, BorderColor)
    PIXELFORGE_QML_VALUE_PROPERTY(bool, checkerboardVisible, CheckerboardVisible)
    PIXELFORGE_QML_VALUE_PROPERTY(int, checkerboardSize, CheckerboardSize)
    PIXELFORGE_QML_VALUE_PROPERTY(bool, interactive, Interactive)

    Q_INVOKABLE void resetView();
    Q_INVOKABLE bool loadImage(const QUrl &fileUrl);

protected:
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    ObjectId nextObjectId();
    void clearObjects();
    void addObject(std::unique_ptr<BaseObject> object);
    void updateCanvasTransform();
    void updateInteractionState();

    CanvasObject m_canvas;
    qreal m_zoom {1.0};
    QPointF m_contentOffset;
    bool m_interactive {true};
    bool m_isPanning {false};
    QPointF m_lastPanPosition;
    ObjectId m_nextObjectId {1};
    std::vector<std::unique_ptr<BaseObject>> m_objects;
};
