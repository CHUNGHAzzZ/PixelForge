#pragma once

#include <QQuickFramebufferObject>

class CanvasItem : public QQuickFramebufferObject
{
    Q_OBJECT

public:
    explicit CanvasItem(QQuickItem *parent = nullptr);

    Renderer *createRenderer() const override;
};
