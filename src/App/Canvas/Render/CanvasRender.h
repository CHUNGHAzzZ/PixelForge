#pragma once

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
    CanvasObjectList cloneObjects() const;
    SceneDirtyRectList consumeDirtySceneRects(bool *allDirty);

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
    BaseObject *objectById(ObjectId id);
    const BaseObject *objectById(ObjectId id) const;
    BaseObject *hitTestObject(const QPointF &canvasPoint);
    void setSelectedObjectId(ObjectId id);
    void clearObjects();
    void addObject(std::unique_ptr<BaseObject> object);
    void markAllSceneDirty();
    void markSceneDirty(const QRectF &sceneRect);
    void markObjectDirty(const QRectF &beforeBounds, const QRectF &afterBounds);
    void updateViewportTransform();
    void updateInteractionState();

    CanvasObject m_canvas;
    QTransform m_viewportTransform;
    qreal m_zoom {1.0};
    QPointF m_contentOffset;
    bool m_interactive {true};
    bool m_isPanning {false};
    bool m_isMovingObject {false};
    QPointF m_lastPanPosition;
    QPointF m_lastObjectMoveCanvasPosition;
    QPointF m_objectGrabLocalPosition;
    ObjectId m_selectedObjectId {InvalidObjectId};
    ObjectId m_nextObjectId {1};
    CanvasObjectList m_objects;
    bool m_allCanvasDirty {true};
    SceneDirtyRectList m_dirtySceneRects;
};

}
