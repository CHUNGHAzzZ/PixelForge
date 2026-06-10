#pragma once

#include "App/Canvas/Object/BaseObject.h"
#include "App/Canvas/Object/CanvasObject.h"

#include <QRect>
#include <QRectF>
#include <QSize>
#include <QTransform>

#include <memory>
#include <vector>

namespace PixelForge {

inline constexpr int CanvasDefaultTileEffectiveSize = 256;
inline constexpr int CanvasDefaultTileBorder = 1;
inline constexpr int CanvasDefaultTileSize = CanvasDefaultTileEffectiveSize;

using CanvasObjectList = std::vector<std::unique_ptr<BaseObject>>;
using SceneDirtyRectList = std::vector<QRectF>;

struct CanvasTileTextureLayout
{
    QRect sceneRect;
    QRect updateSceneRect;
    QRect textureRect;
    QRect contentTexturePixelRect;
    QRectF contentTextureRect;
    QSize textureSize;
    QSize effectiveTextureSize;
    int border {CanvasDefaultTileBorder};
};

struct CanvasTileTextureInfo
{
    int effectiveWidth {CanvasDefaultTileEffectiveSize};
    int effectiveHeight {CanvasDefaultTileEffectiveSize};
    int border {CanvasDefaultTileBorder};

    CanvasTileTextureLayout layoutForSceneRect(const QRect &sceneRect) const
    {
        const QSize effectiveSize(
            std::max(1, sceneRect.width()),
            std::max(1, sceneRect.height()));
        const QSize textureSize(
            effectiveSize.width() + border * 2,
            effectiveSize.height() + border * 2);

        CanvasTileTextureLayout layout;
        layout.sceneRect = sceneRect;
        layout.updateSceneRect = sceneRect.adjusted(-border, -border, border, border);
        layout.textureSize = textureSize;
        layout.effectiveTextureSize = effectiveSize;
        layout.textureRect = QRect(QPoint(), textureSize);
        layout.contentTexturePixelRect = QRect(QPoint(border, border), effectiveSize);
        layout.border = border;
        layout.contentTextureRect = QRectF(
            static_cast<qreal>(border) / textureSize.width(),
            static_cast<qreal>(border) / textureSize.height(),
            static_cast<qreal>(effectiveSize.width()) / textureSize.width(),
            static_cast<qreal>(effectiveSize.height()) / textureSize.height());
        return layout;
    }
};

struct CanvasRenderState
{
    CanvasObject canvas;
    CanvasObjectList objects;
    SceneDirtyRectList dirtySceneRects;
    QTransform viewportTransform;
    ObjectId selectedObjectId {InvalidObjectId};
    bool allDirty {true};
};

}
