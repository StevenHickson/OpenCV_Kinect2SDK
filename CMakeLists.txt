cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

SET_PROPERTY(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)

IF(WIN32 OR win64)
   SET(CMAKE_FIND_LIBRARY_SUFFIXES .lib .dll)
ELSE()
   SET(CMAKE_FIND_LIBRARY_SUFFIXES .a)
ENDIF()

find_package( OpenCV REQUIRED )

project(OpenCV_Kinect2SDK)

add_subdirectory(src)

include_directories(C:/opencv/build)
include_directories(C:/Program Files/Microsoft SDKs/Kinect/v2.0-DevPreview1311/lib/x64)
include_directories(C:/Program Files/Microsoft SDKs/Kinect/v2.0-DevPreview1311/inc)
include_directories(${OpenCV_Kinect2SDK_SOURCE_DIR}/include)

link_directories(C:/blepo/external64/Microsoft/Kinect)

file(GLOB_RECURSE OpenCV_Kinect2SDK_HEADERS include/*.h)
file(GLOB_RECURSE OpenCV_Kinect2SDK_SOURCES src/*.cpp)
set (OpenCV_Kinect2SDK_INCLUDE_DIRS "")
foreach (_headerFile ${OpenCV_Kinect2SDK_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND OpenCV_Kinect2SDK_INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES OpenCV_Kinect2SDK_INCLUDE_DIRS)

include_directories(${OpenCV_Kinect2SDK_INCLUDE_DIRS})
add_library(OpenCV_Kinect2SDK STATIC src/Microsoft_grabber2.cpp)
target_link_libraries (OpenCV_Kinect2SDK ${OpenCV_LIBS} Kinect20.lib)

add_executable (Test_SDK src/OpenCV_Kinect2SDK.cpp)
target_link_libraries (Test_SDK OpenCV_Kinect2SDK ${OpenCV_LIBS} Kinect20.lib)