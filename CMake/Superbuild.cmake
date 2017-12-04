include(ExternalProject)
project(ExplorerSuperBuild)

set( Qt5_DIR "" CACHE PATH "Path to Qt" )
set( ParaView_DIR "" CACHE PATH "Path to ParaView" )
set( Pulse_DIR "" CACHE PATH "Path to Pulse" )

set(DEPENDENCIES)

set( ParaView_FOUND FALSE)
if(ParaView_DIR)
  ## Make sure this is a good ParaView dir
  if ( IS_DIRECTORY ${ParaView_DIR} )
    message(STATUS "Looking for your ParaView...")
    find_package( ParaView NO_MODULE )
    if ( ParaView_FOUND )
      message(STATUS "I found your ParaView!")
      find_package(Qt5 COMPONENTS Core Widgets Charts REQUIRED)
    else()
     message(STATUS "I could not find your ParaView!")
    endif()
  else()
    message(STATUS "Your ParaView_DIR is not a directory...")
    set( ParaView_FOUND FALSE)
  endif()
endif()

if(NOT ParaView_FOUND)
  set( ParaView_DIR "${CMAKE_BINARY_DIR}/ParaView-build" CACHE PATH "Path to ParaView" )
  message(STATUS "Downloading and build ParaView.")
  
  find_package(Qt5 COMPONENTS Core Widgets Charts REQUIRED)
  # Use the Qt that ParaView uses
  ExternalProject_Add(ParaView
    PREFIX ParaView
    GIT_REPOSITORY https://gitlab.kitware.com/paraview/paraview.git
    GIT_TAG v5.2.0
    GIT_SHALLOW 1
    BINARY_DIR ${CMAKE_BINARY_DIR}/ParaView-build
    INSTALL_DIR ${CMAKE_BINARY_DIR}/install
    CMAKE_ARGS 
      -Wno-dev
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install
      -DBUILD_EXAMPLES:BOOL=OFF
      -DBUILD_TESTING:BOOL=OFF
      -DPARAVIEW_USE_QTHELP:BOOL=OFF
      -DPARAVIEW_ENABLE_EMBEDDED_DOCUMENTATION:BOOL=OFF
      -DPARAVIEW_QT_VERSION=5
      -DQt5_DIR=${Qt5_DIR}
  )
  list(APPEND DEPENDENCIES "ParaView")
endif()

set( Pulse_FOUND FALSE)
if(Pulse_DIR)
  ## Make sure this is a good ParaView dir
  if ( IS_DIRECTORY ${Pulse_DIR} )
    message(STATUS "Looking for your Pulse...")
    find_package( Pulse NO_MODULE )
    if ( Pulse_FOUND )
      message(STATUS "I found your Pulse!")
    else()
     message(STATUS "I could not find your Pulse!")
    endif()
  else()
    message(STATUS "Your Pulse_DIR is not a directory...")
    set( Pulse_FOUND FALSE)
  endif()
endif()

if(NOT Pulse_FOUND)
  set( Pulse_DIR "${CMAKE_BINARY_DIR}/Pulse-build" CACHE PATH "Path to Pulse" )
  message(STATUS "Downloading and build Pulse.")
  ExternalProject_Add(Pulse
    PREFIX Pulse
    GIT_REPOSITORY https://gitlab.kitware.com/physiology/engine.git
    GIT_TAG v1.0
    GIT_SHALLOW 1
    BINARY_DIR ${CMAKE_BINARY_DIR}/Pulse-build
    INSTALL_DIR ${CMAKE_BINARY_DIR}/install
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install
      -DDOWNLOAD_BASELINES:BOOL=OFF
  )
  list(APPEND DEPENDENCIES "Pulse")
endif()

ExternalProject_Add(PhysiologyExplorer
  PREFIX PhysiologyExplorer
  DEPENDS ${DEPENDENCIES}
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  BINARY_DIR ${CMAKE_BINARY_DIR}/PhysiologyExplorer-build
  INSTALL_DIR ${CMAKE_BINARY_DIR}/install
  CMAKE_ARGS
    -DDO_SUPERBUILD:BOOL=OFF
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install
    -DParaView_DIR:PATH=${ParaView_DIR}
    -DQt5_DIR=${Qt5_DIR}
    -DPulse_DIR:PATH=${Pulse_DIR}
)