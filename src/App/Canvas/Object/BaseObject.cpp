#include "BaseObject.h"

#include <QPainter>
#include <QtGlobal>

ObjectId BaseObject::id() const
{
    return m_id;
}

void BaseObject::setId(ObjectId id)
{
    m_id = id;
}

const QTransform &BaseObject::transform() const
{
    return m_transform;
}

void BaseObject::setTransform(const QTransform &transform)
{
    m_transform = transform;
}

void BaseObject::applyTransform(const QTransform &transform)
{
    m_transform = transform * m_transform;
}

void BaseObject::translate(const QPointF &offset)
{
    applyTransform(QTransform::fromTranslate(offset.x(), offset.y()));
}

void BaseObject::translate(qreal dx, qreal dy)
{
    translate(QPointF(dx, dy));
}

void BaseObject::scale(qreal factor, const QPointF &localOrigin)
{
    scale(factor, factor, localOrigin);
}

void BaseObject::scale(qreal sx, qreal sy, const QPointF &localOrigin)
{
    const QPointF canvasOrigin = localToCanvas(localOrigin);

    QTransform scaleTransform;
    scaleTransform.translate(canvasOrigin.x(), canvasOrigin.y());
    scaleTransform.scale(sx, sy);
    scaleTransform.translate(-canvasOrigin.x(), -canvasOrigin.y());

    applyTransform(scaleTransform);
}

void BaseObject::rotate(qreal degrees, const QPointF &localOrigin)
{
    const QPointF canvasOrigin = localToCanvas(localOrigin);

    QTransform rotation;
    rotation.translate(canvasOrigin.x(), canvasOrigin.y());
    rotation.rotate(degrees);
    rotation.translate(-canvasOrigin.x(), -canvasOrigin.y());

    applyTransform(rotation);
}

QPointF BaseObject::position() const
{
    return m_transform.map(QPointF());
}

void BaseObject::setPosition(const QPointF &position)
{
    translate(position - this->position());
}

QPointF BaseObject::localToCanvas(const QPointF &localPoint) const
{
    return m_transform.map(localPoint);
}

QPointF BaseObject::canvasToLocal(const QPointF &canvasPoint) const
{
    bool invertible = false;
    const QTransform inverse = m_transform.inverted(&invertible);
    return invertible ? inverse.map(canvasPoint) : QPointF();
}

QRectF BaseObject::canvasBounds() const
{
    return m_transform.mapRect(localBounds());
}

void BaseObject::paint(QPainter &painter) const
{
    painter.save();
    painter.setTransform(m_transform, true);
    paintLocal(painter);
    painter.restore();
}
