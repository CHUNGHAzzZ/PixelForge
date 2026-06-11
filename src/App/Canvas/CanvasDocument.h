#pragma once

#include "App/Canvas/Object/CanvasObject.h"
#include "App/Canvas/Render/CanvasTypes.hpp"
#include "App/Canvas/Render/CanvasUpdatesCompressor.h"

#include <QRectF>

#include <memory>

namespace PixelForge {

class CanvasDocument
{
public:
    const CanvasObject &canvas() const;
    CanvasObject &canvas();
    const CanvasObjectList &objects() const;

    ObjectId selectedObjectId() const;
    void setSelectedObjectId(ObjectId id);
    bool bringObjectToFront(ObjectId id);

    ObjectId nextObjectId();
    BaseObject *objectById(ObjectId id);
    const BaseObject *objectById(ObjectId id) const;
    BaseObject *hitTestObject(const QPointF &canvasPoint);

    void clearObjects();
    void addObject(std::unique_ptr<BaseObject> object);

    bool allDirty() const;
    void markAllDirty();
    void markSceneDirty(const QRectF &sceneRect);
    void markObjectDirty(const QRectF &beforeBounds, const QRectF &afterBounds);
    CanvasDirtyUpdateInfoList takeDirtyUpdateInfos();
    void clearAllDirtyFlag();

private:
    CanvasObject m_canvas;
    CanvasObjectList m_objects;
    ObjectId m_selectedObjectId {InvalidObjectId};
    ObjectId m_nextObjectId {1};
    bool m_allDirty {true};
    CanvasUpdatesCompressor m_updatesCompressor;
};

}
