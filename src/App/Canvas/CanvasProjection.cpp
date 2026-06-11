#include "CanvasProjection.h"

#include "App/Canvas/Object/ImageObject.h"

namespace PixelForge {

QRect CanvasProjection::cacheBounds(const CanvasDocument &document) const
{
    return m_updateInfoBuilder.calculateCacheBounds(document.canvas(), document.objects());
}

CanvasOpenGLUpdateInfoList CanvasProjection::consumeOpenGLUpdateInfos(CanvasDocument &document) const
{
    CanvasDirtyUpdateInfoList dirtyInfos;
    if (document.allDirty()) {
        CanvasDirtyUpdateInfo dirtyInfo;
        dirtyInfo.dirtySceneRect = cacheBounds(document);
        dirtyInfo.levelOfDetail = 0;
        dirtyInfo.compressible = true;
        dirtyInfos.push_back(dirtyInfo);
    } else {
        dirtyInfos = document.takeDirtyUpdateInfos();
    }

    CanvasOpenGLUpdateInfoList updateInfos;
    updateInfos.reserve(dirtyInfos.size());
    for (const CanvasDirtyUpdateInfo &dirtyInfo : dirtyInfos) {
        CanvasOpenGLUpdateInfo updateInfo = m_updateInfoBuilder.buildUpdateInfo(
            dirtyInfo,
            document.canvas(),
            document.objects(),
            document.selectedObjectId());
        if (updateInfo.isValid()) {
            updateInfos.push_back(std::move(updateInfo));
        }
    }

    document.clearAllDirtyFlag();
    return updateInfos;
}

CanvasImageRenderSnapshotList CanvasProjection::imageRenderSnapshots(const CanvasDocument &document) const
{
    CanvasImageRenderSnapshotList snapshots;

    for (const auto &object : document.objects()) {
        if (!object || object->type() != ObjectType::Image) {
            continue;
        }

        const auto *imageObject = dynamic_cast<const ImageObject *>(object.get());
        if (!imageObject || imageObject->image().isNull()) {
            continue;
        }

        CanvasImageRenderSnapshot snapshot;
        snapshot.id = imageObject->id();
        snapshot.transform = imageObject->transform();
        snapshot.localBounds = imageObject->localBounds();
        snapshot.canvasBounds = imageObject->canvasBounds();
        snapshot.image = imageObject->image();
        snapshot.contentKey = snapshot.image.cacheKey();
        snapshots.push_back(std::move(snapshot));
    }

    return snapshots;
}

}
