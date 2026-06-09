#include "CanvasItem.h"

#include <QMouseEvent>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLPaintDevice>
#include <QPainter>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>

namespace {
struct CanvasRenderState
{
    QSizeF documentSize;
    qreal zoom {1.0};
    QPointF contentOffset;
    QColor backgroundColor;
    QColor canvasColor;
    QColor checkerColorA;
    QColor checkerColorB;
    QColor borderColor;
    bool checkerboardVisible {true};
    int checkerboardSize {16};
};

QRectF centeredDocumentRect(const QSize &viewportSize, const CanvasRenderState &state)
{
    const QSizeF documentPixelSize(
        state.documentSize.width() * state.zoom,
        state.documentSize.height() * state.zoom);

    return QRectF(
        (viewportSize.width() - documentPixelSize.width()) * 0.5 + state.contentOffset.x(),
        (viewportSize.height() - documentPixelSize.height()) * 0.5 + state.contentOffset.y(),
        documentPixelSize.width(),
        documentPixelSize.height());
}

class CanvasRenderer final : public QQuickFramebufferObject::Renderer, protected QOpenGLFunctions
{
public:
    void synchronize(QQuickFramebufferObject *item) override
    {
        const auto *canvas = qobject_cast<CanvasItem *>(item);
        if (!canvas) {
            return;
        }

        m_state.documentSize = canvas->documentSize();
        m_state.zoom = canvas->zoom();
        m_state.contentOffset = canvas->contentOffset();
        m_state.backgroundColor = canvas->backgroundColor();
        m_state.canvasColor = canvas->canvasColor();
        m_state.checkerColorA = canvas->checkerColorA();
        m_state.checkerColorB = canvas->checkerColorB();
        m_state.borderColor = canvas->borderColor();
        m_state.checkerboardVisible = canvas->checkerboardVisible();
        m_state.checkerboardSize = canvas->checkerboardSize();
    }

    void render() override
    {
        initializeOpenGLFunctions();

        glViewport(0, 0, m_size.width(), m_size.height());
        glClearColor(
            m_state.backgroundColor.redF(),
            m_state.backgroundColor.greenF(),
            m_state.backgroundColor.blueF(),
            m_state.backgroundColor.alphaF());
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        QOpenGLPaintDevice paintDevice(m_size);
        QPainter painter(&paintDevice);
        painter.setRenderHint(QPainter::Antialiasing, false);

        const QRectF documentRect = centeredDocumentRect(m_size, m_state);
        painter.fillRect(documentRect.translated(0.0, 10.0).adjusted(-10.0, 0.0, 10.0, 0.0), QColor(0, 0, 0, 42));

        if (m_state.checkerboardVisible) {
            drawCheckerboard(painter, documentRect);
        } else {
            painter.fillRect(documentRect, m_state.canvasColor);
        }

        painter.setPen(QPen(m_state.borderColor, 1.0));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(documentRect.adjusted(0.5, 0.5, -0.5, -0.5));
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

private:
    void drawCheckerboard(QPainter &painter, const QRectF &documentRect)
    {
        const qreal tileSize = std::max<qreal>(1.0, m_state.checkerboardSize * m_state.zoom);

        painter.fillRect(documentRect, m_state.canvasColor);

        const int columns = static_cast<int>(std::ceil(documentRect.width() / tileSize));
        const int rows = static_cast<int>(std::ceil(documentRect.height() / tileSize));

        for (int row = 0; row < rows; ++row) {
            for (int column = 0; column < columns; ++column) {
                const QColor &color = ((row + column) % 2 == 0) ? m_state.checkerColorA : m_state.checkerColorB;
                const QRectF tile(
                    documentRect.left() + column * tileSize,
                    documentRect.top() + row * tileSize,
                    std::min<qreal>(tileSize, documentRect.right() - (documentRect.left() + column * tileSize)),
                    std::min<qreal>(tileSize, documentRect.bottom() - (documentRect.top() + row * tileSize)));

                if (tile.width() > 0.0 && tile.height() > 0.0) {
                    painter.fillRect(tile, color);
                }
            }
        }
    }

    QSize m_size;
    CanvasRenderState m_state;
};
}

CanvasItem::CanvasItem(QQuickItem *parent)
    : QQuickFramebufferObject(parent)
{
    setMirrorVertically(true);
    updateInteractionState();
    resetView();
}

QQuickFramebufferObject::Renderer *CanvasItem::createRenderer() const
{
    return new CanvasRenderer();
}

QSizeF CanvasItem::documentSize() const
{
    return m_documentSize;
}

void CanvasItem::setDocumentSize(const QSizeF &size)
{
    const QSizeF boundedSize(std::max<qreal>(1.0, size.width()), std::max<qreal>(1.0, size.height()));
    if (m_documentSize == boundedSize) {
        return;
    }

    m_documentSize = boundedSize;
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
    emit contentOffsetChanged();
    update();
}

QColor CanvasItem::backgroundColor() const
{
    return m_backgroundColor;
}

void CanvasItem::setBackgroundColor(const QColor &color)
{
    if (m_backgroundColor == color || !color.isValid()) {
        return;
    }

    m_backgroundColor = color;
    emit backgroundColorChanged();
    update();
}

QColor CanvasItem::canvasColor() const
{
    return m_canvasColor;
}

void CanvasItem::setCanvasColor(const QColor &color)
{
    if (m_canvasColor == color || !color.isValid()) {
        return;
    }

    m_canvasColor = color;
    emit canvasColorChanged();
    update();
}

QColor CanvasItem::checkerColorA() const
{
    return m_checkerColorA;
}

void CanvasItem::setCheckerColorA(const QColor &color)
{
    if (m_checkerColorA == color || !color.isValid()) {
        return;
    }

    m_checkerColorA = color;
    emit checkerColorAChanged();
    update();
}

QColor CanvasItem::checkerColorB() const
{
    return m_checkerColorB;
}

void CanvasItem::setCheckerColorB(const QColor &color)
{
    if (m_checkerColorB == color || !color.isValid()) {
        return;
    }

    m_checkerColorB = color;
    emit checkerColorBChanged();
    update();
}

QColor CanvasItem::borderColor() const
{
    return m_borderColor;
}

void CanvasItem::setBorderColor(const QColor &color)
{
    if (m_borderColor == color || !color.isValid()) {
        return;
    }

    m_borderColor = color;
    emit borderColorChanged();
    update();
}

bool CanvasItem::checkerboardVisible() const
{
    return m_checkerboardVisible;
}

void CanvasItem::setCheckerboardVisible(bool visible)
{
    if (m_checkerboardVisible == visible) {
        return;
    }

    m_checkerboardVisible = visible;
    emit checkerboardVisibleChanged();
    update();
}

int CanvasItem::checkerboardSize() const
{
    return m_checkerboardSize;
}

void CanvasItem::setCheckerboardSize(int size)
{
    const int boundedSize = std::clamp(size, 1, 512);
    if (m_checkerboardSize == boundedSize) {
        return;
    }

    m_checkerboardSize = boundedSize;
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
    const qreal zoomX = availableWidth / m_documentSize.width();
    const qreal zoomY = availableHeight / m_documentSize.height();
    setZoom(std::clamp(std::min(zoomX, zoomY), 0.05, 1.0));
    setContentOffset(QPointF());
}

void CanvasItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickFramebufferObject::geometryChange(newGeometry, oldGeometry);

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

    const qreal oldZoom = m_zoom;
    const qreal zoomStep = std::pow(1.0015, angleDelta.y());
    const QPointF focus = event->position();
    const QPointF before = (focus - QPointF(width() * 0.5, height() * 0.5) - m_contentOffset) / oldZoom;

    setZoom(m_zoom * zoomStep);

    const QPointF afterOffset = focus - QPointF(width() * 0.5, height() * 0.5) - before * m_zoom;
    setContentOffset(afterOffset);
    event->accept();
}

void CanvasItem::updateInteractionState()
{
    setAcceptedMouseButtons(m_interactive ? Qt::LeftButton : Qt::NoButton);
    setAcceptHoverEvents(m_interactive);
}
