#pragma once

#include "App/Canvas/Render/CanvasTypes.hpp"

#include <QOpenGLFunctions>
#include <QRect>
#include <QRectF>
#include <QSize>

class QImage;

namespace PixelForge {

class CanvasTextureTile
{
public:
    CanvasTextureTile() = default;
    explicit CanvasTextureTile(const QRect &sceneRect);
    ~CanvasTextureTile();

    CanvasTextureTile(CanvasTextureTile &&other) noexcept;
    CanvasTextureTile &operator=(CanvasTextureTile &&other) noexcept;

    CanvasTextureTile(const CanvasTextureTile &) = delete;
    CanvasTextureTile &operator=(const CanvasTextureTile &) = delete;

    const QRect &sceneRect() const;
    const CanvasTileTextureLayout &textureLayout() const;
    const QRectF &textureContentRect() const;
    unsigned int textureId() const;

    bool isDirty() const;
    void markDirty();
    void clearDirty();

    void setTextureLayout(const CanvasTileTextureLayout &layout);
    void create(QOpenGLFunctions *functions);
    void destroy();
    void upload(const QImage &image, const QRectF &textureContentRect);

private:
    QRect m_sceneRect;
    CanvasTileTextureLayout m_textureLayout;
    QSize m_allocatedTextureSize;
    unsigned int m_textureId {0};
    bool m_dirty {true};
    QOpenGLFunctions *m_functions {nullptr};
};

}
