#pragma once

#include "App/Canvas/Render/CanvasTypes.hpp"

#include <QMutex>

namespace PixelForge {

class CanvasUpdatesCompressor
{
public:
    bool putUpdateInfo(const CanvasDirtyUpdateInfo &info);
    CanvasDirtyUpdateInfoList takeUpdateInfo();
    void clear();

private:
    QMutex m_mutex;
    CanvasDirtyUpdateInfoList m_updates;
};

}
