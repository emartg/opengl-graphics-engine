# The triplet x64-mingw-dynamic (see ../vcpkg_triplets) for the continuous integration: the same, without the compiler in
# the hash (ABI) of the packages. The runner images change the build of the compiler, and every package would be built
# again. The key of the cache of the workflow has the version of the compiler. Not used by the presets: a developer
# keeps the compiler in the hash, so that a package is built again when the compiler is upgraded
include("${CMAKE_CURRENT_LIST_DIR}/../vcpkg_triplets/x64-mingw-dynamic.cmake")
set(VCPKG_DISABLE_COMPILER_TRACKING ON)
