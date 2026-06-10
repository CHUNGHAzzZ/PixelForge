#pragma once

#include "App/Canvas/Object/BaseObject.h"

#include <QBrush>
#include <QColor>
#include <QPainterPath>
#include <QPen>
#include <QString>

namespace PixelForge {

class PathObject final : public BaseObject
{
public:
    PathObject() = default;
    explicit PathObject(const QPainterPath &path);
    PathObject(ObjectId id, const QPainterPath &path);

    ObjectType type() const override;

    const QPainterPath &path() const;
    void setPath(const QPainterPath &path);
    bool isEmpty() const;

    QColor fillColor() const;
    void setFillColor(const QColor &color);

    QColor strokeColor() const;
    void setStrokeColor(const QColor &color);

    qreal strokeWidth() const;
    void setStrokeWidth(qreal width);

    QString sourcePath() const;
    void setSourcePath(const QString &path);

    std::uint64_t geometryRevision() const;

    std::unique_ptr<BaseObject> clone() const override;
    QRectF localBounds() const override;

protected:
    void paintLocal(QPainter &painter) const override;

private:
    void bumpGeometryRevision();

    QPainterPath m_path;
    QColor m_fillColor {Qt::transparent};
    QColor m_strokeColor {Qt::black};
    qreal m_strokeWidth {1.0};
    QString m_sourcePath;
    std::uint64_t m_geometryRevision {1};
};

}
