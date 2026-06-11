#pragma once

#include "App/Canvas/Render/CanvasTileCache.h"
#include "App/Canvas/Render/CanvasTypes.hpp"

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QQuickFramebufferObject>
#include <QMatrix4x4>
#include <QRectF>
#include <QSize>

#include <unordered_map>
#include <memory>

class QPointF;

namespace PixelForge {

class BaseObject;
class CanvasTextureTile;
class ImageObject;

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
    void drawImageObjects();
    void drawImageObject(const BaseObject &object);
    void uploadImageObjectTexture(const ImageObject &object);
    void pruneImageTextureCache();
    QMatrix4x4 sceneToClipMatrix() const;
    QMatrix4x4 objectToClipMatrix(const BaseObject &object) const;

    struct ImageTexture
    {
        unsigned int textureId {0};
        QSize size;
        QRectF localBounds;
        bool geometryDirty {true};
    };

    QSize m_size;
    CanvasRenderState m_state;
    CanvasTileCache m_tileCache;
    std::unique_ptr<QOpenGLShaderProgram> m_shader;
    QOpenGLVertexArrayObject m_vertexArrayObject;
    QOpenGLBuffer m_vertexBuffer {QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer m_texCoordBuffer {QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer m_imageVertexBuffer {QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer m_imageTexCoordBuffer {QOpenGLBuffer::VertexBuffer};
    QRect m_geometryCacheBounds;
    QRectF m_imageGeometryLocalBounds;
    int m_geometryTileCount {0};
    bool m_geometryBuffersInitialized {false};
    std::unordered_map<ObjectId, ImageTexture> m_imageTextures;
};

}
