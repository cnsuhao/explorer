include(ExternalProject)

ExternalProject_Add(ParaView
	GIT_REPOSITORY https://gitlab.kitware.com/paraview/paraview.git
	GIT_TAG v5.2.0
    BINARY_DIR ${CMAKE_BINARY_DIR}/ParaView-build
    INSTALL_DIR ${CMAKE_BINARY_DIR}/Install
	CMAKE_ARGS 
		-Wno-dev
		-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
		-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/Install
		-DBUILD_EXAMPLES:BOOL=OFF
		-DBUILD_TESTING:BOOL=OFF
		-DPARAVIEW_QT_VERSION=5
		-DQt5_DIR=${Qt5_DIR}
)

ExternalPRoject_Add(BioGears
	GIT_REPOSITORY https://gitlab.kitware.com/physiology/engine.git
	GIT_TAG ff7a17ae8730d7d24216de9dbe9ec915bacb0135
	BINARY_DIR ${CMAKE_BINARY_DIR}/BioGears-build
	INSTALL_DIR ${CMAKE_BINARY_DIR}/Install
	CMAKE_ARGS
		-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
		-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/Install
)

ExternalProject_Add(BioGearsDemo
	DEPENDS ParaView BioGears
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
	BINARY_DIR ${CMAKE_BINARY_DIR}/BioGearsDemo-build
	INSTALL_DIR ${CMAKE_BINARY_DIR}/Install
	CMAKE_ARGS
		-DDO_SUPERBUILD:BOOL=OFF
		-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
		-DParaView_DIR:PATH=${CMAKE_BINARY_DIR}/ParaView-build
		-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/Install
		-DQt5_DIR=${Qt5_DIR}
)