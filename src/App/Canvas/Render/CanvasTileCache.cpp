#include "CanvasTileCache.h"

#include "App/Canvas/Object/BaseObject.h"
#include "App/Canvas/Object/CanvasObject.h"
#include "App/Canvas/Render/CanvasTypes.hpp"
#include "Utils/Logger.h"

#include <QImage>
#include <QPainter>
#include <QPen>
#include <QtGlobal>

#include <algorithm>
#include <cmath>

namespace {
QRect expandedCanvasRect(const QRectF &rect)
{
    return rect.normalized().adjusted(-2.0, -2.0, 2.0, 2.0).toAlignedRect();
}
}

namespace PixelForge {

void CanvasTileCache::ensureTiles(const QRect &cacheBounds, QOpenGLFunctions *functions)
{
    const QRect boundedCacheBounds(
        cacheBounds.left(),
        cacheBounds.top(),
        std::max(1, cacheBounds.width()),
        std::max(1, cacheBounds.height()));
    if (boundedCacheBounds == m_cacheBounds && !m_tiles.empty()) {
        return;
    }

    destroy();
    m_cacheBounds = boundedCacheBounds;
    m_textureInfo.effectiveWidth = CanvasDefaultTileEffectiveSize;
    m_textureInfo.effectiveHeight = CanvasDefaultTileEffectiveSize;
    m_textureInfo.border = CanvasDefaultTileBorder;

    const int columns = (m_cacheBounds.width() + m_textureInfo.effectiveWidth - 1) / m_textureInfo.effectiveWidth;
    const int rows = (m_cacheBounds.height() + m_textureInfo.effectiveHeight - 1) / m_textureInfo.effectiveHeight;
    m_tiles.reserve(static_cast<size_t>(columns * rows));

    Logger::info(formatLogMessage(
        "Rebuilding canvas tile cache: bounds=%d,%d %dx%d effective=%dx%d border=%d columns=%d rows=%d",
        m_cacheBounds.left(),
        m_cacheBounds.top(),
        m_cacheBounds.width(),
        m_cacheBounds.height(),
        m_textureInfo.effectiveWidth,
        m_textureInfo.effectiveHeight,
        m_textureInfo.border,
        columns,
        rows));

    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            const int x = m_cacheBounds.left() + column * m_textureInfo.effectiveWidth;
            const int y = m_cacheBounds.top() + row * m_textureInfo.effectiveHeight;
            const int right = m_cacheBounds.left() + m_cacheBounds.width();
            const int bottom = m_cacheBounds.top() + m_cacheBounds.height();
            const QRect sceneRect(
                x,
                y,
                std::min(m_textureInfo.effectiveWidth, right - x),
                std::min(m_textureInfo.effectiveHeight, bottom - y));
            CanvasTextureTile tile(sceneRect);
            tile.setTextureLayout(m_textureInfo.layoutForSceneRect(sceneRect));
            tile.create(functions);
            m_tiles.push_back(std::move(tile));
        }
    }
}

void CanvasTileCache::markAllDirty()
{
    for (auto &tile : m_tiles) {
        tile.markDirty();
    }
}

void CanvasTileCache::markDirty(const QRectF &sceneRect)
{
    const QRect dirtyRect = expandedCanvasRect(sceneRect);
    if (dirtyRect.isEmpty()) {
        return;
    }

    for (auto &tile : m_tiles) {
        if (tile.textureLayout().updateSceneRect.intersects(dirtyRect)) {
            tile.markDirty();
        }
    }
}

void CanvasTileCache::markDirty(const SceneDirtyRectList &sceneRects)
{
    for (const QRectF &rect : sceneRects) {
        markDirty(rect);
    }
}

void CanvasTileCache::updateDirtyTiles(
    const CanvasObject &canvas,
    const CanvasObjectList &objects,
    ObjectId selectedObjectId)
{
    int updatedTileCount = 0;
    for (auto &tile : m_tiles) {
        if (tile.isDirty()) {
            uploadTile(tile, canvas, objects, selectedObjectId);
            ++updatedTileCount;
        }
    }

    if (updatedTileCount > 0) {
        Logger::debug(formatLogMessage("Updated %d dirty canvas texture tiles", updatedTileCount));
    }
}

void CanvasTileCache::destroy()
{
    m_tiles.clear();
    m_cacheBounds = QRect();
    m_textureInfo = CanvasTileTextureInfo();
}

const std::vector<CanvasTextureTile> &CanvasTileCache::tiles() const
{
    return m_tiles;
}

const QRect &CanvasTileCache::cacheBounds() const
{
    return m_cacheBounds;
}

void CanvasTileCache::uploadTile(
    CanvasTextureTile &tile,
    const CanvasObject &canvas,
    const CanvasObjectList &objects,
    ObjectId selectedObjectId)
{
    const CanvasTileTextureLayout &layout = tile.textureLayout();
    QImage image(layout.textureSize, QImage::Format_RGBA8888);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.translate(layout.border, layout.border);
    painter.translate(-layout.sceneRect.topLeft());
    painter.setClipRect(layout.updateSceneRect);

    canvas.paintBoard(painter);
    for (const auto &object : objects) {
        if (!object || !object->canvasBounds().intersects(layout.updateSceneRect)) {
            continue;
        }

        object->paint(painter);
    }

    for (const auto &object : objects) {
        if (!object || object->id() != selectedObjectId || !object->canvasBounds().intersects(layout.updateSceneRect)) {
            continue;
        }

        QPen selectionPen(QColor(QStringLiteral("#4B7BEC")), 1.5);
        selectionPen.setCosmetic(true);
        painter.setPen(selectionPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(object->canvasBounds().adjusted(0.75, 0.75, -0.75, -0.75));
    }
    painter.end();

    tile.upload(image, layout.contentTextureRect);
}

}
