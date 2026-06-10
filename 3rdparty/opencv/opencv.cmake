set(_OPENCV_ROOT "${CMAKE_CURRENT_LIST_DIR}")

set(OPENCV_PLATFORM_ROOT "")
if(MSVC)
    set(OPENCV_PLATFORM_ROOT "${_OPENCV_ROOT}/win64")
else()
    message(FATAL_ERROR "Bundled OpenCV currently supports the MSVC win64 layout only")
endif()

set(OPENCV_INCLUDE_DIR "${OPENCV_PLATFORM_ROOT}/include")
set(OPENCV_LIB_DIR "${OPENCV_PLATFORM_ROOT}/lib")

function(opencv_define_imported_targets)
    if(NOT EXISTS "${OPENCV_INCLUDE_DIR}/opencv2/opencv.hpp")
        message(FATAL_ERROR
            "OpenCV headers not found: ${OPENCV_INCLUDE_DIR}/opencv2/opencv.hpp. "
            "Please copy the OpenCV headers into 3rdparty/opencv/win64/include."
        )
    endif()

    set(_opencv_world_release "${OPENCV_LIB_DIR}/opencv_world460.lib")
    if(NOT EXISTS "${_opencv_world_release}")
        message(FATAL_ERROR
            "OpenCV world library not found: ${_opencv_world_release}. "
            "Please copy opencv_world460.lib into 3rdparty/opencv/win64/lib."
        )
    endif()

    set(_opencv_thirdparty_libs
        libprotobuf
        libjpeg-turbo
        libpng
        libtiff
        zlib
    )

    foreach(_lib IN LISTS _opencv_thirdparty_libs)
        set(_lib_path "${OPENCV_LIB_DIR}/${_lib}.lib")
        if(NOT EXISTS "${_lib_path}")
            message(FATAL_ERROR
                "OpenCV static dependency not found: ${_lib_path}. "
                "Please copy ${_lib}.lib into 3rdparty/opencv/win64/lib."
            )
        endif()

        if(NOT TARGET OpenCV::${_lib})
            add_library(OpenCV::${_lib} STATIC IMPORTED)
            set_target_properties(OpenCV::${_lib} PROPERTIES
                IMPORTED_CONFIGURATIONS Release
                IMPORTED_LOCATION_RELEASE "${_lib_path}"
            )
        endif()
    endforeach()

    if(NOT TARGET OpenCV::world)
        add_library(OpenCV::world STATIC IMPORTED)
        set_target_properties(OpenCV::world PROPERTIES
            IMPORTED_CONFIGURATIONS Release
            IMPORTED_LOCATION_RELEASE "${_opencv_world_release}"
            INTERFACE_INCLUDE_DIRECTORIES "${OPENCV_INCLUDE_DIR}"
            INTERFACE_LINK_LIBRARIES "OpenCV::libprotobuf;OpenCV::libjpeg-turbo;OpenCV::libpng;OpenCV::libtiff;OpenCV::zlib"
        )
    endif()
endfunction()
