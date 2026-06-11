#include "CanvasTileCache.h"

#include "App/Canvas/Render/CanvasTypes.hpp"
#include "Utils/Logger.h"

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
    m_columns = columns;
    m_rows = rows;
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

void CanvasTileCache::recalculateCache(const CanvasOpenGLUpdateInfoList &updateInfos)
{
    int updatedTileCount = 0;
    int updatedPatchCount = 0;
    for (const CanvasOpenGLUpdateInfo &updateInfo : updateInfos) {
        for (const CanvasTextureTileUpdateInfo &tileInfo : updateInfo.tileList) {
            CanvasTextureTile *tile = textureTile(tileInfo.column, tileInfo.row);
            if (!tile) {
                continue;
            }

            tile->update(tileInfo);
            ++updatedTileCount;
            ++updatedPatchCount;
        }
    }

    if (updatedTileCount > 0) {
        Logger::debug(formatLogMessage(
            "Recalculated canvas texture cache: updates=%d patches=%d",
            static_cast<int>(updateInfos.size()),
            updatedPatchCount));
    }
}

void CanvasTileCache::destroy()
{
    m_tiles.clear();
    m_cacheBounds = QRect();
    m_textureInfo = CanvasTileTextureInfo();
    m_columns = 0;
    m_rows = 0;
}

const std::vector<CanvasTextureTile> &CanvasTileCache::tiles() const
{
    return m_tiles;
}

const QRect &CanvasTileCache::cacheBounds() const
{
    return m_cacheBounds;
}

CanvasTextureTile *CanvasTileCache::textureTile(int column, int row)
{
    if (column < 0 || row < 0 || column >= m_columns || row >= m_rows) {
        return nullptr;
    }

    const size_t index = static_cast<size_t>(row * m_columns + column);
    if (index >= m_tiles.size()) {
        return nullptr;
    }

    return &m_tiles[index];
}

}
