#pragma once

#include "App/Canvas/Object/BaseObject.h"

#include <QColor>
#include <QSizeF>

namespace PixelForge {

class CanvasObject final : public BaseObject
{
public:
    ObjectType type() const override;

    QSizeF size() const;
    void setSize(const QSizeF &size);

    QRectF localBounds() const override;
    QRectF transformedBounds() const;

    int gridSize() const;
    void setGridSize(int size);

    bool gridVisible() const;
    void setGridVisible(bool visible);

    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &color);

    QColor color() const;
    void setColor(const QColor &color);

    QColor gridColorA() const;
    void setGridColorA(const QColor &color);

    QColor gridColorB() const;
    void setGridColorB(const QColor &color);

    QColor borderColor() const;
    void setBorderColor(const QColor &color);

    QPointF localToViewport(const QPointF &localPoint) const;
    QPointF viewportToLocal(const QPointF &viewportPoint) const;

    void paintBoard(QPainter &painter) const;
    std::unique_ptr<BaseObject> clone() const override;

protected:
    void paintLocal(QPainter &painter) const override;

private:
    QSizeF m_size {1024.0, 768.0};
    int m_gridSize {16};
    bool m_gridVisible {true};
    QColor m_backgroundColor {QStringLiteral("#F1F0F2")};
    QColor m_color {QStringLiteral("#FFFFFF")};
    QColor m_gridColorA {QStringLiteral("#F1F0F2")};
    QColor m_gridColorB {QStringLiteral("#F1F0F2")};
    QColor m_borderColor {QStringLiteral("#F1F0F2")};
};

}
