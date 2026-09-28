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
)

if(MINGW)
	find_library(ASSIMP_LIBRARY assimp
		"/mingw64/lib"
		"/mingw32/lib"
		"/ucrt64/lib"
		"/usr/lib64"
		"/usr/lib"
		"/usr/local/lib"
		"/opt/local/lib"
	)
else()
	find_library(ASSIMP_LIBRARY assimp
		"/usr/lib64"
		"/usr/lib"
		"/usr/local/lib"
		"/opt/local/lib"
	)
endif()

# Clear stale cached results so toolchain changes (for example MSVC -> MinGW) are re-evaluated.
unset(ASSIMP_INCLUDE_DIR CACHE)
unset(ASSIMP_LIBRARY CACHE)

if(ENGINE_USE_BUNDLED_DEPS)
	find_path(ASSIMP_INCLUDE_DIR assimp/mesh.h PATHS "${ENGINE_ROOT_DIR}/external/include")
	if(NOT MINGW)
		find_library(ASSIMP_LIBRARY assimp PATHS "${ENGINE_ROOT_DIR}/external/lib")
	endif()
endif()

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
