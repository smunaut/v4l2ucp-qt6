set(SOURCES mainWindow.cpp previewSettings.cpp v4l2controls.cpp v4l2ucp.cpp)
set(HEADERS mainWindow.h previewSettings.h v4l2controls.h)
set(UI_FILES previewSettings.ui)
set(RCS v4l2ucp.qrc)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include_directories(${CMAKE_BINARY_DIR}/src)

add_executable(v4l2ucp ${SOURCES} ${MOC_SOURCES} ${UI_HEADERS} ${RC_SOURCES})
target_link_libraries(v4l2ucp Qt6::Core Qt6::Widgets ${V4L2_LIBRARY})

add_executable(v4l2ctrl v4l2ctrl.c)
target_link_libraries(v4l2ctrl ${V4L2_LIBRARY})

install(TARGETS v4l2ucp v4l2ctrl DESTINATION bin)
