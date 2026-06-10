#include "App/PixelForgeApplication.h"
#include "Utils/Logger.h"
#include "Utils/QmlTypeRegistration.h"

#include <QGuiApplication>
#include <QDebug>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QSurfaceFormat>
#include <QUrl>

#include <opencv2/core.hpp>

namespace {
void configureSurface()
{
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(4, 1);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);
}
}

int main(int argc, char *argv[])
{
    configureSurface();
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName(QStringLiteral("PixelForge"));
    QGuiApplication::setOrganizationName(QStringLiteral("PixelForge"));

    PixelForge::Logger::init();
    PixelForge::Logger::info("PixelForge application started");

    PixelForge::registerQmlTypes();

    PixelForge::PixelForgeApplication controller;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("appController"), &controller);
    engine.load(QUrl(QStringLiteral("qrc:/PixelForge/qml/Main.qml")));

    if (engine.rootObjects().isEmpty()) {
        PixelForge::Logger::fatal("Failed to load QML root object");
        PixelForge::Logger::shutdown();
        return -1;
    }

    const int result = app.exec();
    PixelForge::Logger::info("PixelForge application exited");
    PixelForge::Logger::shutdown();
    return result;
}
