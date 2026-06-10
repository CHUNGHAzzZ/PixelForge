#include "CanvasRender.h"

#include "App/Canvas/Common.hpp"
#include "App/Canvas/Object/ImageObject.h"
#include "App/Canvas/Render/CanvasRenderer.h"
#include "Utils/Logger.h"

#include <QMouseEvent>
#include <QWheelEvent>

#include <opencv2/imgcodecs.hpp>

#include <algorithm>
#include <cmath>
#include <memory>

namespace PixelForge {

CanvasRender::CanvasRender(QQuickItem *parent)
    : QQuickFramebufferObject(parent)
{
    setMirrorVertically(true);
    updateViewportTransform();
    updateInteractionState();
    resetView();
}

QQuickFramebufferObject::Renderer *CanvasRender::createRenderer() const
{
    return new CanvasRenderer();
}

const CanvasObject &CanvasRender::canvasObject() const
{
    return m_canvas;
}

const QTransform &CanvasRender::viewportTransform() const
{
    return m_viewportTransform;
}

ObjectId CanvasRender::selectedObjectId() const
{
    return m_selectedObjectId;
}

CanvasObjectList CanvasRender::cloneObjects() const
{
    CanvasObjectList objects;
    objects.reserve(m_objects.size());

    for (const auto &object : m_objects) {
        if (object) {
            objects.push_back(object->clone());
        }
    }

    return objects;
}

SceneDirtyRectList CanvasRender::consumeDirtySceneRects(bool *allDirty)
{
    if (allDirty) {
        *allDirty = m_allCanvasDirty;
    }

    SceneDirtyRectList dirtyRects = std::move(m_dirtySceneRects);
    m_dirtySceneRects.clear();
    m_allCanvasDirty = false;
    return dirtyRects;
}

QSizeF CanvasRender::documentSize() const
{
    return m_canvas.size();
}

void CanvasRender::setDocumentSize(const QSizeF &size)
{
    const QSizeF boundedSize(std::max<qreal>(1.0, size.width()), std::max<qreal>(1.0, size.height()));
    if (m_canvas.size() == boundedSize) {
        return;
    }

    m_canvas.setSize(boundedSize);
    Logger::info(formatLogMessage(
        "Document size changed to %.0fx%.0f",
        boundedSize.width(),
        boundedSize.height()));
    markAllSceneDirty();
    updateViewportTransform();
    emit documentSizeChanged();
    update();
}

qreal CanvasRender::zoom() const
{
    return m_zoom;
}

void CanvasRender::setZoom(qreal zoom)
{
    const qreal boundedZoom = std::clamp(zoom, 0.05, 32.0);
    if (qFuzzyCompare(m_zoom, boundedZoom)) {
        return;
    }

    m_zoom = boundedZoom;
    updateViewportTransform();
    emit zoomChanged();
    update();
}

QPointF CanvasRender::contentOffset() const
{
    return m_contentOffset;
}

void CanvasRender::setContentOffset(const QPointF &offset)
{
    if (m_contentOffset == offset) {
        return;
    }

    m_contentOffset = offset;
    updateViewportTransform();
    emit contentOffsetChanged();
    update();
}

QColor CanvasRender::backgroundColor() const
{
    return m_canvas.backgroundColor();
}

void CanvasRender::setBackgroundColor(const QColor &color)
{
    if (m_canvas.backgroundColor() == color || !color.isValid()) {
        return;
    }

    m_canvas.setBackgroundColor(color);
    Logger::debug(formatLogMessage("Viewport background color changed: %s", color.name(QColor::HexArgb).toUtf8().constData()));
    markAllSceneDirty();
    emit backgroundColorChanged();
    update();
}

QColor CanvasRender::canvasColor() const
{
    return m_canvas.color();
}

void CanvasRender::setCanvasColor(const QColor &color)
{
    if (m_canvas.color() == color || !color.isValid()) {
        return;
    }

    m_canvas.setColor(color);
    Logger::debug(formatLogMessage("Canvas color changed: %s", color.name(QColor::HexArgb).toUtf8().constData()));
    markAllSceneDirty();
    emit canvasColorChanged();
    update();
}

QColor CanvasRender::checkerColorA() const
{
    return m_canvas.gridColorA();
}

void CanvasRender::setCheckerColorA(const QColor &color)
{
    if (m_canvas.gridColorA() == color || !color.isValid()) {
        return;
    }

    m_canvas.setGridColorA(color);
    Logger::debug(formatLogMessage("Canvas checkerboard color A changed: %s", color.name(QColor::HexArgb).toUtf8().constData()));
    markAllSceneDirty();
    emit checkerColorAChanged();
    update();
}

QColor CanvasRender::checkerColorB() const
{
    return m_canvas.gridColorB();
}

void CanvasRender::setCheckerColorB(const QColor &color)
{
    if (m_canvas.gridColorB() == color || !color.isValid()) {
        return;
    }

    m_canvas.setGridColorB(color);
    Logger::debug(formatLogMessage("Canvas checkerboard color B changed: %s", color.name(QColor::HexArgb).toUtf8().constData()));
    markAllSceneDirty();
    emit checkerColorBChanged();
    update();
}

QColor CanvasRender::borderColor() const
{
    return m_canvas.borderColor();
}

void CanvasRender::setBorderColor(const QColor &color)
{
    if (m_canvas.borderColor() == color || !color.isValid()) {
        return;
    }

    m_canvas.setBorderColor(color);
    Logger::debug(formatLogMessage("Canvas border color changed: %s", color.name(QColor::HexArgb).toUtf8().constData()));
    markAllSceneDirty();
    emit borderColorChanged();
    update();
}

bool CanvasRender::checkerboardVisible() const
{
    return m_canvas.gridVisible();
}

void CanvasRender::setCheckerboardVisible(bool visible)
{
    if (m_canvas.gridVisible() == visible) {
        return;
    }

    m_canvas.setGridVisible(visible);
    Logger::debug(formatLogMessage("Canvas checkerboard visibility changed: %s", visible ? "true" : "false"));
    markAllSceneDirty();
    emit checkerboardVisibleChanged();
    update();
}

int CanvasRender::checkerboardSize() const
{
    return m_canvas.gridSize();
}

void CanvasRender::setCheckerboardSize(int size)
{
    const int boundedSize = std::clamp(size, 1, 512);
    if (m_canvas.gridSize() == boundedSize) {
        return;
    }

    m_canvas.setGridSize(boundedSize);
    Logger::debug(formatLogMessage("Canvas checkerboard size changed: %d", boundedSize));
    markAllSceneDirty();
    emit checkerboardSizeChanged();
    update();
}

bool CanvasRender::interactive() const
{
    return m_interactive;
}

void CanvasRender::setInteractive(bool interactive)
{
    if (m_interactive == interactive) {
        return;
    }

    m_interactive = interactive;
    updateInteractionState();
    emit interactiveChanged();
}

void CanvasRender::resetView()
{
    const qreal availableWidth = std::max<qreal>(1.0, width() - 48.0);
    const qreal availableHeight = std::max<qreal>(1.0, height() - 48.0);
    const qreal zoomX = availableWidth / m_canvas.size().width();
    const qreal zoomY = availableHeight / m_canvas.size().height();
    setZoom(std::clamp(std::min(zoomX, zoomY), 0.05, 1.0));
    setContentOffset(QPointF());
}

bool CanvasRender::loadImage(const QUrl &fileUrl)
{
    if (!fileUrl.isLocalFile()) {
        Logger::warning("Canvas image load ignored non-local URL");
        return false;
    }

    const QString localPath = fileUrl.toLocalFile();
    Logger::info(formatLogMessage("Loading image into canvas: %s", localPath.toUtf8().constData()));

    const cv::Mat decodedImage = cv::imread(localPath.toStdString(), cv::IMREAD_UNCHANGED);
    QImage loadedImage = imageFromMat(decodedImage);
    if (loadedImage.isNull()) {
        Logger::error(formatLogMessage("Failed to decode image: %s", localPath.toUtf8().constData()));
        return false;
    }

    auto imageObject = std::make_unique<ImageObject>(nextObjectId(), loadedImage);
    imageObject->setSourcePath(localPath);

    const QSizeF imageSize(loadedImage.size());
    const QSizeF canvasSize = m_canvas.size();
    const QSizeF availableSize(canvasSize.width() * 0.8, canvasSize.height() * 0.8);
    const qreal scale = std::min<qreal>(
        1.0,
        std::min(
            availableSize.width() / std::max<qreal>(1.0, imageSize.width()),
            availableSize.height() / std::max<qreal>(1.0, imageSize.height())));
    const QSizeF displaySize(imageSize.width() * scale, imageSize.height() * scale);

    const qreal stagger = 32.0 * static_cast<qreal>(m_objects.size() % 8);
    const QPointF centeredPosition(
        (canvasSize.width() - displaySize.width()) * 0.5 + stagger,
        (canvasSize.height() - displaySize.height()) * 0.5 + stagger);
    QTransform imageTransform;
    imageTransform.translate(centeredPosition.x(), centeredPosition.y());
    imageTransform.scale(scale, scale);
    imageObject->setTransform(imageTransform);

    const bool firstObject = m_objects.empty();
    addObject(std::move(imageObject));
    if (firstObject) {
        resetView();
    }
    update();
    Logger::info(formatLogMessage(
        "Image object added: %s source=%dx%d display=%.0fx%.0f position=%.0f,%.0f",
        localPath.toUtf8().constData(),
        loadedImage.width(),
        loadedImage.height(),
        displaySize.width(),
        displaySize.height(),
        centeredPosition.x(),
        centeredPosition.y()));
    return true;
}

void CanvasRender::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickFramebufferObject::geometryChange(newGeometry, oldGeometry);
    updateViewportTransform();

    if (oldGeometry.size().isEmpty()) {
        resetView();
    }
}

void CanvasRender::mousePressEvent(QMouseEvent *event)
{
    if (!m_interactive || event->button() != Qt::LeftButton) {
        QQuickFramebufferObject::mousePressEvent(event);
        return;
    }

    bool invertible = false;
    const QPointF canvasPoint = m_viewportTransform.inverted(&invertible).map(event->position());
    BaseObject *hitObject = invertible ? hitTestObject(canvasPoint) : nullptr;
    if (hitObject) {
        setSelectedObjectId(hitObject->id());
        m_isMovingObject = true;
        m_lastObjectMoveCanvasPosition = canvasPoint;
        m_objectGrabLocalPosition = hitObject->canvasToLocal(canvasPoint);
    } else {
        setSelectedObjectId(InvalidObjectId);
        m_isPanning = true;
        m_lastPanPosition = event->position();
    }
    event->accept();
}

void CanvasRender::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_interactive) {
        QQuickFramebufferObject::mouseMoveEvent(event);
        return;
    }

    if (m_isMovingObject) {
        bool invertible = false;
        const QPointF canvasPoint = m_viewportTransform.inverted(&invertible).map(event->position());
        BaseObject *object = objectById(m_selectedObjectId);
        if (invertible && object) {
            const QRectF beforeBounds = object->canvasBounds();
            const QPointF grabbedCanvasPosition = object->localToCanvas(m_objectGrabLocalPosition);
            object->translate(canvasPoint - grabbedCanvasPosition);
            const QRectF afterBounds = object->canvasBounds();
            markObjectDirty(beforeBounds, afterBounds);
            m_lastObjectMoveCanvasPosition = canvasPoint;
            update();
        }
        event->accept();
        return;
    }

    if (m_isPanning) {
        setContentOffset(m_contentOffset + event->position() - m_lastPanPosition);
        m_lastPanPosition = event->position();
        event->accept();
        return;
    }

    QQuickFramebufferObject::mouseMoveEvent(event);
}

void CanvasRender::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_interactive || event->button() != Qt::LeftButton) {
        QQuickFramebufferObject::mouseReleaseEvent(event);
        return;
    }

    m_isPanning = false;
    m_isMovingObject = false;
    event->accept();
}

void CanvasRender::wheelEvent(QWheelEvent *event)
{
    if (!m_interactive) {
        QQuickFramebufferObject::wheelEvent(event);
        return;
    }

    const QPoint angleDelta = event->angleDelta();
    if (angleDelta.y() == 0) {
        QQuickFramebufferObject::wheelEvent(event);
        return;
    }

    const qreal zoomStep = std::pow(1.0015, angleDelta.y());
    const QPointF focus = event->position();

    bool invertible = false;
    const QPointF canvasFocus = m_viewportTransform.inverted(&invertible).map(focus);
    if (!invertible) {
        QQuickFramebufferObject::wheelEvent(event);
        return;
    }

    setZoom(m_zoom * zoomStep);

    const QPointF centeredTopLeft(
        (width() - m_canvas.size().width() * m_zoom) * 0.5,
        (height() - m_canvas.size().height() * m_zoom) * 0.5);
    const QPointF afterOffset = focus - QPointF(canvasFocus.x() * m_zoom, canvasFocus.y() * m_zoom) - centeredTopLeft;
    setContentOffset(afterOffset);
    event->accept();
}

void CanvasRender::updateViewportTransform()
{
    const QPointF centeredTopLeft(
        (width() - m_canvas.size().width() * m_zoom) * 0.5,
        (height() - m_canvas.size().height() * m_zoom) * 0.5);

    QTransform transform;
    transform.translate(centeredTopLeft.x() + m_contentOffset.x(), centeredTopLeft.y() + m_contentOffset.y());
    transform.scale(m_zoom, m_zoom);
    m_viewportTransform = transform;
}

ObjectId CanvasRender::nextObjectId()
{
    return m_nextObjectId++;
}

BaseObject *CanvasRender::objectById(ObjectId id)
{
    if (id == InvalidObjectId) {
        return nullptr;
    }

    for (const auto &object : m_objects) {
        if (object && object->id() == id) {
            return object.get();
        }
    }

    return nullptr;
}

const BaseObject *CanvasRender::objectById(ObjectId id) const
{
    if (id == InvalidObjectId) {
        return nullptr;
    }

    for (const auto &object : m_objects) {
        if (object && object->id() == id) {
            return object.get();
        }
    }

    return nullptr;
}

BaseObject *CanvasRender::hitTestObject(const QPointF &canvasPoint)
{
    for (auto it = m_objects.rbegin(); it != m_objects.rend(); ++it) {
        BaseObject *object = it->get();
        if (!object || !object->canvasBounds().contains(canvasPoint)) {
            continue;
        }

        const QPointF localPoint = object->canvasToLocal(canvasPoint);
        if (object->localBounds().contains(localPoint)) {
            return object;
        }
    }

    return nullptr;
}

void CanvasRender::setSelectedObjectId(ObjectId id)
{
    if (m_selectedObjectId == id) {
        return;
    }

    const BaseObject *beforeObject = objectById(m_selectedObjectId);
    const BaseObject *afterObject = objectById(id);
    const QRectF beforeBounds = beforeObject ? beforeObject->canvasBounds() : QRectF();
    const QRectF afterBounds = afterObject ? afterObject->canvasBounds() : QRectF();
    m_selectedObjectId = id;
    markObjectDirty(beforeBounds, afterBounds);
    Logger::debug(formatLogMessage("Selected canvas object id=%llu", static_cast<unsigned long long>(id)));
    update();
}

void CanvasRender::clearObjects()
{
    for (const auto &object : m_objects) {
        if (object) {
            markSceneDirty(object->canvasBounds());
        }
    }

    m_objects.clear();
    m_selectedObjectId = InvalidObjectId;
    Logger::debug("Cleared canvas objects");
}

void CanvasRender::addObject(std::unique_ptr<BaseObject> object)
{
    if (!object) {
        return;
    }

    if (object->id() == InvalidObjectId) {
        object->setId(nextObjectId());
    }

    markSceneDirty(object->canvasBounds());
    Logger::debug(formatLogMessage(
        "Added canvas object id=%llu type=%d",
        static_cast<unsigned long long>(object->id()),
        static_cast<int>(object->type())));
    m_objects.push_back(std::move(object));
    update();
}

void CanvasRender::markAllSceneDirty()
{
    m_allCanvasDirty = true;
    m_dirtySceneRects.clear();
}

void CanvasRender::markSceneDirty(const QRectF &sceneRect)
{
    if (sceneRect.isEmpty()) {
        return;
    }

    m_dirtySceneRects.push_back(sceneRect);
}

void CanvasRender::markObjectDirty(const QRectF &beforeBounds, const QRectF &afterBounds)
{
    const QRectF dirtyBounds = beforeBounds.united(afterBounds).adjusted(-2.0, -2.0, 2.0, 2.0);
    markSceneDirty(dirtyBounds);
}

void CanvasRender::updateInteractionState()
{
    setAcceptedMouseButtons(m_interactive ? Qt::LeftButton : Qt::NoButton);
    setAcceptHoverEvents(m_interactive);
}

}
