#include "CanvasTextureTile.h"

#include <QImage>
#include <QPainter>

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

void CanvasTextureTile::update(const CanvasTextureTileUpdateInfo &updateInfo)
{
    if (!m_functions || m_textureId == 0 || !updateInfo.isValid()) {
        return;
    }

    const QImage glImage = updateInfo.patchPixels.convertToFormat(QImage::Format_RGBA8888);
    const QPoint patchOffset = updateInfo.patchSceneRect.topLeft() - m_textureLayout.updateSceneRect.topLeft();
    const int topHeight = updateInfo.patchSceneRect.top() == updateInfo.currentSceneRect.top()
        ? std::max(0, patchOffset.y())
        : 0;
    const int leftWidth = updateInfo.patchSceneRect.left() == updateInfo.currentSceneRect.left()
        ? std::max(0, patchOffset.x())
        : 0;
    const int rightWidth = updateInfo.patchSceneRect.right() == updateInfo.currentSceneRect.right()
        ? std::max(0, m_textureLayout.textureSize.width() - patchOffset.x() - glImage.width())
        : 0;
    const int bottomHeight = updateInfo.patchSceneRect.bottom() == updateInfo.currentSceneRect.bottom()
        ? std::max(0, m_textureLayout.textureSize.height() - patchOffset.y() - glImage.height())
        : 0;

    QImage uploadImage = glImage;
    QPoint uploadOffset = patchOffset;
    if (topHeight > 0 || leftWidth > 0 || rightWidth > 0 || bottomHeight > 0) {
        uploadImage = QImage(
            leftWidth + glImage.width() + rightWidth,
            topHeight + glImage.height() + bottomHeight,
            QImage::Format_RGBA8888);
        uploadImage.fill(Qt::transparent);

        QPainter painter(&uploadImage);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
        painter.drawImage(QPoint(leftWidth, topHeight), glImage);

        if (topHeight > 0) {
            painter.drawImage(
                QRect(leftWidth, 0, glImage.width(), topHeight),
                glImage.copy(0, 0, glImage.width(), 1));
        }
        if (bottomHeight > 0) {
            painter.drawImage(
                QRect(leftWidth, topHeight + glImage.height(), glImage.width(), bottomHeight),
                glImage.copy(0, glImage.height() - 1, glImage.width(), 1));
        }
        if (leftWidth > 0) {
            painter.drawImage(
                QRect(0, topHeight, leftWidth, glImage.height()),
                glImage.copy(0, 0, 1, glImage.height()));
        }
        if (rightWidth > 0) {
            painter.drawImage(
                QRect(leftWidth + glImage.width(), topHeight, rightWidth, glImage.height()),
                glImage.copy(glImage.width() - 1, 0, 1, glImage.height()));
        }
        if (topHeight > 0 && leftWidth > 0) {
            painter.drawImage(QRect(0, 0, leftWidth, topHeight), glImage.copy(0, 0, 1, 1));
        }
        if (topHeight > 0 && rightWidth > 0) {
            painter.drawImage(QRect(leftWidth + glImage.width(), 0, rightWidth, topHeight), glImage.copy(glImage.width() - 1, 0, 1, 1));
        }
        if (bottomHeight > 0 && leftWidth > 0) {
            painter.drawImage(QRect(0, topHeight + glImage.height(), leftWidth, bottomHeight), glImage.copy(0, glImage.height() - 1, 1, 1));
        }
        if (bottomHeight > 0 && rightWidth > 0) {
            painter.drawImage(QRect(leftWidth + glImage.width(), topHeight + glImage.height(), rightWidth, bottomHeight), glImage.copy(glImage.width() - 1, glImage.height() - 1, 1, 1));
        }
        painter.end();
        uploadOffset -= QPoint(leftWidth, topHeight);
    }

    m_functions->glBindTexture(GL_TEXTURE_2D, m_textureId);
    m_functions->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if (m_allocatedTextureSize != m_textureLayout.textureSize) {
        QImage emptyTexture(m_textureLayout.textureSize, QImage::Format_RGBA8888);
        emptyTexture.fill(Qt::transparent);
        m_functions->glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            emptyTexture.width(),
            emptyTexture.height(),
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            emptyTexture.constBits());
        m_allocatedTextureSize = emptyTexture.size();
    }

    m_functions->glTexSubImage2D(
        GL_TEXTURE_2D,
        updateInfo.levelOfDetail,
        uploadOffset.x(),
        uploadOffset.y(),
        uploadImage.width(),
        uploadImage.height(),
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        uploadImage.constBits());
    m_functions->glGenerateMipmap(GL_TEXTURE_2D);
    m_functions->glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    clearDirty();
}

}
