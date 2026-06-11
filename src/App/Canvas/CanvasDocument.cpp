#include "CanvasDocument.h"

#include "App/Canvas/Object/BaseObject.h"
#include "Utils/Logger.h"

#include <algorithm>

namespace PixelForge {

const CanvasObject &CanvasDocument::canvas() const
{
    return m_canvas;
}

CanvasObject &CanvasDocument::canvas()
{
    return m_canvas;
}

const CanvasObjectList &CanvasDocument::objects() const
{
    return m_objects;
}

ObjectId CanvasDocument::selectedObjectId() const
{
    return m_selectedObjectId;
}

void CanvasDocument::setSelectedObjectId(ObjectId id)
{
    m_selectedObjectId = id;
}

bool CanvasDocument::bringObjectToFront(ObjectId id)
{
    if (id == InvalidObjectId || m_objects.size() < 2) {
        return false;
    }

    auto it = std::find_if(m_objects.begin(), m_objects.end(), [id](const auto &object) {
        return object && object->id() == id;
    });
    if (it == m_objects.end() || std::next(it) == m_objects.end()) {
        return false;
    }

    std::unique_ptr<BaseObject> object = std::move(*it);
    m_objects.erase(it);
    m_objects.push_back(std::move(object));
    Logger::debug(formatLogMessage("Brought canvas object to front id=%llu", static_cast<unsigned long long>(id)));
    return true;
}

ObjectId CanvasDocument::nextObjectId()
{
    return m_nextObjectId++;
}

BaseObject *CanvasDocument::objectById(ObjectId id)
{
    if (id == InvalidObjectId) {
        return nullptr;
    }

    for (const auto &object : m_objects) {
        if (object && object->id() == id) {
            return object.get();
        }
    }

    return nullptr;
}

const BaseObject *CanvasDocument::objectById(ObjectId id) const
{
    if (id == InvalidObjectId) {
        return nullptr;
    }

    for (const auto &object : m_objects) {
        if (object && object->id() == id) {
            return object.get();
        }
    }

    return nullptr;
}

BaseObject *CanvasDocument::hitTestObject(const QPointF &canvasPoint)
{
    for (auto it = m_objects.rbegin(); it != m_objects.rend(); ++it) {
        BaseObject *object = it->get();
        if (!object || !object->canvasBounds().contains(canvasPoint)) {
            continue;
        }

        const QPointF localPoint = object->canvasToLocal(canvasPoint);
        if (object->localBounds().contains(localPoint)) {
            return object;
        }
    }

    return nullptr;
}

void CanvasDocument::clearObjects()
{
    for (const auto &object : m_objects) {
        if (object && object->type() != ObjectType::Image) {
            markSceneDirty(object->canvasBounds());
        }
    }

    m_objects.clear();
    m_selectedObjectId = InvalidObjectId;
    Logger::debug("Cleared canvas objects");
}

void CanvasDocument::addObject(std::unique_ptr<BaseObject> object)
{
    if (!object) {
        return;
    }

    if (object->id() == InvalidObjectId) {
        object->setId(nextObjectId());
    }

    if (object->type() != ObjectType::Image) {
        markSceneDirty(object->canvasBounds());
    }
    Logger::debug(formatLogMessage(
        "Added canvas object id=%llu type=%d",
        static_cast<unsigned long long>(object->id()),
        static_cast<int>(object->type())));
    m_objects.push_back(std::move(object));
}

bool CanvasDocument::allDirty() const
{
    return m_allDirty;
}

void CanvasDocument::markAllDirty()
{
    m_allDirty = true;
    m_updatesCompressor.clear();
}

void CanvasDocument::markSceneDirty(const QRectF &sceneRect)
{
    if (sceneRect.isEmpty()) {
        return;
    }

    CanvasDirtyUpdateInfo updateInfo;
    updateInfo.dirtySceneRect = sceneRect.normalized().toAlignedRect();
    updateInfo.levelOfDetail = 0;
    updateInfo.compressible = true;
    m_updatesCompressor.putUpdateInfo(updateInfo);
}

void CanvasDocument::markObjectDirty(const QRectF &beforeBounds, const QRectF &afterBounds)
{
    const QRectF dirtyBounds = beforeBounds.united(afterBounds).adjusted(-2.0, -2.0, 2.0, 2.0);
    markSceneDirty(dirtyBounds);
}

CanvasDirtyUpdateInfoList CanvasDocument::takeDirtyUpdateInfos()
{
    return m_updatesCompressor.takeUpdateInfo();
}

void CanvasDocument::clearAllDirtyFlag()
{
    m_allDirty = false;
}

}
