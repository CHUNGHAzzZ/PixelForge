#include "Utils/QmlTypeRegistration.h"

#include "App/Canvas/Render/CanvasRender.h"
#include "Utils/Logger.h"

#include <qqml.h>

namespace PixelForge {

void registerQmlTypes()
{
    qmlRegisterType<CanvasRender>("PixelForge.Canvas", 1, 0, "CanvasRender");
    Logger::info("Registered QML type PixelForge.Canvas/CanvasRender");
}

}
