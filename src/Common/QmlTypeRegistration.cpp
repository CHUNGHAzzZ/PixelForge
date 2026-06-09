#include "Common/QmlTypeRegistration.h"

#include "Canvas/CanvasItem.h"

#include <qqml.h>

namespace PixelForge {

void registerQmlTypes()
{
    qmlRegisterType<CanvasItem>("PixelForge.Canvas", 1, 0, "CanvasView");
}

}
