#include "CanvasUpdatesCompressor.h"

#include <QMutexLocker>

namespace PixelForge {

bool CanvasUpdatesCompressor::putUpdateInfo(const CanvasDirtyUpdateInfo &info)
{
    if (!info.isValid()) {
        return false;
    }

    QMutexLocker locker(&m_mutex);
    if (info.compressible) {
        auto it = m_updates.begin();
        while (it != m_updates.end()) {
            if (it->compressible &&
                it->levelOfDetail == info.levelOfDetail &&
                info.dirtySceneRect.contains(it->dirtySceneRect)) {
                it = m_updates.erase(it);
            } else {
                ++it;
            }
        }
    }

    m_updates.push_back(info);
    return m_updates.size() <= 1;
}

CanvasDirtyUpdateInfoList CanvasUpdatesCompressor::takeUpdateInfo()
{
    QMutexLocker locker(&m_mutex);
    CanvasDirtyUpdateInfoList updates;
    updates.swap(m_updates);
    return updates;
}

void CanvasUpdatesCompressor::clear()
{
    QMutexLocker locker(&m_mutex);
    m_updates.clear();
}

}
