#pragma once

#include "App/Canvas/Render/CanvasTextureTile.h"
#include "App/Canvas/Render/CanvasTypes.hpp"

#include <QRect>
#include <QRectF>

#include <memory>
#include <vector>

class QOpenGLFunctions;

namespace PixelForge {

class CanvasTileCache
{
public:
    void ensureTiles(const QRect &cacheBounds, QOpenGLFunctions *functions);
    void markAllDirty();
    void markDirty(const QRectF &sceneRect);
    void markDirty(const SceneDirtyRectList &sceneRects);
    void recalculateCache(const CanvasOpenGLUpdateInfoList &updateInfos);
    void destroy();

    const std::vector<CanvasTextureTile> &tiles() const;
    const QRect &cacheBounds() const;

private:
    CanvasTextureTile *textureTile(int column, int row);

    QRect m_cacheBounds;
    CanvasTileTextureInfo m_textureInfo;
    std::vector<CanvasTextureTile> m_tiles;
    int m_columns {0};
    int m_rows {0};
};

}
