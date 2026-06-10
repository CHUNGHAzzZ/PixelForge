#pragma once

#include "App/Canvas/Render/CanvasTileCache.h"
#include "App/Canvas/Render/CanvasTypes.hpp"

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QQuickFramebufferObject>
#include <QMatrix4x4>
#include <QSize>

#include <memory>

class QPointF;

namespace PixelForge {

class CanvasTextureTile;

class CanvasRenderer final : public QQuickFramebufferObject::Renderer, protected QOpenGLFunctions
{
public:
    ~CanvasRenderer() override;

    void synchronize(QQuickFramebufferObject *item) override;
    void render() override;
    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;

private:
    void ensureShader();
    void ensureGeometryBuffers();
    void ensureTileGeometryBuffers();
    void updateTiles();
    void drawTiles();
    void drawTile(const CanvasTextureTile &tile, int tileIndex);
    QMatrix4x4 sceneToClipMatrix() const;

    QSize m_size;
    CanvasRenderState m_state;
    CanvasTileCache m_tileCache;
    std::unique_ptr<QOpenGLShaderProgram> m_shader;
    QOpenGLVertexArrayObject m_vertexArrayObject;
    QOpenGLBuffer m_vertexBuffer {QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer m_texCoordBuffer {QOpenGLBuffer::VertexBuffer};
    QRect m_geometryCacheBounds;
    int m_geometryTileCount {0};
    bool m_geometryBuffersInitialized {false};
};

}
