set(_QT6_ROOT "${CMAKE_CURRENT_LIST_DIR}")

set(QT6_PLATFORM_ROOT "")
if(MSVC)
    set(QT6_PLATFORM_ROOT "${_QT6_ROOT}/win64")
elseif(APPLE)
    set(QT6_PLATFORM_ROOT "${_QT6_ROOT}/macos")
else()
    set(QT6_PLATFORM_ROOT "${_QT6_ROOT}/ubuntu")
endif()

if(NOT EXISTS "${QT6_PLATFORM_ROOT}")
    message(FATAL_ERROR "Qt6 not found: ${QT6_PLATFORM_ROOT}")
endif()

set(QT6_BIN_DIR "${QT6_PLATFORM_ROOT}/bin")
set(QT6_LIB_DIR "${QT6_PLATFORM_ROOT}/lib")
set(QT6_INCLUDE_DIR "${QT6_PLATFORM_ROOT}/include")
set(QT6_QML_DIR "${QT6_PLATFORM_ROOT}/qml")

if(WIN32)
    set(QT6_RUNTIME_DIR "${QT6_BIN_DIR}")
    if(NOT EXISTS "${QT6_RUNTIME_DIR}/Qt6Core.dll" AND EXISTS "${QT6_LIB_DIR}/Qt6Core.dll")
        set(QT6_RUNTIME_DIR "${QT6_LIB_DIR}")
    endif()
endif()

function(qt6_require_tools)
    foreach(_tool ${ARGN})
        set(_tool_path "${QT6_BIN_DIR}/${_tool}.exe")
        if(NOT EXISTS "${_tool_path}")
            message(FATAL_ERROR
                "Qt tool '${_tool}.exe' not found: ${_tool_path}. "
                "Please copy it from your Qt 6 installation into 3rdparty/qt6/win64/bin."
            )
        endif()

        string(TOUPPER "${_tool}" _tool_upper)
        set(QT6_${_tool_upper}_EXECUTABLE "${_tool_path}" CACHE FILEPATH "Qt ${_tool} tool" FORCE)
        set(QT6_${_tool_upper}_EXECUTABLE "${_tool_path}" PARENT_SCOPE)
    endforeach()
endfunction()

function(qt6_define_imported_targets)
    if(NOT MSVC)
        message(FATAL_ERROR "qt6_define_imported_targets() currently supports the bundled MSVC Qt layout only")
    endif()

    if(NOT EXISTS "${QT6_LIB_DIR}")
        message(FATAL_ERROR "Qt6 lib dir not found: ${QT6_LIB_DIR}")
    endif()

    set(_qt_modules Core Gui Network Qml QmlModels QmlWorkerScript Quick OpenGL)

    foreach(_module IN LISTS _qt_modules)
        set(_release_lib "${QT6_LIB_DIR}/Qt6${_module}.lib")
        set(_debug_lib "${QT6_LIB_DIR}/Qt6${_module}d.lib")

        if(EXISTS "${_release_lib}" OR EXISTS "${_debug_lib}")
            if(NOT TARGET Qt6::${_module})
                add_library(Qt6::${_module} UNKNOWN IMPORTED)
                set_target_properties(Qt6::${_module} PROPERTIES
                    IMPORTED_CONFIGURATIONS "Debug;Release"
                    INTERFACE_INCLUDE_DIRECTORIES "${QT6_INCLUDE_DIR};${QT6_INCLUDE_DIR}/Qt${_module}"
                )

                if(EXISTS "${_release_lib}")
                    set_target_properties(Qt6::${_module} PROPERTIES
                        IMPORTED_LOCATION_RELEASE "${_release_lib}"
                    )
                endif()

                if(EXISTS "${_debug_lib}")
                    set_target_properties(Qt6::${_module} PROPERTIES
                        IMPORTED_LOCATION_DEBUG "${_debug_lib}"
                    )
                endif()
            endif()
        endif()
    endforeach()
endfunction()

function(qt6_require_modules)
    foreach(_module ${ARGN})
        if(NOT TARGET Qt6::${_module})
            message(FATAL_ERROR
                "Qt module '${_module}' is required but was not found under ${QT6_LIB_DIR}. "
                "Please add Qt6${_module}.lib/Qt6${_module}d.lib and the matching runtime DLLs if the module has runtime binaries."
            )
        endif()
    endforeach()
endfunction()

function(qt6_deploy_windows_runtime target_name)
    if(NOT WIN32)
        return()
    endif()

    set(options)
    set(oneValueArgs)
    set(multiValueArgs MODULES)
    cmake_parse_arguments(DEPLOY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT DEPLOY_MODULES)
        set(DEPLOY_MODULES Core Gui Network Qml QmlModels QmlWorkerScript Quick OpenGL)
    endif()

    foreach(_module IN LISTS DEPLOY_MODULES)
        set(_release_dll "${QT6_RUNTIME_DIR}/Qt6${_module}.dll")
        set(_debug_dll "${QT6_RUNTIME_DIR}/Qt6${_module}d.dll")

        if(NOT EXISTS "${_release_dll}" AND NOT EXISTS "${_debug_dll}")
            message(FATAL_ERROR "Qt6${_module} runtime not found in ${QT6_RUNTIME_DIR}")
        endif()

        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<IF:$<CONFIG:Debug>,${_debug_dll},${_release_dll}>"
                "$<TARGET_FILE_DIR:${target_name}>"
            COMMENT "Copying Qt6${_module} runtime"
        )
    endforeach()

    set(_platform_release "${QT6_RUNTIME_DIR}/qwindows.dll")
    set(_platform_debug "${QT6_RUNTIME_DIR}/qwindowsd.dll")

    if(NOT EXISTS "${_platform_release}" AND EXISTS "${QT6_LIB_DIR}/qwindows.dll")
        set(_platform_release "${QT6_LIB_DIR}/qwindows.dll")
    endif()

    if(NOT EXISTS "${_platform_debug}" AND EXISTS "${QT6_LIB_DIR}/qwindowsd.dll")
        set(_platform_debug "${QT6_LIB_DIR}/qwindowsd.dll")
    endif()

    if(NOT EXISTS "${_platform_release}" AND NOT EXISTS "${_platform_debug}")
        message(FATAL_ERROR "Qt Windows platform plugin qwindows.dll not found")
    endif()

    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:${target_name}>/platforms"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<IF:$<CONFIG:Debug>,${_platform_debug},${_platform_release}>"
            "$<TARGET_FILE_DIR:${target_name}>/platforms/$<IF:$<CONFIG:Debug>,qwindowsd.dll,qwindows.dll>"
        COMMENT "Copying Qt Windows platform plugin"
    )
endfunction()

function(qt6_deploy_qml_imports target_name)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs IMPORTS)
    cmake_parse_arguments(DEPLOY_QML "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT DEPLOY_QML_IMPORTS)
        set(DEPLOY_QML_IMPORTS QtQml QtQuick)
    endif()

    if(NOT EXISTS "${QT6_QML_DIR}")
        message(FATAL_ERROR
            "Qt QML import dir not found: ${QT6_QML_DIR}. "
            "Please copy <QtPrefix>/qml into 3rdparty/qt6/win64/qml."
        )
    endif()

    foreach(_import IN LISTS DEPLOY_QML_IMPORTS)
        if(NOT EXISTS "${QT6_QML_DIR}/${_import}")
            message(FATAL_ERROR "Qt QML import '${_import}' not found in ${QT6_QML_DIR}")
        endif()

        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${QT6_QML_DIR}/${_import}"
                "$<TARGET_FILE_DIR:${target_name}>/qml/${_import}"
            COMMENT "Copying QML import ${_import}"
        )
    endforeach()
endfunction()

function(qt6_install_windows_runtime)
    if(NOT WIN32)
        return()
    endif()

    set(options)
    set(oneValueArgs)
    set(multiValueArgs MODULES)
    cmake_parse_arguments(INSTALL_QT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT INSTALL_QT_MODULES)
        set(INSTALL_QT_MODULES Core Gui Network Qml QmlModels QmlWorkerScript Quick OpenGL)
    endif()

    foreach(_module IN LISTS INSTALL_QT_MODULES)
        install(FILES
            "$<IF:$<CONFIG:Debug>,${QT6_RUNTIME_DIR}/Qt6${_module}d.dll,${QT6_RUNTIME_DIR}/Qt6${_module}.dll>"
            DESTINATION bin
        )
    endforeach()

    install(FILES
        "$<IF:$<CONFIG:Debug>,${QT6_RUNTIME_DIR}/qwindowsd.dll,${QT6_RUNTIME_DIR}/qwindows.dll>"
        DESTINATION bin/platforms
    )
endfunction()

function(qt6_install_qml_imports)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs IMPORTS)
    cmake_parse_arguments(INSTALL_QML "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT INSTALL_QML_IMPORTS)
        set(INSTALL_QML_IMPORTS QtQml QtQuick)
    endif()

    if(NOT EXISTS "${QT6_QML_DIR}")
        message(FATAL_ERROR
            "Qt QML import dir not found: ${QT6_QML_DIR}. "
            "Please copy <QtPrefix>/qml into 3rdparty/qt6/win64/qml."
        )
    endif()

    foreach(_import IN LISTS INSTALL_QML_IMPORTS)
        install(DIRECTORY "${QT6_QML_DIR}/${_import}" DESTINATION bin/qml)
    endforeach()
endfunction()
