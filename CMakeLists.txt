cmake_minimum_required(VERSION 3.1.0)

set(PROJECT "LinePlotPlugin")

PROJECT(${PROJECT})

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DWIN32 /EHsc /MP")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif(MSVC)

find_package(Qt5 COMPONENTS Widgets WebEngineWidgets REQUIRED)

qt5_add_resources(RESOURCE_FILES res/lineplot_resources.qrc)

set(SOURCES
    src/LineplotPlugin.h
    src/LineplotPlugin.cpp
    src/LineplotWidget.h
    src/LineplotWidget.cpp
    src/SettingsAction.h
    src/SettingsAction.cpp
    src/MainToolbarAction.h
    src/MainToolbarAction.cpp
    src/GlobalSettingsAction.h
    src/GlobalSettingsAction.cpp
    src/EndmembersAction.cpp
    src/EndmembersAction.h
    src/EndmembersModel.cpp
    src/EndmembersModel.h
    src/Endmember.cpp
    src/Endmember.h
    src/GeneralAction.cpp
    src/GeneralAction.h
    src/MapAction.cpp
    src/MapAction.h
    src/LineplotPlugin.json
)

set(RESOURCES
    res/lineplot_resources.qrc
)

source_group( Plugin FILES ${SOURCES})
source_group( Resources FILES ${RESOURCES})
source_group( Resources\\generated FILES ${CMAKE_CURRENT_BINARY_DIR}/qrc_lineplot_resources.cpp)

include_directories("$ENV{HDPS_INSTALL_DIR}/$<CONFIGURATION>/include/")

add_library(${PROJECT} SHARED ${SOURCES} ${RESOURCES} ${RESOURCE_FILES})

qt5_use_modules(${PROJECT} Widgets WebEngineWidgets)

target_link_libraries(${PROJECT} Qt5::Widgets)
target_link_libraries(${PROJECT} Qt5::WebEngineWidgets)
target_link_libraries(${PROJECT} "$ENV{HDPS_INSTALL_DIR}/$<CONFIGURATION>/lib/HDPS_Public.lib")
target_link_libraries(${PROJECT} "$ENV{HDPS_INSTALL_DIR}/$<CONFIGURATION>/lib/PointData.lib")
target_link_libraries(${PROJECT} "$ENV{HDPS_INSTALL_DIR}/$<CONFIGURATION>/lib/ClusterData.lib")
target_link_libraries(${PROJECT} "$ENV{HDPS_INSTALL_DIR}/$<CONFIGURATION>/lib/ImageData.lib")

add_custom_command(TARGET ${PROJECT} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "$<TARGET_FILE:${PROJECT}>"
    "$ENV{HDPS_INSTALL_DIR}/$<CONFIGURATION>/Plugins/$<TARGET_FILE_NAME:${PROJECT}>"
)
