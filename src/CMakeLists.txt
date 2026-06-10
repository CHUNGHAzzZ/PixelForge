include("${CMAKE_SOURCE_DIR}/3rdparty/qt6/qt6.cmake")
include("${CMAKE_SOURCE_DIR}/3rdparty/opencv/opencv.cmake")

qt6_define_imported_targets()
opencv_define_imported_targets()
qt6_require_tools(moc rcc)
qt6_require_modules(Core Gui Network Qml QmlModels QmlWorkerScript Quick OpenGL)

set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${GENERATED_DIR}")

set(QT6_TOOL_PATHS "${QT6_RUNTIME_DIR}" "${QT6_BIN_DIR}" "$ENV{PATH}")
cmake_path(CONVERT "${QT6_TOOL_PATHS}" TO_NATIVE_PATH_LIST QT6_TOOL_PATH_ENV)

set(MOC_HEADERS
    App/ApplicationController.h
    App/Canvas/Render/CanvasItem.h
)

set(GENERATED_MOC_SOURCES)
foreach(_header IN LISTS MOC_HEADERS)
    get_filename_component(_header_name "${_header}" NAME_WE)
    set(_moc_output "${GENERATED_DIR}/moc_${_header_name}.cpp")

    add_custom_command(
        OUTPUT "${_moc_output}"
        COMMAND ${CMAKE_COMMAND} -E env "PATH=${QT6_TOOL_PATH_ENV}"
            "${QT6_MOC_EXECUTABLE}"
            -I"${QT6_INCLUDE_DIR}"
            -I"${QT6_INCLUDE_DIR}/QtCore"
            -I"${QT6_INCLUDE_DIR}/QtGui"
            -I"${QT6_INCLUDE_DIR}/QtNetwork"
            -I"${QT6_INCLUDE_DIR}/QtQml"
            -I"${QT6_INCLUDE_DIR}/QtQuick"
            -I"${QT6_INCLUDE_DIR}/QtOpenGL"
            -I"${CMAKE_CURRENT_SOURCE_DIR}"
            --include "${CMAKE_CURRENT_SOURCE_DIR}/Common/QmlProperty.hpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/${_header}"
            -o "${_moc_output}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${_header}"
                "${CMAKE_CURRENT_SOURCE_DIR}/Common/QmlProperty.hpp"
        VERBATIM
    )

    list(APPEND GENERATED_MOC_SOURCES "${_moc_output}")
endforeach()

set(QRC_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/GUI/qml.qrc")
set(GENERATED_QRC_SOURCE "${GENERATED_DIR}/qrc_qml.cpp")

add_custom_command(
    OUTPUT "${GENERATED_QRC_SOURCE}"
    COMMAND ${CMAKE_COMMAND} -E env "PATH=${QT6_TOOL_PATH_ENV}"
        "${QT6_RCC_EXECUTABLE}"
        --name qml
        "${QRC_SOURCE}"
        -o "${GENERATED_QRC_SOURCE}"
    DEPENDS
        "${QRC_SOURCE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/GUI/qml/Main.qml"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/GUI"
    VERBATIM
)

set(PIXELFORGE_SOURCES
    main.cpp
    App/ApplicationController.cpp
    App/ApplicationController.h
    App/Canvas/Render/CanvasItem.cpp
    App/Canvas/Render/CanvasItem.h
    App/Canvas/Object/BaseObject.cpp
    App/Canvas/Object/BaseObject.h
    App/Canvas/Object/CanvasObject.cpp
    App/Canvas/Object/CanvasObject.h
    App/Canvas/Object/ImageObject.cpp
    App/Canvas/Object/ImageObject.h
    App/Canvas/Object/ObjectTypes.hpp
    App/Canvas/Object/PathObject.cpp
    App/Canvas/Object/PathObject.h
    Common/QmlProperty.hpp
    Common/QmlTypeRegistration.cpp
    Common/QmlTypeRegistration.h
    GUI/qml.qrc
    GUI/qml/Main.qml
    ${GENERATED_MOC_SOURCES}
    ${GENERATED_QRC_SOURCE}
)

set(PIXELFORGE_LOCAL_SOURCES
    main.cpp
    App/ApplicationController.cpp
    App/ApplicationController.h
    App/Canvas/Render/CanvasItem.cpp
    App/Canvas/Render/CanvasItem.h
    App/Canvas/Object/BaseObject.cpp
    App/Canvas/Object/BaseObject.h
    App/Canvas/Object/CanvasObject.cpp
    App/Canvas/Object/CanvasObject.h
    App/Canvas/Object/ImageObject.cpp
    App/Canvas/Object/ImageObject.h
    App/Canvas/Object/ObjectTypes.hpp
    App/Canvas/Object/PathObject.cpp
    App/Canvas/Object/PathObject.h
    Common/QmlProperty.hpp
    Common/QmlTypeRegistration.cpp
    Common/QmlTypeRegistration.h
    GUI/qml.qrc
    GUI/qml/Main.qml
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${PIXELFORGE_LOCAL_SOURCES})
source_group("Generated" FILES ${GENERATED_MOC_SOURCES} ${GENERATED_QRC_SOURCE})

add_executable(PixelForge
    ${PIXELFORGE_SOURCES}
)

target_include_directories(PixelForge
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_compile_options(PixelForge
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>
)

target_link_libraries(PixelForge
    PRIVATE
        OpenCV::world
        Qt6::Core
        Qt6::Gui
        Qt6::Network
        Qt6::Qml
        Qt6::QmlModels
        Qt6::QmlWorkerScript
        Qt6::Quick
        Qt6::OpenGL
        opengl32
)

set_target_properties(PixelForge PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:PixelForge>"
)

qt6_deploy_windows_runtime(PixelForge
    MODULES Core Gui Network Qml QmlModels QmlWorkerScript Quick OpenGL
)

qt6_deploy_qml_imports(PixelForge
    IMPORTS QtQml QtQuick
)

install(TARGETS PixelForge
    RUNTIME DESTINATION bin
)

qt6_install_windows_runtime(
    MODULES Core Gui Network Qml QmlModels QmlWorkerScript Quick OpenGL
)

qt6_install_qml_imports(
    IMPORTS QtQml QtQuick
)
