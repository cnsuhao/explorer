cmake_minimum_required(VERSION 3.3)
set(project_name BioGearsDemo)
project(${project_name})

set(${project_name}_CMAKE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

# Superbuild stuff
option(DO_SUPERBUILD "Build ${project_name}'s dependencies" ON)

if (DO_SUPERBUILD)
	include("${${project_name}_CMAKE_SOURCE_DIR}/Superbuild.cmake")
	return()
endif()

find_package(ParaView REQUIRED)
include (${PARAVIEW_USE_FILE})
if (NOT PARAVIEW_BUILD_QT_GUI)
	status(FATAL_ERROR "${project_name} requires ParaView to be built with Qt")
endif()

# include rules for creating custom applications.
include(ParaViewBranding)

# Set a consistent MACOSX_RPATH default across all CMake versions.
# When CMake 2.8.12 is required, change this default to 1.
# When CMake 3.0.0 is required, remove this block (see CMP0042).
if(NOT DEFINED CMAKE_MACOSX_RPATH)
  set(CMAKE_MACOSX_RPATH 0)
endif()

# Set up our directory structure for output libraries and binaries
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${${project_name}_BINARY_DIR}/bin")
endif()
if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  if(UNIX)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${${project_name}_BINARY_DIR}/lib")
  else()
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${${project_name}_BINARY_DIR}/bin")
  endif()
endif()
if(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${${project_name}_BINARY_DIR}/lib")
endif()

set(moc_sources
  ${project_name}MainWindow.h
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
  ${MOC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES})

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR})

build_paraview_client(${project_name}
  TITLE "BioGears Demo"
  ORGANIZATION  "Kitware, Inc."
  VERSION_MAJOR 0
  VERSION_MINOR 1
  VERSION_PATCH 0
#  SPLASH_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/RSplash.png"
  PVMAIN_WINDOW ${project_name}MainWindow
  PVMAIN_WINDOW_INCLUDE ${project_name}MainWindow.h
  EXTRA_DEPENDENCIES vtkPVServerManagerRendering
  SOURCES ${${project_name}_SOURCE_FILES}
)

