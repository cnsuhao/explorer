cmake_minimum_required(VERSION 3.7)

# Superbuild stuff
option(DO_SUPERBUILD "Download and build any project dependencies" ON)
if (DO_SUPERBUILD)
	include("CMake/Superbuild.cmake")
	return()
endif()

set(project_name PhysiologyExplorer)
project(${project_name})

set(${project_name}_CMAKE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

find_package(Qt5Core REQUIRED)

find_package(ParaView REQUIRED)
if (NOT PARAVIEW_BUILD_QT_GUI)
	status(FATAL_ERROR "${project_name} requires ParaView to be built with Qt")
endif()

# Set a consistent MACOSX_RPATH default across all CMake versions.
# When CMake 2.8.12 is required, change this default to 1.
# When CMake 3.0.0 is required, remove this block (see CMP0042).
if(NOT DEFINED CMAKE_MACOSX_RPATH)
  set(CMAKE_MACOSX_RPATH 0)
endif()

# Set up our directory structure for output libraries and binaries
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin")
endif()
if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  if(UNIX)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_INSTALL_PREFIX}/lib")
  else()
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin")
  endif()
endif()
if(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_INSTALL_PREFIX}/lib")
endif()

set(moc_sources
  MainExplorerWindow.h
  QPulse.h
  GeometryView.h
  ExplorerIntroWidget.h
  VitalsMonitorWidget.h
  DataRequestsWidget.h
  AnaphylaxisShowcaseWidget.h
  MultiTraumaShowcaseWidget.h
)

#------------------------------------------------------------------------------
# Add extra library containing custom code for the client.
IF (PARAVIEW_QT_VERSION VERSION_GREATER "4")
  QT5_WRAP_CPP(MOC_BUILT_SOURCES
    ${moc_sources})
  QT5_WRAP_UI(UI_BUILT_SOURCES
    MainExplorerWindow.ui
    ExplorerIntro.ui
    VitalsMonitor.ui
    DataRequests.ui
    AnaphylaxisShowcase.ui
    MultiTraumaShowcase.ui)
ELSE ()
  QT4_WRAP_CPP(MOC_BUILT_SOURCES
    ${moc_sources})
  QT4_WRAP_UI(UI_BUILT_SOURCES
    MainExplorerWindow.ui 
    ExplorerIntro.ui 
    VitalsMonitor.ui
    DataRequests.ui
    AnaphylaxisShowcase.ui
    MultiTraumaShowcase.ui)
ENDIF ()

find_package(Pulse REQUIRED)

SOURCE_GROUP("Generated" FILES
  ${MOC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES}
)

SET(${project_name}_SOURCE_FILES
  MainExplorerWindow.cxx
  MainExplorerWindow.h
  QPulse.cxx
  QPulse.h 
  QPulsePlot.cxx
  QPulsePlot.h
  GeometryView.cxx
  GeometryView.h
  vtkWaveformWidget.cxx
  vtkWaveformWidget.h
  DataRequestsWidget.cxx
  DataRequestsWidget.h
  VitalsMonitorWidget.cxx
  VitalsMonitorWidget.h
  ExplorerIntroWidget.cxx
  ExplorerIntroWidget.h
  AnaphylaxisShowcaseWidget.cxx
  AnaphylaxisShowcaseWidget.h
  MultiTraumaShowcaseWidget.cxx
  MultiTraumaShowcaseWidget.h
  ${MOC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES})

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Pulse_INCLUDE_DIRS}
  )

set(CMAKE_MODULE_PATH ${ParaView_CMAKE_DIR})
set(BRAND TRUE)
if(BRAND)
  include (${PARAVIEW_USE_FILE})
  # include rules for creating custom applications.
  include(ParaViewBranding)
  build_paraview_client(${project_name}
    TITLE "Pulse Physiology Explorer"
    ORGANIZATION  "Kitware, Inc."
    VERSION_MAJOR 0
    VERSION_MINOR 1
    VERSION_PATCH 0
  #  SPLASH_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/RSplash.png"
    PVMAIN_WINDOW MainExplorerWindow
    PVMAIN_WINDOW_INCLUDE MainExplorerWindow.h
    EXTRA_DEPENDENCIES ${Qt5Charts_LIBRARIES} vtkPVServerManagerRendering
                       ${Pulse_LIBS}
    SOURCES ${${project_name}_SOURCE_FILES}
  )
else()
  include(ParaViewQt)
  add_executable(${project_name} ${${project_name}_SOURCE_FILES})
  target_include_directories(${project_name} PRIVATE ${Qt_INCLUDE_DIRS})
  target_include_directories(${project_name} PRIVATE ${PARAVIEW_INCLUDE_DIRS})
  target_include_directories(${project_name} PRIVATE ${Pulse_INCLUDE_DIRS})
  target_link_libraries(${project_name} debug "${Pulse_DEBUG_LIBS}")
  target_link_libraries(${project_name} optimized "${Pulse_LIBS}")  
  target_link_libraries(${project_name} general ${Qt5Charts_LIBRARIES} ${PARAVIEW_LIBRARIES} 
        pqApplicationComponents
        vtkPVServerManagerApplication
        vtksys vtkPVServerManagerRendering)
endif()

file(COPY data DESTINATION ${Pulse_INSTALL}/bin)
# Need to support debug still
if(WIN32)
  set(postfix)#for debug builds this should be "d"
  set(Qt_DLLs 
    ${Qt5_DIR}/../../../bin/d3dcompiler_47.dll
    ${Qt5_DIR}/../../../bin/libEGL${postfix}.dll
    ${Qt5_DIR}/../../../bin/libGLESv2${postfix}.dll
    ${Qt5_DIR}/../../../bin/opengl32sw.dll
    ${Qt5_DIR}/../../../bin/Qt5Bluetooth${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5Charts${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5Concurrent${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5Core${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5DataVisualization${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5DBus${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5Designer${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5DesignerComponents${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5Gamepad${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5Gui${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5Help${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5Location${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5Multimedia${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5MultimediaQuick_p${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5MultimediaWidgets${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5Network${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5NetworkAuth${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5Nfc${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5OpenGL${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5Positioning${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5PrintSupport${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5Purchasing${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5Qml${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5Quick${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5QuickControls2${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5QuickParticles${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5QuickTemplates2${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5QuickTest${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5QuickWidgets${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5RemoteObjects${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5Script${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5ScriptTools${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5Scxml${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5Sensors${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5SerialBus${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5SerialPort${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5Sql${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5Svg${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5Test${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5TextToSpeech${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5WebChannel${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5WebEngine${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5WebEngineCore${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5WebEngineWidgets${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5WebSockets${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5WebView${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5Widgets${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5WinExtras${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5Xml${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt5XmlPatterns${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt53DAnimation${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt53DCore${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt53DExtras${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt53DInput${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt53DLogic${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt53DQuick${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt53DQuickAnimation${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt53DQuickExtras${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt53DQuickInput${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt53DQuickRender${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt53DQuickScene2D${postfix}.dll
    ${Qt5_DIR}/../../../bin/Qt53DRender${postfix}.dll
  )
  install(FILES ${Qt_DLLs} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
  install(DIRECTORY ${Qt5_DIR}/../../../plugins/platforms DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
endif()

