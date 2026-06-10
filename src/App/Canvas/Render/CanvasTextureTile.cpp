#include "CanvasTextureTile.h"

#include <QImage>

#include <utility>

namespace PixelForge {

CanvasTextureTile::CanvasTextureTile(const QRect &sceneRect)
    : m_sceneRect(sceneRect)
{
    CanvasTileTextureInfo defaultTextureInfo;
    m_textureLayout = defaultTextureInfo.layoutForSceneRect(sceneRect);
}

CanvasTextureTile::~CanvasTextureTile()
{
    destroy();
}

CanvasTextureTile::CanvasTextureTile(CanvasTextureTile &&other) noexcept
{
    *this = std::move(other);
}

CanvasTextureTile &CanvasTextureTile::operator=(CanvasTextureTile &&other) noexcept
{
    if (this == &other) {
        return *this;
    }

    destroy();
    m_sceneRect = other.m_sceneRect;
    m_textureLayout = other.m_textureLayout;
    m_allocatedTextureSize = other.m_allocatedTextureSize;
    m_textureId = other.m_textureId;
    m_dirty = other.m_dirty;
    m_functions = other.m_functions;

    other.m_textureLayout = CanvasTileTextureLayout();
    other.m_allocatedTextureSize = QSize();
    other.m_textureId = 0;
    other.m_functions = nullptr;
    other.m_dirty = true;
    return *this;
}

const QRect &CanvasTextureTile::sceneRect() const
{
    return m_sceneRect;
}

const CanvasTileTextureLayout &CanvasTextureTile::textureLayout() const
{
    return m_textureLayout;
}

const QRectF &CanvasTextureTile::textureContentRect() const
{
    return m_textureLayout.contentTextureRect;
}

unsigned int CanvasTextureTile::textureId() const
{
    return m_textureId;
}

bool CanvasTextureTile::isDirty() const
{
    return m_dirty;
}

void CanvasTextureTile::markDirty()
{
    m_dirty = true;
}

void CanvasTextureTile::clearDirty()
{
    m_dirty = false;
}

void CanvasTextureTile::setTextureLayout(const CanvasTileTextureLayout &layout)
{
    m_textureLayout = layout;
    m_sceneRect = layout.sceneRect;
    markDirty();
}

void CanvasTextureTile::create(QOpenGLFunctions *functions)
{
    if (!functions || m_textureId != 0) {
        return;
    }

    m_functions = functions;
    m_functions->glGenTextures(1, &m_textureId);
    m_functions->glBindTexture(GL_TEXTURE_2D, m_textureId);
    m_functions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    m_functions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_functions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_functions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_functions->glBindTexture(GL_TEXTURE_2D, 0);
    markDirty();
}

void CanvasTextureTile::destroy()
{
    if (m_functions && m_textureId != 0) {
        m_functions->glDeleteTextures(1, &m_textureId);
    }

    m_textureId = 0;
    m_allocatedTextureSize = QSize();
    m_functions = nullptr;
    m_dirty = true;
}

void CanvasTextureTile::upload(const QImage &image, const QRectF &textureContentRect)
{
    if (!m_functions || m_textureId == 0 || image.isNull()) {
        return;
    }

    m_textureLayout.contentTextureRect = textureContentRect;
    const QImage glImage = image.convertToFormat(QImage::Format_RGBA8888);
    m_functions->glBindTexture(GL_TEXTURE_2D, m_textureId);
    m_functions->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    if (m_allocatedTextureSize != glImage.size()) {
        m_functions->glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            glImage.width(),
            glImage.height(),
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            glImage.constBits());
        m_allocatedTextureSize = glImage.size();
    } else {
        m_functions->glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            0,
            0,
            glImage.width(),
            glImage.height(),
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            glImage.constBits());
    }
    m_functions->glGenerateMipmap(GL_TEXTURE_2D);
    m_functions->glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    clearDirty();
}

}
