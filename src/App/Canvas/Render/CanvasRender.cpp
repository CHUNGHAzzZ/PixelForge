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
    return m_document.canvas();
}

const QTransform &CanvasRender::viewportTransform() const
{
    return m_viewportTransform;
}

ObjectId CanvasRender::selectedObjectId() const
{
    return m_document.selectedObjectId();
}

CanvasImageRenderSnapshotList CanvasRender::imageRenderSnapshots() const
{
    return m_projection.imageRenderSnapshots(m_document);
}

QRect CanvasRender::cacheBounds() const
{
    return m_projection.cacheBounds(m_document);
}

CanvasOpenGLUpdateInfoList CanvasRender::consumeOpenGLUpdateInfos(bool *allDirty)
{
    if (allDirty) {
        *allDirty = m_document.allDirty();
    }

    return m_projection.consumeOpenGLUpdateInfos(m_document);
}

QSizeF CanvasRender::documentSize() const
{
    return m_document.canvas().size();
}

void CanvasRender::setDocumentSize(const QSizeF &size)
{
    const QSizeF boundedSize(std::max<qreal>(1.0, size.width()), std::max<qreal>(1.0, size.height()));
    if (m_document.canvas().size() == boundedSize) {
        return;
    }

    m_document.canvas().setSize(boundedSize);
    Logger::info(formatLogMessage(
        "Document size changed to %.0fx%.0f",
        boundedSize.width(),
        boundedSize.height()));
    m_document.markAllDirty();
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
    return m_document.canvas().backgroundColor();
}

void CanvasRender::setBackgroundColor(const QColor &color)
{
    if (m_document.canvas().backgroundColor() == color || !color.isValid()) {
        return;
    }

    m_document.canvas().setBackgroundColor(color);
    Logger::debug(formatLogMessage("Viewport background color changed: %s", color.name(QColor::HexArgb).toUtf8().constData()));
    m_document.markAllDirty();
    emit backgroundColorChanged();
    update();
}

QColor CanvasRender::canvasColor() const
{
    return m_document.canvas().color();
}

void CanvasRender::setCanvasColor(const QColor &color)
{
    if (m_document.canvas().color() == color || !color.isValid()) {
        return;
    }

    m_document.canvas().setColor(color);
    Logger::debug(formatLogMessage("Canvas color changed: %s", color.name(QColor::HexArgb).toUtf8().constData()));
    m_document.markAllDirty();
    emit canvasColorChanged();
    update();
}

QColor CanvasRender::checkerColorA() const
{
    return m_document.canvas().gridColorA();
}

void CanvasRender::setCheckerColorA(const QColor &color)
{
    if (m_document.canvas().gridColorA() == color || !color.isValid()) {
        return;
    }

    m_document.canvas().setGridColorA(color);
    Logger::debug(formatLogMessage("Canvas checkerboard color A changed: %s", color.name(QColor::HexArgb).toUtf8().constData()));
    m_document.markAllDirty();
    emit checkerColorAChanged();
    update();
}

QColor CanvasRender::checkerColorB() const
{
    return m_document.canvas().gridColorB();
}

void CanvasRender::setCheckerColorB(const QColor &color)
{
    if (m_document.canvas().gridColorB() == color || !color.isValid()) {
        return;
    }

    m_document.canvas().setGridColorB(color);
    Logger::debug(formatLogMessage("Canvas checkerboard color B changed: %s", color.name(QColor::HexArgb).toUtf8().constData()));
    m_document.markAllDirty();
    emit checkerColorBChanged();
    update();
}

QColor CanvasRender::borderColor() const
{
    return m_document.canvas().borderColor();
}

void CanvasRender::setBorderColor(const QColor &color)
{
    if (m_document.canvas().borderColor() == color || !color.isValid()) {
        return;
    }

    m_document.canvas().setBorderColor(color);
    Logger::debug(formatLogMessage("Canvas border color changed: %s", color.name(QColor::HexArgb).toUtf8().constData()));
    m_document.markAllDirty();
    emit borderColorChanged();
    update();
}

bool CanvasRender::checkerboardVisible() const
{
    return m_document.canvas().gridVisible();
}

void CanvasRender::setCheckerboardVisible(bool visible)
{
    if (m_document.canvas().gridVisible() == visible) {
        return;
    }

    m_document.canvas().setGridVisible(visible);
    Logger::debug(formatLogMessage("Canvas checkerboard visibility changed: %s", visible ? "true" : "false"));
    m_document.markAllDirty();
    emit checkerboardVisibleChanged();
    update();
}

int CanvasRender::checkerboardSize() const
{
    return m_document.canvas().gridSize();
}

void CanvasRender::setCheckerboardSize(int size)
{
    const int boundedSize = std::clamp(size, 1, 512);
    if (m_document.canvas().gridSize() == boundedSize) {
        return;
    }

    m_document.canvas().setGridSize(boundedSize);
    Logger::debug(formatLogMessage("Canvas checkerboard size changed: %d", boundedSize));
    m_document.markAllDirty();
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
    const qreal zoomX = availableWidth / m_document.canvas().size().width();
    const qreal zoomY = availableHeight / m_document.canvas().size().height();
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

    auto imageObject = std::make_unique<ImageObject>(m_document.nextObjectId(), loadedImage);
    imageObject->setSourcePath(localPath);

    const bool firstObject = m_document.objects().empty();
    const QSizeF imageSize(loadedImage.size());
    if (firstObject) {
        setDocumentSize(imageSize);
    }

    const QSizeF canvasSize = m_document.canvas().size();
    const qreal stagger = 32.0 * static_cast<qreal>(m_document.objects().size() % 8);
    const QPointF imagePosition = firstObject
        ? QPointF()
        : QPointF(
            (canvasSize.width() - imageSize.width()) * 0.5 + stagger,
            (canvasSize.height() - imageSize.height()) * 0.5 + stagger);
    QTransform imageTransform;
    imageTransform.translate(imagePosition.x(), imagePosition.y());
    imageObject->setTransform(imageTransform);

    m_document.addObject(std::move(imageObject));
    if (firstObject) {
        resetView();
    }
    update();
    Logger::info(formatLogMessage(
        "Image object added: %s source=%dx%d display=1:1 position=%.0f,%.0f",
        localPath.toUtf8().constData(),
        loadedImage.width(),
        loadedImage.height(),
        imagePosition.x(),
        imagePosition.y()));
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
    BaseObject *hitObject = invertible ? m_document.hitTestObject(canvasPoint) : nullptr;
    if (hitObject) {
        const ObjectId hitObjectId = hitObject->id();
        const QPointF hitObjectLocalPosition = hitObject->canvasToLocal(canvasPoint);
        setSelectedObjectId(hitObjectId);
        m_isMovingObject = true;
        m_lastObjectMoveCanvasPosition = canvasPoint;
        m_objectGrabLocalPosition = hitObjectLocalPosition;
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
        BaseObject *object = m_document.objectById(m_document.selectedObjectId());
        if (invertible && object) {
            const QRectF beforeBounds = object->canvasBounds();
            const QPointF grabbedCanvasPosition = object->localToCanvas(m_objectGrabLocalPosition);
            object->translate(canvasPoint - grabbedCanvasPosition);
            const QRectF afterBounds = object->canvasBounds();
            if (object->type() != ObjectType::Image) {
                m_document.markObjectDirty(beforeBounds, afterBounds);
            }
            m_lastObjectMoveCanvasPosition = canvasPoint;
            update();
        }
        event->accept();
        return;
    }

    if (m_isPanning) {
        m_contentOffset += event->position() - m_lastPanPosition;
        updateViewportTransform();
        emit contentOffsetChanged();
        update();
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

    const qreal oldZoom = m_zoom;
    m_zoom = std::clamp(m_zoom * zoomStep, 0.05, 32.0);
    if (qFuzzyCompare(oldZoom, m_zoom)) {
        event->accept();
        return;
    }

    const QPointF centeredTopLeft(
        (width() - m_document.canvas().size().width() * m_zoom) * 0.5,
        (height() - m_document.canvas().size().height() * m_zoom) * 0.5);
    m_contentOffset = focus - QPointF(canvasFocus.x() * m_zoom, canvasFocus.y() * m_zoom) - centeredTopLeft;
    updateViewportTransform();
    emit zoomChanged();
    emit contentOffsetChanged();
    update();
    event->accept();
}

void CanvasRender::updateViewportTransform()
{
    const QPointF centeredTopLeft(
        (width() - m_document.canvas().size().width() * m_zoom) * 0.5,
        (height() - m_document.canvas().size().height() * m_zoom) * 0.5);

    QTransform transform;
    transform.translate(centeredTopLeft.x() + m_contentOffset.x(), centeredTopLeft.y() + m_contentOffset.y());
    transform.scale(m_zoom, m_zoom);
    m_viewportTransform = transform;
}

void CanvasRender::setSelectedObjectId(ObjectId id)
{
    if (m_document.selectedObjectId() == id) {
        return;
    }

    const BaseObject *beforeObject = m_document.objectById(m_document.selectedObjectId());
    const BaseObject *afterObject = m_document.objectById(id);
    const QRectF beforeBounds = beforeObject ? beforeObject->canvasBounds() : QRectF();
    const QRectF afterBounds = afterObject ? afterObject->canvasBounds() : QRectF();
    const bool beforeNeedsTileUpdate = beforeObject && beforeObject->type() != ObjectType::Image;
    const bool afterNeedsTileUpdate = afterObject && afterObject->type() != ObjectType::Image;
    const bool selectedObjectIsImage = afterObject && afterObject->type() == ObjectType::Image;
    m_document.setSelectedObjectId(id);
    const bool selectedImageMovedToFront = selectedObjectIsImage && m_document.bringObjectToFront(id);
    if (beforeNeedsTileUpdate || afterNeedsTileUpdate) {
        m_document.markObjectDirty(beforeBounds, afterBounds);
    }
    if (selectedImageMovedToFront) {
        Logger::debug(formatLogMessage("Selected image object raised id=%llu", static_cast<unsigned long long>(id)));
    }
    Logger::debug(formatLogMessage("Selected canvas object id=%llu", static_cast<unsigned long long>(id)));
    update();
}

void CanvasRender::updateInteractionState()
{
    setAcceptedMouseButtons(m_interactive ? Qt::LeftButton : Qt::NoButton);
    setAcceptHoverEvents(m_interactive);
}

}
