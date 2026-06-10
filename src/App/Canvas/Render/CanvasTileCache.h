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
    void updateDirtyTiles(
        const CanvasObject &canvas,
        const CanvasObjectList &objects,
        ObjectId selectedObjectId);
    void destroy();

    const std::vector<CanvasTextureTile> &tiles() const;
    const QRect &cacheBounds() const;

private:
    void uploadTile(
        CanvasTextureTile &tile,
        const CanvasObject &canvas,
        const CanvasObjectList &objects,
        ObjectId selectedObjectId);

    QRect m_cacheBounds;
    CanvasTileTextureInfo m_textureInfo;
    std::vector<CanvasTextureTile> m_tiles;
};

}
