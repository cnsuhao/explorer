cmake_minimum_required(VERSION 3.7)
set(project_name PhysiologyExplorer)
project(${project_name})

set(${project_name}_CMAKE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

find_package(Qt5 COMPONENTS Core Widgets Charts REQUIRED)

# Superbuild stuff
option(DO_SUPERBUILD "Build ${project_name}'s dependencies" ON)

if (DO_SUPERBUILD)
	include("${${project_name}_CMAKE_SOURCE_DIR}/Superbuild.cmake")
	return()
endif()

set(ParaView_DIR ${CMAKE_INSTALL_PREFIX}/../ParaView-build)
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
  ${project_name}MainWindow.h
  FileLoader.h
  Scenario.h
)

#------------------------------------------------------------------------------
# Add extra library containing custom code for the client.
IF (PARAVIEW_QT_VERSION VERSION_GREATER "4")
  QT5_WRAP_CPP(MOC_BUILT_SOURCES
    ${moc_sources})
  QT5_WRAP_UI(UI_BUILT_SOURCES
    ${project_name}MainWindow.ui)
ELSE ()
  QT4_WRAP_CPP(MOC_BUILT_SOURCES
    ${moc_sources})
  QT4_WRAP_UI(UI_BUILT_SOURCES
    ${project_name}MainWindow.ui)
ENDIF ()

SOURCE_GROUP("Generated" FILES
  ${MOC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES}
)

SET(${project_name}_SOURCE_FILES
  ${project_name}MainWindow.cxx
  ${project_name}MainWindow.h
  FileLoader.cxx
  FileLoader.h
  Scenario.cxx
  Scenario.h
  ${MOC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES})

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  "${CMAKE_INSTALL_PREFIX}/../install/include/"
  "${CMAKE_INSTALL_PREFIX}/../install/include/cdm"
  "${CMAKE_INSTALL_PREFIX}/../install/include/bind"
  )

set(CONFIG)
if(MSVC)
 set(CONFIG "/$(Configuration)")
endif()

#LINK_DIRECTORIES(
#  "${CMAKE_CURRENT_BINARY_DIR}/../install/lib${CONFIG}"
#  )

SET(PULSE_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib${CONFIG}")

if (WIN32)
  SET(LIB_ENDING "lib")
elseif (APPLE)
  SET(LIB_ENDING "a")
else()
  SET(LIB_ENDING "a")
endif()



set(CMAKE_MODULE_PATH ${ParaView_CMAKE_DIR})
include (${PARAVIEW_USE_FILE})
# include rules for creating custom applications.
include(ParaViewBranding)

file(COPY data DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

build_paraview_client(${project_name}
  TITLE "Pulse Physiology Explorer"
  ORGANIZATION  "Kitware, Inc."
  VERSION_MAJOR 0
  VERSION_MINOR 1
  VERSION_PATCH 0
#  SPLASH_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/RSplash.png"
  PVMAIN_WINDOW ${project_name}MainWindow
  PVMAIN_WINDOW_INCLUDE ${project_name}MainWindow.h
  EXTRA_DEPENDENCIES ${Qt5Charts_LIBRARIES} vtkPVServerManagerRendering
                     ${PULSE_LIB_DIR}/PulseEngine.${LIB_ENDING} 
                     ${PULSE_LIB_DIR}/CommonDataModel.${LIB_ENDING} 
                     ${PULSE_LIB_DIR}/DataModelBindings.${LIB_ENDING}
                     ${PULSE_LIB_DIR}/log4cpp.${LIB_ENDING}
                     ${PULSE_LIB_DIR}/libprotobuf.${LIB_ENDING}
  SOURCES ${${project_name}_SOURCE_FILES}
)

