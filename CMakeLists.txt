cmake_minimum_required(VERSION 4.0)

project(v4l2ucp LANGUAGES C CXX)
set(CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(V4L2UCP_VERSION 2.1.0)

add_definitions(-Wall -DV4L2UCP_VERSION="${V4L2UCP_VERSION}")
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)

MESSAGE(STATUS "Looking for libv4l")
find_library(V4L2_LIBRARY v4l2)
if (V4L2_LIBRARY)
    MESSAGE(STATUS "Looking for libv4l - found")
else (V4L2_LIBRARY)
    MESSAGE(FATAL_ERROR "Looking for libv4l - not found.")
endif (V4L2_LIBRARY)

add_subdirectory(src)

set(DESKTOP_ENTRY v4l2ucp.desktop)
set(APP_ICON v4l2ucp.png)

install(FILES ${CMAKE_SOURCE_DIR}/${APP_ICON} DESTINATION share/icons)
install(FILES ${CMAKE_SOURCE_DIR}/${DESKTOP_ENTRY} DESTINATION share/applications)
