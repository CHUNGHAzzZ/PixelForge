#include "CanvasItem.h"

#include "App/Canvas/Object/ImageObject.h"

#include <QMouseEvent>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLPaintDevice>
#include <QPainter>
#include <QWheelEvent>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

namespace {
struct CanvasRenderState
{
    CanvasObject canvas;
    std::vector<std::unique_ptr<BaseObject>> objects;
};

class CanvasRenderer final : public QQuickFramebufferObject::Renderer, protected QOpenGLFunctions
{
public:
    void synchronize(QQuickFramebufferObject *item) override
    {
        const auto *canvas = qobject_cast<CanvasItem *>(item);
        if (!canvas) {
            return;
        }

        m_state.canvas = canvas->canvasObject();
        m_state.objects = canvas->cloneObjects();
    }

    void render() override
    {
        initializeOpenGLFunctions();

        glViewport(0, 0, m_size.width(), m_size.height());
        glClearColor(
            m_state.canvas.backgroundColor().redF(),
            m_state.canvas.backgroundColor().greenF(),
            m_state.canvas.backgroundColor().blueF(),
            m_state.canvas.backgroundColor().alphaF());
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        QOpenGLPaintDevice paintDevice(m_size);
        QPainter painter(&paintDevice);
        painter.setRenderHint(QPainter::Antialiasing, false);

        m_state.canvas.paint(painter);
        painter.save();
        painter.setTransform(m_state.canvas.transform(), true);
        for (const auto &object : m_state.objects) {
            if (object) {
                object->paint(painter);
            }
        }
        painter.restore();
        painter.end();
    }

    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override
    {
        m_size = size;

        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        format.setSamples(4);
        return new QOpenGLFramebufferObject(size, format);
    }

    QSize m_size;
    CanvasRenderState m_state;
};

QImage imageFromMat(const cv::Mat &image)
{
    if (image.empty()) {
        return {};
    }

    cv::Mat converted;
    switch (image.channels()) {
    case 1:
        cv::cvtColor(image, converted, cv::COLOR_GRAY2RGBA);
        break;
    case 3:
        cv::cvtColor(image, converted, cv::COLOR_BGR2RGBA);
        break;
    case 4:
        cv::cvtColor(image, converted, cv::COLOR_BGRA2RGBA);
        break;
    default:
        return {};
    }

    return QImage(
               converted.data,
               converted.cols,
               converted.rows,
               static_cast<qsizetype>(converted.step),
               QImage::Format_RGBA8888)
        .copy();
}
}

CanvasItem::CanvasItem(QQuickItem *parent)
    : QQuickFramebufferObject(parent)
{
    setMirrorVertically(true);
    updateCanvasTransform();
    updateInteractionState();
    resetView();
}

QQuickFramebufferObject::Renderer *CanvasItem::createRenderer() const
{
    return new CanvasRenderer();
}

const CanvasObject &CanvasItem::canvasObject() const
{
    return m_canvas;
}

const QTransform &CanvasItem::canvasTransform() const
{
    return m_canvas.transform();
}

std::vector<std::unique_ptr<BaseObject>> CanvasItem::cloneObjects() const
{
    std::vector<std::unique_ptr<BaseObject>> objects;
    objects.reserve(m_objects.size());

    for (const auto &object : m_objects) {
        if (object) {
            objects.push_back(object->clone());
        }
    }

    return objects;
}

QSizeF CanvasItem::documentSize() const
{
    return m_canvas.size();
}

void CanvasItem::setDocumentSize(const QSizeF &size)
{
    const QSizeF boundedSize(std::max<qreal>(1.0, size.width()), std::max<qreal>(1.0, size.height()));
    if (m_canvas.size() == boundedSize) {
        return;
    }

    m_canvas.setSize(boundedSize);
    updateCanvasTransform();
    emit documentSizeChanged();
    update();
}

qreal CanvasItem::zoom() const
{
    return m_zoom;
}

void CanvasItem::setZoom(qreal zoom)
{
    const qreal boundedZoom = std::clamp(zoom, 0.05, 32.0);
    if (qFuzzyCompare(m_zoom, boundedZoom)) {
        return;
    }

    m_zoom = boundedZoom;
    updateCanvasTransform();
    emit zoomChanged();
    update();
}

QPointF CanvasItem::contentOffset() const
{
    return m_contentOffset;
}

void CanvasItem::setContentOffset(const QPointF &offset)
{
    if (m_contentOffset == offset) {
        return;
    }

    m_contentOffset = offset;
    updateCanvasTransform();
    emit contentOffsetChanged();
    update();
}

QColor CanvasItem::backgroundColor() const
{
    return m_canvas.backgroundColor();
}

void CanvasItem::setBackgroundColor(const QColor &color)
{
    if (m_canvas.backgroundColor() == color || !color.isValid()) {
        return;
    }

    m_canvas.setBackgroundColor(color);
    emit backgroundColorChanged();
    update();
}

QColor CanvasItem::canvasColor() const
{
    return m_canvas.color();
}

void CanvasItem::setCanvasColor(const QColor &color)
{
    if (m_canvas.color() == color || !color.isValid()) {
        return;
    }

    m_canvas.setColor(color);
    emit canvasColorChanged();
    update();
}

QColor CanvasItem::checkerColorA() const
{
    return m_canvas.gridColorA();
}

void CanvasItem::setCheckerColorA(const QColor &color)
{
    if (m_canvas.gridColorA() == color || !color.isValid()) {
        return;
    }

    m_canvas.setGridColorA(color);
    emit checkerColorAChanged();
    update();
}

QColor CanvasItem::checkerColorB() const
{
    return m_canvas.gridColorB();
}

void CanvasItem::setCheckerColorB(const QColor &color)
{
    if (m_canvas.gridColorB() == color || !color.isValid()) {
        return;
    }

    m_canvas.setGridColorB(color);
    emit checkerColorBChanged();
    update();
}

QColor CanvasItem::borderColor() const
{
    return m_canvas.borderColor();
}

void CanvasItem::setBorderColor(const QColor &color)
{
    if (m_canvas.borderColor() == color || !color.isValid()) {
        return;
    }

    m_canvas.setBorderColor(color);
    emit borderColorChanged();
    update();
}

bool CanvasItem::checkerboardVisible() const
{
    return m_canvas.gridVisible();
}

void CanvasItem::setCheckerboardVisible(bool visible)
{
    if (m_canvas.gridVisible() == visible) {
        return;
    }

    m_canvas.setGridVisible(visible);
    emit checkerboardVisibleChanged();
    update();
}

int CanvasItem::checkerboardSize() const
{
    return m_canvas.gridSize();
}

void CanvasItem::setCheckerboardSize(int size)
{
    const int boundedSize = std::clamp(size, 1, 512);
    if (m_canvas.gridSize() == boundedSize) {
        return;
    }

    m_canvas.setGridSize(boundedSize);
    emit checkerboardSizeChanged();
    update();
}

bool CanvasItem::interactive() const
{
    return m_interactive;
}

void CanvasItem::setInteractive(bool interactive)
{
    if (m_interactive == interactive) {
        return;
    }

    m_interactive = interactive;
    updateInteractionState();
    emit interactiveChanged();
}

void CanvasItem::resetView()
{
    const qreal availableWidth = std::max<qreal>(1.0, width() - 48.0);
    const qreal availableHeight = std::max<qreal>(1.0, height() - 48.0);
    const qreal zoomX = availableWidth / m_canvas.size().width();
    const qreal zoomY = availableHeight / m_canvas.size().height();
    setZoom(std::clamp(std::min(zoomX, zoomY), 0.05, 1.0));
    setContentOffset(QPointF());
}

bool CanvasItem::loadImage(const QUrl &fileUrl)
{
    if (!fileUrl.isLocalFile()) {
        return false;
    }

    const cv::Mat decodedImage = cv::imread(fileUrl.toLocalFile().toStdString(), cv::IMREAD_UNCHANGED);
    QImage loadedImage = imageFromMat(decodedImage);
    if (loadedImage.isNull()) {
        return false;
    }

    auto imageObject = std::make_unique<ImageObject>(nextObjectId(), loadedImage);
    imageObject->setSourcePath(fileUrl.toLocalFile());

    clearObjects();
    addObject(std::move(imageObject));
    setDocumentSize(loadedImage.size());
    resetView();
    update();
    return true;
}

void CanvasItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickFramebufferObject::geometryChange(newGeometry, oldGeometry);
    updateCanvasTransform();

    if (oldGeometry.size().isEmpty()) {
        resetView();
    }
}

void CanvasItem::mousePressEvent(QMouseEvent *event)
{
    if (!m_interactive || event->button() != Qt::LeftButton) {
        QQuickFramebufferObject::mousePressEvent(event);
        return;
    }

    m_isPanning = true;
    m_lastPanPosition = event->position();
    event->accept();
}

void CanvasItem::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_interactive || !m_isPanning) {
        QQuickFramebufferObject::mouseMoveEvent(event);
        return;
    }

    setContentOffset(m_contentOffset + event->position() - m_lastPanPosition);
    m_lastPanPosition = event->position();
    event->accept();
}

void CanvasItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_interactive || event->button() != Qt::LeftButton) {
        QQuickFramebufferObject::mouseReleaseEvent(event);
        return;
    }

    m_isPanning = false;
    event->accept();
}

void CanvasItem::wheelEvent(QWheelEvent *event)
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
    const QPointF canvasFocus = m_canvas.transform().inverted(&invertible).map(focus);
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

void CanvasItem::updateCanvasTransform()
{
    const QPointF centeredTopLeft(
        (width() - m_canvas.size().width() * m_zoom) * 0.5,
        (height() - m_canvas.size().height() * m_zoom) * 0.5);

    QTransform transform;
    transform.translate(centeredTopLeft.x() + m_contentOffset.x(), centeredTopLeft.y() + m_contentOffset.y());
    transform.scale(m_zoom, m_zoom);
    m_canvas.setTransform(transform);
}

ObjectId CanvasItem::nextObjectId()
{
    return m_nextObjectId++;
}

void CanvasItem::clearObjects()
{
    m_objects.clear();
}

void CanvasItem::addObject(std::unique_ptr<BaseObject> object)
{
    if (!object) {
        return;
    }

    if (object->id() == InvalidObjectId) {
        object->setId(nextObjectId());
    }

    m_objects.push_back(std::move(object));
    update();
}

void CanvasItem::updateInteractionState()
{
    setAcceptedMouseButtons(m_interactive ? Qt::LeftButton : Qt::NoButton);
    setAcceptHoverEvents(m_interactive);
}
