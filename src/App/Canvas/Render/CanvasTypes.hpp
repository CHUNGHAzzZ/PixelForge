#pragma once

#include "App/Canvas/Object/BaseObject.h"
#include "App/Canvas/Object/CanvasObject.h"

#include <QRect>
#include <QRectF>
#include <QSize>
#include <QTransform>
#include <QImage>

#include <memory>
#include <QString>
#include <vector>

namespace PixelForge {

inline constexpr int CanvasDefaultTileEffectiveSize = 256;
inline constexpr int CanvasDefaultTileBorder = 1;
inline constexpr int CanvasDefaultTileSize = CanvasDefaultTileEffectiveSize;

using CanvasObjectList = std::vector<std::unique_ptr<BaseObject>>;
using SceneDirtyRectList = std::vector<QRectF>;

struct CanvasDirtyUpdateInfo
{
    QRect dirtySceneRect;
    int levelOfDetail {0};
    bool compressible {true};

    bool isValid() const
    {
        return !dirtySceneRect.isEmpty();
    }
};

using CanvasDirtyUpdateInfoList = std::vector<CanvasDirtyUpdateInfo>;

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

struct CanvasTextureTileUpdateInfo
{
    int column {0};
    int row {0};
    QRect effectiveTileSceneRect;
    QRect physicalTileSceneRect;
    QRect patchSceneRect;
    QRect currentSceneRect;
    QImage patchPixels;
    int levelOfDetail {0};
    QString colorSpace {QStringLiteral("RGBA8888")};

    bool isValid() const
    {
        return !patchSceneRect.isEmpty() && !patchPixels.isNull();
    }

    QPoint patchOffset() const
    {
        return patchSceneRect.topLeft() - physicalTileSceneRect.topLeft();
    }

    QSize patchSize() const
    {
        return patchSceneRect.size();
    }

    QSize physicalTileSize() const
    {
        return physicalTileSceneRect.size();
    }

    bool isEntireTileUpdated() const
    {
        return patchSceneRect == physicalTileSceneRect;
    }
};

using CanvasTextureTileUpdateInfoList = std::vector<CanvasTextureTileUpdateInfo>;

struct CanvasOpenGLUpdateInfo
{
    QRect dirtySceneRect;
    int levelOfDetail {0};
    CanvasTextureTileUpdateInfoList tileList;

    bool isValid() const
    {
        return !dirtySceneRect.isEmpty() && !tileList.empty();
    }
};

using CanvasOpenGLUpdateInfoList = std::vector<CanvasOpenGLUpdateInfo>;

struct CanvasImageRenderSnapshot
{
    ObjectId id {InvalidObjectId};
    QTransform transform;
    QRectF localBounds;
    QRectF canvasBounds;
    QImage image;
    qint64 contentKey {0};

    bool isValid() const
    {
        return id != InvalidObjectId && !image.isNull() && !localBounds.isEmpty();
    }
};

using CanvasImageRenderSnapshotList = std::vector<CanvasImageRenderSnapshot>;

struct CanvasRenderState
{
    CanvasObject canvas;
    CanvasImageRenderSnapshotList imageSnapshots;
    CanvasOpenGLUpdateInfoList openGLUpdateInfos;
    QRect cacheBounds;
    QTransform viewportTransform;
    ObjectId selectedObjectId {InvalidObjectId};
    bool allDirty {true};
};

}
