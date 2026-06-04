# Third-Party Dependency Plan

PixelForge is now an empty QML + OpenGL canvas shell. Keep the dependency set small until the paint engine needs more.

## Required Now

- Qt 6 Core: application object model, event loop, properties, resources.
- Qt 6 Gui: windowing, input, OpenGL surface setup.
- Qt 6 Network: public dependency of Qt Qml.
- Qt 6 Qml: QML engine and C++ integration.
- Qt 6 QmlIntegration: public QML/C++ type integration headers.
- Qt 6 QmlModels: runtime dependency of Qt Quick.
- Qt 6 Quick: Qt Quick scene graph and QQuickFramebufferObject.
- Qt 6 OpenGL: OpenGL helper types and framebuffer integration.
- Windows system OpenGL: `opengl32.lib`.

## Required Qt Tools

- `moc.exe`: required for QObject metadata.
- `rcc.exe`: required because QML is packaged through `src/qml.qrc`.

Both tools should live in `3rdparty/qt6/win64/bin`.

## Qt 6 Runtime Files To Keep

For the current Windows third-party layout, keep the matching Debug and Release `.lib`/`.dll` files for:

- `Qt6Core`
- `Qt6Gui`
- `Qt6Network`
- `Qt6Qml`
- `Qt6QmlModels`
- `Qt6Quick`
- `Qt6OpenGL`

Also keep:

- `qwindows.dll`
- `qwindowsd.dll`

The current runtime deployment helper supports the existing repository layout where Qt DLLs and `qwindows*.dll` live under `3rdparty/qt6/win64/lib`.

## Can Be Removed For The Empty 2D Canvas Shell

- Assimp: only needed for 3D model import.
- GLEW: not needed while Qt owns OpenGL context/function loading.
- ImGui: replaced by QML UI.
- GLM: not needed yet; add later only if the canvas/render math needs it.
- stb_image: not needed yet; use Qt image APIs first unless the engine needs custom decoding.
- FastNoiseLite: old world-generation dependency.
- Old `bin/Resource` shaders/textures: old game resources.
- Qt Widgets / OpenGLWidgets / QuickWidgets: not used by the QML-first shell.
- Qt Quick3D: not used by the 2D canvas shell.
- Qt QuickTest: not needed outside Qt Quick test executables.

## Optional Later

- Qt 6 QuickControls2: useful if you want native Qt controls instead of custom QML controls. The current bundled Qt copy does not appear to include it.
- Skia: strong candidate for a Krita-like raster paint core if you want CPU/GPU accelerated compositing and brushes.
- OpenColorIO / LittleCMS: useful when color management becomes a requirement.
- libpng / libjpeg-turbo / libwebp / OpenEXR: useful when document import/export grows beyond Qt image support.
