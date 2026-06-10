#include "Common/QmlTypeRegistration.h"

#include "App/Canvas/Render/CanvasItem.h"

#include <qqml.h>

namespace PixelForge {

void registerQmlTypes()
{
    qmlRegisterType<CanvasItem>("PixelForge.Canvas", 1, 0, "CanvasItem");
}

}
