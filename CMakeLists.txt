cmake_minimum_required(VERSION 3.17)

option(MV_UNITY_BUILD "Combine target source files into batches for faster compilation" OFF)

# -----------------------------------------------------------------------------
# SpectralView Plugin
# -----------------------------------------------------------------------------
set(SVPLUGIN "SpectralViewPlugin")

PROJECT(${SVPLUGIN})

# -----------------------------------------------------------------------------
# CMake Options
# -----------------------------------------------------------------------------
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DWIN32 /EHsc /MP /permissive- /Zc:__cplusplus /W3")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG /NODEFAULTLIB:LIBCMT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif(MSVC)

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------
find_package(Qt6 COMPONENTS Widgets WebEngineWidgets Concurrent REQUIRED)

find_package(ManiVault COMPONENTS Core PointData ClusterData ImageData CONFIG)

# -----------------------------------------------------------------------------
# Source files
# -----------------------------------------------------------------------------
set(PLUGIN
    src/SpectralViewPlugin.h
    src/SpectralViewPlugin.cpp
    src/EndmembersModel.cpp
    src/EndmembersModel.h
    src/Endmember.cpp
    src/Endmember.h
)

set(PLUGIN_MOC_HEADERS
	src/SpectralViewPlugin.h
)

set(ACTIONS
    src/SettingsAction.h
    src/SettingsAction.cpp
    src/ViewSettingsAction.h
    src/ViewSettingsAction.cpp
    src/WavelengthsRGBAction.h
    src/WavelengthsRGBAction.cpp
    src/EndmembersAction.cpp
    src/EndmembersAction.h
    src/GeneralAction.cpp
    src/GeneralAction.h
    src/MapAction.cpp
    src/MapAction.h
)

set(WIDGETS
    src/LineplotWidget.h
    src/LineplotWidget.cpp
    src/EndmembersCheckDialog.cpp
    src/EndmembersCheckDialog.h
)

set(WEB
	res/lineplot/lineplot.html
	res/lineplot/lineplot.css
	res/lineplot/lineplot.js
	res/lineplot/lineplot.ui.js
	res/lineplot/endmembers.js
	res/lineplot/linesRGB.js	
	res/jstools/debug_data.js
	res/jstools/colormaps.js
	res/jstools/globals.js
	res/jstools/tools.bridge.js
	res/jstools/ui.css	
	res/jslibs/qwebchannel.js
	res/jslibs/wNumb.min.js
	res/jslibs/material.min.js
	res/jslibs/d3.v4.js
	res/jslibs/d3-scale-chromatic.js
	res/jslibs/material.min.css
)

set(AUX
	res/lineplot_resources.qrc
	src/SpectralViewPlugin.json
)

qt6_add_resources(RESOURCE_FILES res/lineplot_resources.qrc)

set(SOURCES ${PLUGIN} ${ACTIONS} ${WIDGETS})

source_group(Plugin FILES ${PLUGIN})
source_group(Actions FILES ${ACTIONS})
source_group(Widget FILES ${WIDGETS})
source_group(Web FILES ${WEB})
source_group(Aux FILES ${AUX})

# -----------------------------------------------------------------------------
# CMake Target
# -----------------------------------------------------------------------------
add_library(${SVPLUGIN} SHARED ${SOURCES} ${WEB} ${AUX} ${RESOURCE_FILES})

# -----------------------------------------------------------------------------
# Target include directories
# -----------------------------------------------------------------------------
target_include_directories(${SVPLUGIN} PRIVATE "${ManiVault_INCLUDE_DIR}")

# -----------------------------------------------------------------------------
# Target properties
# -----------------------------------------------------------------------------
# Request C++17
target_compile_features(${SVPLUGIN} PRIVATE cxx_std_17)

if(MV_UNITY_BUILD)
    set_target_properties(${SVPLUGIN} PROPERTIES UNITY_BUILD ON)
endif()

# -----------------------------------------------------------------------------
# Target library linking
# -----------------------------------------------------------------------------
target_link_libraries(${SVPLUGIN} PRIVATE Qt6::Widgets)
target_link_libraries(${SVPLUGIN} PRIVATE Qt6::WebEngineWidgets)
target_link_libraries(${SVPLUGIN} PRIVATE Qt6::Concurrent)

target_link_libraries(${SVPLUGIN} PRIVATE ManiVault::Core)
target_link_libraries(${SVPLUGIN} PRIVATE ManiVault::PointData)
target_link_libraries(${SVPLUGIN} PRIVATE ManiVault::ClusterData)
target_link_libraries(${SVPLUGIN} PRIVATE ManiVault::ImageData)

# -----------------------------------------------------------------------------
# Target installation
# -----------------------------------------------------------------------------
install(TARGETS ${SVPLUGIN}
    RUNTIME DESTINATION Plugins COMPONENT PLUGINS # Windows .dll
    LIBRARY DESTINATION Plugins COMPONENT PLUGINS # Linux/Mac .so
)

add_custom_command(TARGET ${SVPLUGIN} POST_BUILD
	COMMAND "${CMAKE_COMMAND}"
	--install ${CMAKE_CURRENT_BINARY_DIR}
	--config $<CONFIGURATION>
	--prefix ${ManiVault_INSTALL_DIR}/$<CONFIGURATION>
)

set_target_properties(${SVPLUGIN}
    PROPERTIES
    FOLDER ViewPlugins
)

# -----------------------------------------------------------------------------
# Miscellaneous
# -----------------------------------------------------------------------------
# Automatically set the debug environment (command + working directory) for MSVC in debug mode
if(MSVC)
	set_property(TARGET ${SVPLUGIN} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<IF:$<CONFIG:DEBUG>,${ManiVault_INSTALL_DIR}/debug,${ManiVault_INSTALL_DIR}/release>)
	set_property(TARGET ${SVPLUGIN} PROPERTY VS_DEBUGGER_COMMAND $<IF:$<CONFIG:DEBUG>,"${ManiVault_INSTALL_DIR}/debug/ManiVault Studio.exe","${ManiVault_INSTALL_DIR}/release/ManiVault Studio.exe">)
endif()
