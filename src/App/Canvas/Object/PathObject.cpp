#include "PathObject.h"

#include <QPainter>

#include <algorithm>

namespace PixelForge {

PathObject::PathObject(const QPainterPath &path)
    : m_path(path)
{
}

PathObject::PathObject(ObjectId id, const QPainterPath &path)
    : m_path(path)
{
    setId(id);
}

ObjectType PathObject::type() const
{
    return ObjectType::Path;
}

const QPainterPath &PathObject::path() const
{
    return m_path;
}

void PathObject::setPath(const QPainterPath &path)
{
    m_path = path;
    bumpGeometryRevision();
}

bool PathObject::isEmpty() const
{
    return m_path.isEmpty();
}

QColor PathObject::fillColor() const
{
    return m_fillColor;
}

void PathObject::setFillColor(const QColor &color)
{
    m_fillColor = color.isValid() ? color : QColor(Qt::transparent);
}

QColor PathObject::strokeColor() const
{
    return m_strokeColor;
}

void PathObject::setStrokeColor(const QColor &color)
{
    m_strokeColor = color.isValid() ? color : QColor(Qt::black);
}

qreal PathObject::strokeWidth() const
{
    return m_strokeWidth;
}

void PathObject::setStrokeWidth(qreal width)
{
    m_strokeWidth = std::max<qreal>(0.0, width);
}

QString PathObject::sourcePath() const
{
    return m_sourcePath;
}

void PathObject::setSourcePath(const QString &path)
{
    m_sourcePath = path;
}

std::uint64_t PathObject::geometryRevision() const
{
    return m_geometryRevision;
}

std::unique_ptr<BaseObject> PathObject::clone() const
{
    return std::make_unique<PathObject>(*this);
}

QRectF PathObject::localBounds() const
{
    const qreal strokePadding = m_strokeWidth * 0.5;
    return m_path.boundingRect().adjusted(-strokePadding, -strokePadding, strokePadding, strokePadding);
}

void PathObject::paintLocal(QPainter &painter) const
{
    if (m_path.isEmpty()) {
        return;
    }

    QPen pen(m_strokeColor, m_strokeWidth);
    pen.setCosmetic(false);
    if (m_strokeWidth > 0.0) {
        painter.setPen(pen);
    } else {
        painter.setPen(Qt::NoPen);
    }
    if (m_fillColor.alpha() > 0) {
        painter.setBrush(QBrush(m_fillColor));
    } else {
        painter.setBrush(Qt::NoBrush);
    }
    painter.drawPath(m_path);
}

void PathObject::bumpGeometryRevision()
{
    ++m_geometryRevision;
    if (m_geometryRevision == 0) {
        m_geometryRevision = 1;
    }
}

}
