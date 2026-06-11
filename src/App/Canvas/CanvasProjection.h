#pragma once

#include "App/Canvas/CanvasDocument.h"
#include "App/Canvas/Render/CanvasOpenGLUpdateInfoBuilder.h"
#include "App/Canvas/Render/CanvasTypes.hpp"

namespace PixelForge {

class CanvasProjection
{
public:
    QRect cacheBounds(const CanvasDocument &document) const;
    CanvasOpenGLUpdateInfoList consumeOpenGLUpdateInfos(CanvasDocument &document) const;
    CanvasImageRenderSnapshotList imageRenderSnapshots(const CanvasDocument &document) const;

private:
    CanvasOpenGLUpdateInfoBuilder m_updateInfoBuilder;
};

}
