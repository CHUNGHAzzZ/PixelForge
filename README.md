# PixelForge

PixelForge is a lightweight 2D canvas application framework built with Qt 6, QML, and OpenGL. It is intended as the foundation for a digital painting and canvas tool, with a QML-driven user interface and a native OpenGL rendering surface for future brush, layer, and document systems.

## Overview

The project currently provides a clean application shell:

- Qt Quick based desktop window
- Dock-style editor layout with toolbar, tool panel, canvas area, and layer panel
- OpenGL-backed canvas item exposed to QML
- Resource-packed QML entry point
- Minimal C++ application controller for future document state

## Tech Stack

- C++20
- Qt 6
- QML / Qt Quick
- OpenGL
- CMake
- Visual Studio 2022 on Windows

## Project Structure

```text
PixelForge/
├── 3rdparty/          # Bundled third-party dependencies
├── docs/              # Project notes and dependency documentation
├── src/
│   ├── App/           # Application-level state and controllers
│   ├── Canvas/        # OpenGL canvas integration
│   ├── qml/           # QML user interface
│   ├── CMakeLists.txt
│   ├── main.cpp
│   └── qml.qrc
├── test/              # Test entry point
├── CMakeLists.txt
└── win_build.bat      # Windows build script
```

## Build

Run the Windows build script from the project root:

```bat
win_build.bat release
```

Debug builds are also supported:

```bat
win_build.bat debug
```

The built application is installed to:

```text
build/PixelForge/win64/bin/PixelForge.exe
```

## Status

PixelForge is currently an empty UI and rendering framework. The next development areas are the document model, brush engine, layer stack, canvas interaction, and image import/export pipeline.
