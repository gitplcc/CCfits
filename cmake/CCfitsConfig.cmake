include(CMakeFindDependencyMacro)
find_dependency(cfitsio 4.6.0)

include("${CMAKE_CURRENT_LIST_DIR}/CCfitsTargets.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/CCfitsMacros.cmake")
