#include "CanvasObject.h"

#include <QPainter>
#include <QPen>
#include <QtGlobal>

#include <algorithm>
#include <cmath>

namespace PixelForge {

ObjectType CanvasObject::type() const
{
    return ObjectType::Canvas;
}

QSizeF CanvasObject::size() const
{
    return m_size;
}

void CanvasObject::setSize(const QSizeF &size)
{
    m_size = QSizeF(std::max<qreal>(1.0, size.width()), std::max<qreal>(1.0, size.height()));
}

QRectF CanvasObject::localBounds() const
{
    return QRectF(QPointF(), m_size);
}

QRectF CanvasObject::transformedBounds() const
{
    return transform().mapRect(localBounds());
}

int CanvasObject::gridSize() const
{
    return m_gridSize;
}

void CanvasObject::setGridSize(int size)
{
    m_gridSize = std::clamp(size, 1, 512);
}

bool CanvasObject::gridVisible() const
{
    return m_gridVisible;
}

void CanvasObject::setGridVisible(bool visible)
{
    m_gridVisible = visible;
}

QColor CanvasObject::backgroundColor() const
{
    return m_backgroundColor;
}

void CanvasObject::setBackgroundColor(const QColor &color)
{
    if (color.isValid()) {
        m_backgroundColor = color;
    }
}

QColor CanvasObject::color() const
{
    return m_color;
}

void CanvasObject::setColor(const QColor &color)
{
    if (color.isValid()) {
        m_color = color;
    }
}

QColor CanvasObject::gridColorA() const
{
    return m_gridColorA;
}

void CanvasObject::setGridColorA(const QColor &color)
{
    if (color.isValid()) {
        m_gridColorA = color;
    }
}

QColor CanvasObject::gridColorB() const
{
    return m_gridColorB;
}

void CanvasObject::setGridColorB(const QColor &color)
{
    if (color.isValid()) {
        m_gridColorB = color;
    }
}

QColor CanvasObject::borderColor() const
{
    return m_borderColor;
}

void CanvasObject::setBorderColor(const QColor &color)
{
    if (color.isValid()) {
        m_borderColor = color;
    }
}

QPointF CanvasObject::localToViewport(const QPointF &localPoint) const
{
    return localToCanvas(localPoint);
}

QPointF CanvasObject::viewportToLocal(const QPointF &viewportPoint) const
{
    return canvasToLocal(viewportPoint);
}

void CanvasObject::paintBoard(QPainter &painter) const
{
    paintLocal(painter);
}

std::unique_ptr<BaseObject> CanvasObject::clone() const
{
    return std::make_unique<CanvasObject>(*this);
}

void CanvasObject::paintLocal(QPainter &painter) const
{
    const QRectF bounds = localBounds();
    const qreal tileSize = std::max<qreal>(1.0, m_gridSize);

    painter.fillRect(bounds, m_color);

    if (m_gridVisible) {
        const int columns = static_cast<int>(std::ceil(bounds.width() / tileSize));
        const int rows = static_cast<int>(std::ceil(bounds.height() / tileSize));

        for (int row = 0; row < rows; ++row) {
            for (int column = 0; column < columns; ++column) {
                const QColor &color = ((row + column) % 2 == 0) ? m_gridColorA : m_gridColorB;
                const QRectF tile(
                    bounds.left() + column * tileSize,
                    bounds.top() + row * tileSize,
                    std::min<qreal>(tileSize, bounds.right() - (bounds.left() + column * tileSize)),
                    std::min<qreal>(tileSize, bounds.bottom() - (bounds.top() + row * tileSize)));

                if (tile.width() > 0.0 && tile.height() > 0.0) {
                    painter.fillRect(tile, color);
                }
            }
        }
    }

    QPen borderPen(m_borderColor, 1.0);
    borderPen.setCosmetic(true);
    painter.setPen(borderPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(bounds.adjusted(0.5, 0.5, -0.5, -0.5));
}

}
