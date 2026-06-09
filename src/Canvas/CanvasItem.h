#pragma once

#include "Common/QmlProperty.hpp"

#include <QColor>
#include <QPointF>
#include <QQuickFramebufferObject>
#include <QSizeF>

class QMouseEvent;
class QWheelEvent;

class CanvasItem : public QQuickFramebufferObject
{
    Q_OBJECT

public:
    explicit CanvasItem(QQuickItem *parent = nullptr);

    Renderer *createRenderer() const override;

    PIXELFORGE_QML_QT_PROPERTY(QSizeF, documentSize, DocumentSize)
    PIXELFORGE_QML_VALUE_PROPERTY(qreal, zoom, Zoom)
    PIXELFORGE_QML_QT_PROPERTY(QPointF, contentOffset, ContentOffset)
    PIXELFORGE_QML_QT_PROPERTY(QColor, backgroundColor, BackgroundColor)
    PIXELFORGE_QML_QT_PROPERTY(QColor, canvasColor, CanvasColor)
    PIXELFORGE_QML_QT_PROPERTY(QColor, checkerColorA, CheckerColorA)
    PIXELFORGE_QML_QT_PROPERTY(QColor, checkerColorB, CheckerColorB)
    PIXELFORGE_QML_QT_PROPERTY(QColor, borderColor, BorderColor)
    PIXELFORGE_QML_VALUE_PROPERTY(bool, checkerboardVisible, CheckerboardVisible)
    PIXELFORGE_QML_VALUE_PROPERTY(int, checkerboardSize, CheckerboardSize)
    PIXELFORGE_QML_VALUE_PROPERTY(bool, interactive, Interactive)

    Q_INVOKABLE void resetView();

protected:
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void updateInteractionState();

    QSizeF m_documentSize {1024.0, 768.0};
    qreal m_zoom {1.0};
    QPointF m_contentOffset;
    QColor m_backgroundColor {QStringLiteral("#111315")};
    QColor m_canvasColor {QStringLiteral("#FFFFFF")};
    QColor m_checkerColorA {QStringLiteral("#F7F8FA")};
    QColor m_checkerColorB {QStringLiteral("#E1E4E8")};
    QColor m_borderColor {QStringLiteral("#30343A")};
    bool m_checkerboardVisible {true};
    int m_checkerboardSize {16};
    bool m_interactive {true};
    bool m_isPanning {false};
    QPointF m_lastPanPosition;
};
