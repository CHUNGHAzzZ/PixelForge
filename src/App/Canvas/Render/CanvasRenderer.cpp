#include "App/Canvas/Render/CanvasRenderer.h"

#include "App/Canvas/Render/CanvasRender.h"
#include "App/Canvas/Render/CanvasTextureTile.h"
#include "Utils/Logger.h"

#include <QFile>
#include <QMatrix4x4>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QPointF>
#include <QRectF>
#include <QSizeF>

#include <algorithm>
#include <cmath>
#include <vector>

namespace PixelForge {

namespace {
QString readResourceText(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Logger::error(formatLogMessage("Failed to open shader resource: %s", path.toUtf8().constData()));
        return {};
    }

    return QString::fromUtf8(file.readAll());
}

QRect cacheBoundsForState(const CanvasRenderState &state)
{
    QRectF bounds = state.canvas.canvasBounds();
    for (const auto &object : state.objects) {
        if (object) {
            bounds = bounds.united(object->canvasBounds());
        }
    }

    const QRect alignedBounds = bounds.adjusted(-8.0, -8.0, 8.0, 8.0).toAlignedRect();
    const int left = std::floor(static_cast<double>(alignedBounds.left()) / CanvasDefaultTileEffectiveSize) * CanvasDefaultTileEffectiveSize;
    const int top = std::floor(static_cast<double>(alignedBounds.top()) / CanvasDefaultTileEffectiveSize) * CanvasDefaultTileEffectiveSize;
    const int right = std::ceil(static_cast<double>(alignedBounds.right() + 1) / CanvasDefaultTileEffectiveSize) * CanvasDefaultTileEffectiveSize;
    const int bottom = std::ceil(static_cast<double>(alignedBounds.bottom() + 1) / CanvasDefaultTileEffectiveSize) * CanvasDefaultTileEffectiveSize;
    return QRect(left, top, std::max(1, right - left), std::max(1, bottom - top));
}
}

CanvasRenderer::~CanvasRenderer()
{
    m_tileCache.destroy();
    m_texCoordBuffer.destroy();
    m_vertexBuffer.destroy();
    m_vertexArrayObject.destroy();
}

void CanvasRenderer::synchronize(QQuickFramebufferObject *item)
{
    const auto *canvas = qobject_cast<CanvasRender *>(item);
    if (!canvas) {
        return;
    }

    m_state.canvas = canvas->canvasObject();
    m_state.objects = canvas->cloneObjects();
    m_state.viewportTransform = canvas->viewportTransform();
    m_state.selectedObjectId = canvas->selectedObjectId();
    bool allDirty = false;
    m_state.dirtySceneRects = const_cast<CanvasRender *>(canvas)->consumeDirtySceneRects(&allDirty);
    m_state.allDirty = allDirty;
}

void CanvasRenderer::render()
{
    initializeOpenGLFunctions();
    ensureShader();
    ensureGeometryBuffers();
    updateTiles();
    ensureTileGeometryBuffers();

    glViewport(0, 0, m_size.width(), m_size.height());
    glClearColor(
        m_state.canvas.backgroundColor().redF(),
        m_state.canvas.backgroundColor().greenF(),
        m_state.canvas.backgroundColor().blueF(),
        m_state.canvas.backgroundColor().alphaF());
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    drawTiles();
}

QOpenGLFramebufferObject *CanvasRenderer::createFramebufferObject(const QSize &size)
{
    m_size = size;

    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(4);
    return new QOpenGLFramebufferObject(size, format);
}

void CanvasRenderer::ensureShader()
{
    if (m_shader) {
        return;
    }

    auto shader = std::make_unique<QOpenGLShaderProgram>();
    const QString vertexShader = readResourceText(QStringLiteral(":/PixelForge/Shader/CanvasTile.vert"));
    const QString fragmentShader = readResourceText(QStringLiteral(":/PixelForge/Shader/CanvasTile.frag"));
    if (vertexShader.isEmpty() || fragmentShader.isEmpty()) {
        return;
    }

    if (!shader->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShader)) {
        Logger::error(formatLogMessage("Failed to compile canvas tile vertex shader: %s", shader->log().toUtf8().constData()));
        return;
    }

    if (!shader->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShader)) {
        Logger::error(formatLogMessage("Failed to compile canvas tile fragment shader: %s", shader->log().toUtf8().constData()));
        return;
    }

    shader->bindAttributeLocation("vertexPosition", 0);
    shader->bindAttributeLocation("textureCoord", 1);
    if (!shader->link()) {
        Logger::error(formatLogMessage("Failed to link canvas tile shader: %s", shader->log().toUtf8().constData()));
        return;
    }

    Logger::info("Canvas tile shader initialized");
    m_shader = std::move(shader);
}

void CanvasRenderer::ensureGeometryBuffers()
{
    if (m_geometryBuffersInitialized) {
        return;
    }

    if (!m_shader) {
        return;
    }

    m_vertexArrayObject.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vertexArrayObject);

    m_vertexBuffer.create();
    m_vertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);

    m_texCoordBuffer.create();
    m_texCoordBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);

    m_geometryBuffersInitialized = true;
}

void CanvasRenderer::ensureTileGeometryBuffers()
{
    if (!m_geometryBuffersInitialized || !m_shader) {
        return;
    }

    const int tileCount = static_cast<int>(m_tileCache.tiles().size());
    if (m_geometryCacheBounds == m_tileCache.cacheBounds() && m_geometryTileCount == tileCount) {
        return;
    }

    std::vector<GLfloat> vertices;
    std::vector<GLfloat> texCoords;
    vertices.reserve(static_cast<size_t>(tileCount) * 12);
    texCoords.reserve(static_cast<size_t>(tileCount) * 12);

    for (const CanvasTextureTile &tile : m_tileCache.tiles()) {
        const QRectF sceneRect(tile.sceneRect());
        vertices.insert(vertices.end(), {
            static_cast<GLfloat>(sceneRect.left()), static_cast<GLfloat>(sceneRect.top()),
            static_cast<GLfloat>(sceneRect.right()), static_cast<GLfloat>(sceneRect.top()),
            static_cast<GLfloat>(sceneRect.right()), static_cast<GLfloat>(sceneRect.bottom()),
            static_cast<GLfloat>(sceneRect.left()), static_cast<GLfloat>(sceneRect.top()),
            static_cast<GLfloat>(sceneRect.right()), static_cast<GLfloat>(sceneRect.bottom()),
            static_cast<GLfloat>(sceneRect.left()), static_cast<GLfloat>(sceneRect.bottom()),
        });

        const QRectF textureRect = tile.textureContentRect();
        texCoords.insert(texCoords.end(), {
            static_cast<GLfloat>(textureRect.left()), static_cast<GLfloat>(textureRect.top()),
            static_cast<GLfloat>(textureRect.right()), static_cast<GLfloat>(textureRect.top()),
            static_cast<GLfloat>(textureRect.right()), static_cast<GLfloat>(textureRect.bottom()),
            static_cast<GLfloat>(textureRect.left()), static_cast<GLfloat>(textureRect.top()),
            static_cast<GLfloat>(textureRect.right()), static_cast<GLfloat>(textureRect.bottom()),
            static_cast<GLfloat>(textureRect.left()), static_cast<GLfloat>(textureRect.bottom()),
        });
    }

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vertexArrayObject);
    m_vertexBuffer.bind();
    m_vertexBuffer.allocate(vertices.data(), static_cast<int>(vertices.size() * sizeof(GLfloat)));
    m_shader->enableAttributeArray(0);
    m_shader->setAttributeBuffer(0, GL_FLOAT, 0, 2);
    m_vertexBuffer.release();

    m_texCoordBuffer.bind();
    m_texCoordBuffer.allocate(texCoords.data(), static_cast<int>(texCoords.size() * sizeof(GLfloat)));
    m_shader->enableAttributeArray(1);
    m_shader->setAttributeBuffer(1, GL_FLOAT, 0, 2);
    m_texCoordBuffer.release();

    m_geometryCacheBounds = m_tileCache.cacheBounds();
    m_geometryTileCount = tileCount;
}

void CanvasRenderer::updateTiles()
{
    m_tileCache.ensureTiles(cacheBoundsForState(m_state), this);

    if (m_state.allDirty) {
        m_tileCache.markAllDirty();
    } else {
        m_tileCache.markDirty(m_state.dirtySceneRects);
    }

    m_tileCache.updateDirtyTiles(m_state.canvas, m_state.objects, m_state.selectedObjectId);
}

void CanvasRenderer::drawTiles()
{
    if (!m_shader || !m_shader->bind()) {
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glActiveTexture(GL_TEXTURE0);
    m_shader->setUniformValue("tileTexture", 0);
    m_shader->setUniformValue("modelViewProjection", sceneToClipMatrix());
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vertexArrayObject);

    const QRectF viewportRect {QPointF(), QSizeF(m_size)};
    const auto &tiles = m_tileCache.tiles();
    for (int tileIndex = 0; tileIndex < static_cast<int>(tiles.size()); ++tileIndex) {
        const CanvasTextureTile &tile = tiles[static_cast<size_t>(tileIndex)];
        const QRectF mappedRect = m_state.viewportTransform.mapRect(QRectF(tile.sceneRect()));
        if (!mappedRect.intersects(viewportRect)) {
            continue;
        }

        drawTile(tile, tileIndex);
    }

    m_shader->release();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
}

void CanvasRenderer::drawTile(const CanvasTextureTile &tile, int tileIndex)
{
    glBindTexture(GL_TEXTURE_2D, tile.textureId());
    const int vertexByteOffset = tileIndex * 6 * 2 * static_cast<int>(sizeof(GLfloat));
    m_vertexBuffer.bind();
    m_shader->setAttributeBuffer(0, GL_FLOAT, vertexByteOffset, 2);
    m_vertexBuffer.release();
    const int texCoordByteOffset = tileIndex * 6 * 2 * static_cast<int>(sizeof(GLfloat));
    m_texCoordBuffer.bind();
    m_shader->setAttributeBuffer(1, GL_FLOAT, texCoordByteOffset, 2);
    m_texCoordBuffer.release();
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

QMatrix4x4 CanvasRenderer::sceneToClipMatrix() const
{
    QMatrix4x4 projection;
    projection.ortho(
        0.0f,
        static_cast<float>(std::max(1, m_size.width())),
        static_cast<float>(std::max(1, m_size.height())),
        0.0f,
        -1.0f,
        1.0f);

    QMatrix4x4 model(m_state.viewportTransform);
    return projection * model;
}

}
