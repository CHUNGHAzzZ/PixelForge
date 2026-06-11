#pragma once

#include "App/Canvas/Render/CanvasTypes.hpp"

namespace PixelForge {

class CanvasOpenGLUpdateInfoBuilder
{
public:
    CanvasOpenGLUpdateInfo buildUpdateInfo(
        const CanvasDirtyUpdateInfo &dirtyInfo,
        const CanvasObject &canvas,
        const CanvasObjectList &objects,
        ObjectId selectedObjectId) const;

    QRect calculateEffectiveTileRect(int column, int row, const QRect &sceneBounds) const;
    QRect calculatePhysicalTileRect(int column, int row, const QRect &sceneBounds, int levelOfDetail) const;
    QRect calculateCacheBounds(const CanvasObject &canvas, const CanvasObjectList &objects) const;
    int xToColumn(int x) const;
    int yToRow(int y) const;

    void setTextureBorder(int border);
    void setEffectiveTextureSize(const QSize &size);

private:
    QImage renderPatch(
        const QRect &patchSceneRect,
        const CanvasObject &canvas,
        const CanvasObjectList &objects,
        ObjectId selectedObjectId) const;

    QSize m_effectiveTextureSize {CanvasDefaultTileEffectiveSize, CanvasDefaultTileEffectiveSize};
    int m_textureBorder {CanvasDefaultTileBorder};
};

}
