# FindASSIMP - attempts to locate the Assimp library.
# 
# This module defines the following variables (on success):
# ASSIMP_FOUND - system has Assimp
# ASSIMP_INCLUDE_DIR - the Assimp include directories
# ASSIMP_LIBRARIES - link these to use Assimp

find_path(ASSIMP_INCLUDE_DIR assimp/mesh.h
	"/usr/include"
	"/usr/local/include"
	"/opt/local/include"
	"${CMAKE_SOURCE_DIR}/external/include"
)

find_library(ASSIMP_LIBRARY assimp
	"/usr/lib64"
	"/usr/lib"
	"/usr/local/lib"
	"/opt/local/lib"
	"${CMAKE_SOURCE_DIR}/external/lib"
)

if(ASSIMP_INCLUDE_DIR AND ASSIMP_LIBRARY)
	SET(ASSIMP_FOUND TRUE)
	SET(ASSIMP_LIBRARIES ${ASSIMP_LIBRARY})
endif(ASSIMP_INCLUDE_DIR AND ASSIMP_LIBRARY)

if(ASSIMP_FOUND)
	if(NOT ASSIMP_FIND_QUIETLY)
		MESSAGE(STATUS "Found ASSIMP: ${ASSIMP_LIBRARY}")
	endif(NOT ASSIMP_FIND_QUIETLY)
else(ASSIMP_FOUND)
	if(ASSIMP_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could not find libASSIMP")
	endif(ASSIMP_FIND_REQUIRED)
endif(ASSIMP_FOUND)
