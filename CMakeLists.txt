cmake_minimum_required(VERSION 3.17)

set(SVPLUGIN "SpectralViewPlugin")

PROJECT(${SVPLUGIN})

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

# Get the env path and allow the user to change it
set(INSTALL_DIR $ENV{HDPS_INSTALL_DIR} CACHE PATH "The HDPS Install Directoy")
# Normalize the incoming install path
file(TO_CMAKE_PATH ${INSTALL_DIR} INSTALL_DIR)

find_package(Qt6 6.3.1 COMPONENTS Widgets WebEngineWidgets Concurrent REQUIRED)

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

add_library(${SVPLUGIN} SHARED ${SOURCES} ${WEB} ${AUX} ${RESOURCE_FILES})

qt_wrap_cpp(SV_PARAMETERS_MOC ${PLUGIN_MOC_HEADERS} TARGET ${SVPLUGIN})
target_sources(${SVPLUGIN} PRIVATE ${SV_PARAMETERS_MOC})

target_include_directories(${SVPLUGIN} PRIVATE "${INSTALL_DIR}/$<CONFIGURATION>/include/")

# Request C++17
target_compile_features(${SVPLUGIN} PRIVATE cxx_std_17)

target_link_libraries(${SVPLUGIN} PRIVATE Qt6::Widgets)
target_link_libraries(${SVPLUGIN} PRIVATE Qt6::WebEngineWidgets)
target_link_libraries(${SVPLUGIN} PRIVATE Qt6::Concurrent)

set(HDPS_LINK_PATH "${INSTALL_DIR}/$<CONFIGURATION>/lib")
set(PLUGIN_LINK_PATH "${INSTALL_DIR}/$<CONFIGURATION>/$<IF:$<CXX_COMPILER_ID:MSVC>,lib,Plugins>")
set(HDPS_LINK_SUFFIX $<IF:$<CXX_COMPILER_ID:MSVC>,${CMAKE_LINK_LIBRARY_SUFFIX},${CMAKE_SHARED_LIBRARY_SUFFIX}>)

set(HDPS_LINK_LIBRARY "${HDPS_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}HDPS_Public${HDPS_LINK_SUFFIX}")
set(POINTDATA_LINK_LIBRARY "${PLUGIN_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}PointData${HDPS_LINK_SUFFIX}") 
set(IMAGEDATA_LINK_LIBRARY "${PLUGIN_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}ImageData${HDPS_LINK_SUFFIX}") 
set(CLUSTERDATA_LINK_LIBRARY "${PLUGIN_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}ClusterData${HDPS_LINK_SUFFIX}") 

target_link_libraries(${SVPLUGIN} PRIVATE "${HDPS_LINK_LIBRARY}")
target_link_libraries(${SVPLUGIN} PRIVATE "${POINTDATA_LINK_LIBRARY}")
target_link_libraries(${SVPLUGIN} PRIVATE "${IMAGEDATA_LINK_LIBRARY}")
target_link_libraries(${SVPLUGIN} PRIVATE "${CLUSTERDATA_LINK_LIBRARY}")

install(TARGETS ${SVPLUGIN}
    RUNTIME DESTINATION Plugins COMPONENT PLUGINS # Windows .dll
    LIBRARY DESTINATION Plugins COMPONENT PLUGINS # Linux/Mac .so
)

add_custom_command(TARGET ${SVPLUGIN} POST_BUILD
	COMMAND "${CMAKE_COMMAND}"
	--install ${CMAKE_CURRENT_BINARY_DIR}
	--config $<CONFIGURATION>
	--prefix ${INSTALL_DIR}/$<CONFIGURATION>
)

# Automatically set the debug environment (command + working directory) for MSVC in debug mode
if(MSVC)
	set_property(TARGET ${SVPLUGIN} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<IF:$<CONFIG:DEBUG>,${INSTALL_DIR}/debug,${INSTALL_DIR}/release>)
	set_property(TARGET ${SVPLUGIN} PROPERTY VS_DEBUGGER_COMMAND $<IF:$<CONFIG:DEBUG>,${INSTALL_DIR}/debug/HDPS.exe,${INSTALL_DIR}/release/HDPS.exe>)
endif()
