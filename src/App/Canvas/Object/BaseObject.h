#pragma once

#include "App/Canvas/Object/ObjectTypes.hpp"

#include <QPointF>
#include <QRectF>
#include <QTransform>

#include <memory>

class QPainter;

class BaseObject
{
public:
    virtual ~BaseObject() = default;

    ObjectId id() const;
    void setId(ObjectId id);

    virtual ObjectType type() const = 0;

    const QTransform &transform() const;
    void setTransform(const QTransform &transform);

    void applyTransform(const QTransform &transform);
    void translate(const QPointF &offset);
    void translate(qreal dx, qreal dy);
    void scale(qreal factor, const QPointF &localOrigin = QPointF());
    void scale(qreal sx, qreal sy, const QPointF &localOrigin = QPointF());
    void rotate(qreal degrees, const QPointF &localOrigin = QPointF());

    QPointF position() const;
    void setPosition(const QPointF &position);

    QPointF localToCanvas(const QPointF &localPoint) const;
    QPointF canvasToLocal(const QPointF &canvasPoint) const;
    QRectF canvasBounds() const;

    void paint(QPainter &painter) const;
    virtual std::unique_ptr<BaseObject> clone() const = 0;
    virtual QRectF localBounds() const = 0;

protected:
    virtual void paintLocal(QPainter &painter) const = 0;

private:
    ObjectId m_id {InvalidObjectId};
    QTransform m_transform;
};
