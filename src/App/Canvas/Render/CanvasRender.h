#pragma once

#include "App/Canvas/CanvasDocument.h"
#include "App/Canvas/CanvasProjection.h"
#include "App/Canvas/Render/CanvasTypes.hpp"
#include "Utils/QmlProperty.hpp"

#include <QColor>
#include <QPointF>
#include <QQuickFramebufferObject>
#include <QRectF>
#include <QSizeF>
#include <QTransform>
#include <QUrl>

#include <memory>
#include <vector>

class QMouseEvent;
class QWheelEvent;

namespace PixelForge {

class CanvasRender : public QQuickFramebufferObject
{
    Q_OBJECT

public:
    explicit CanvasRender(QQuickItem *parent = nullptr);

    Renderer *createRenderer() const override;
    const CanvasObject &canvasObject() const;
    const QTransform &viewportTransform() const;
    ObjectId selectedObjectId() const;
    CanvasImageRenderSnapshotList imageRenderSnapshots() const;
    QRect cacheBounds() const;
    CanvasOpenGLUpdateInfoList consumeOpenGLUpdateInfos(bool *allDirty);

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
    void setSelectedObjectId(ObjectId id);
    void updateViewportTransform();
    void updateInteractionState();

    CanvasDocument m_document;
    CanvasProjection m_projection;
    QTransform m_viewportTransform;
    qreal m_zoom {1.0};
    QPointF m_contentOffset;
    bool m_interactive {true};
    bool m_isPanning {false};
    bool m_isMovingObject {false};
    QPointF m_lastPanPosition;
    QPointF m_lastObjectMoveCanvasPosition;
    QPointF m_objectGrabLocalPosition;
};

}
