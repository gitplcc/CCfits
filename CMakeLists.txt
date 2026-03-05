cmake_minimum_required(VERSION 3.23...4.2)

project(CCfits
  VERSION 2.7
  DESCRIPTION "CCfits is an object oriented interface to the CFITSIO library."
  HOMEPAGE_URL "https://heasarc.gsfc.nasa.gov/docs/software/fitsio/ccfits"
  LANGUAGES CXX
)

include(CMakeDependentOption)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# Allow the developer to select whether to build Dynamic or Static libraries
option(BUILD_SHARED_LIBS "Build Shared Libraries" OFF)

#==============================================================================
# Stand-alone builds:
#==============================================================================

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  # Enable CCache if available
  find_program(CCACHE_PROGRAM ccache)
  mark_as_advanced(CCACHE_PROGRAM)

  CMAKE_DEPENDENT_OPTION(BUILD_USING_CCACHE
    "Enable using ccache for speed up compilation." ON
    "CCACHE_PROGRAM" OFF
  )
  if(BUILD_USING_CCACHE)
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
  endif()

  # Enable tests by default
  include(CTest)
endif()

#==============================================================================
# Source code:
#==============================================================================

file(GLOB H_FILES "*.h")
list(APPEND H_FILES CCfits)

set(SRC_FILES
  AsciiTable.cxx
  BinTable.cxx
  ColumnCreator.cxx
  Column.cxx
  ColumnData.cxx
  ColumnVectorData.cxx
  ExtHDU.cxx
  FITS.cxx
  FitsError.cxx
  FITSUtil.cxx
  GroupTable.cxx
  HDUCreator.cxx
  HDU.cxx
  KeyData.cxx
  KeywordCreator.cxx
  Keyword.cxx
  PHDU.cxx
  Table.cxx
)

#==============================================================================
# Dependencies:
#==============================================================================

find_package(cfitsio 4.6)
if(cfitsio_FOUND)
  message(STATUS "Dependency cfitsio found in config mode.")
else()
  message(STATUS "Falling-back to use pkg-config to find cfitsio dependency.")
  # Try to get CFITSIO info (incl. dependencies) using pkg-config:
  set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH "on")
  find_package(PkgConfig)
  if(${PkgConfig_FOUND})
    PKG_CHECK_MODULES(CFITSIO REQUIRED cfitsio)
    if(CFITSIO_FOUND)
      message(STATUS "Found CFITSIO:")
      message(STATUS "  CFITSIO_INCLUDE_DIRS = ${CFITSIO_INCLUDE_DIRS}")
      message(STATUS "  CFITSIO_LIBRARY_DIRS = ${CFITSIO_LIBRARY_DIRS}")
      message(STATUS "  CFITSIO_LIBRARIES = ${CFITSIO_LIBRARIES}")
      include_directories(${CFITSIO_INCLUDE_DIRS})
      link_directories(${CFITSIO_LIBRARY_DIRS})
    endif(CFITSIO_FOUND)
  # If pkg-config not found, try to find CFITSIO; dependencies e.g.
  # zlib & curl are really only needed here if CFITSIO is static.
  else()
    find_package(CFITSIO REQUIRED)
    find_package(ZLIB REQUIRED)
    if(NOT WIN32)
      find_package(CURL)
    endif(NOT WIN32)
  endif(${PkgConfig_FOUND})
endif()

#==============================================================================
# Target:
#==============================================================================

add_library(${PROJECT_NAME})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET ccfits_hdrs
      TYPE HEADERS
      FILES ${H_FILES}
  PRIVATE
    ${SRC_FILES}
)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)
set_target_properties(${PROJECT_NAME}
  PROPERTIES
    VERSION   ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    INTERFACE_${PROJECT_NAME}_MAJOR_VERSION ${PROJECT_VERSION_MAJOR}
)
set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY
  COMPATIBLE_INTERFACE_STRING ${PROJECT_VERSION_MAJOR}
)

# Visual Studio:
if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  if(BUILD_SHARED_LIBS)
    # MSVC shared is broken; better fix would be to "hide all symbols
    # for all compilers", and export public symbols with a macro.
    set_property(TARGET ${PROJECT_NAME} PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS ON)
  endif(BUILD_SHARED_LIBS)
endif(MSVC)

# Add link dependencies:
if(${PkgConfig_FOUND})
  # pkg-config supplies the CFITSIO dependencies (zlib, curl)
  if(CFITSIO_FOUND)
    target_link_libraries(${PROJECT_NAME} ${CFITSIO_LIBRARIES})
  endif(CFITSIO_FOUND)
else()
  # pkg-config wasn't found, so add everything individually:
  include_directories(${CFITSIO_INCLUDE_DIR})
  target_link_libraries(${PROJECT_NAME} ${CFITSIO_LIBRARY})
  target_link_libraries(${PROJECT_NAME} ${ZLIB_LIBRARIES})
  if(CURL_FOUND)
    target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARIES})
   endif(CURL_FOUND)
endif(${PkgConfig_FOUND})

#==============================================================================
# Install:
#==============================================================================

install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}Targets
  LIBRARY
    COMPONENT Development
  FILE_SET ccfits_hdrs
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
    COMPONENT Development
)
write_basic_package_version_file(
  ${PROJECT_NAME}ConfigVersion.cmake
  VERSION ${PROJECT_NAME_VERSION}
  COMPATIBILITY AnyNewerVersion
)
export(
  EXPORT ${PROJECT_NAME}Targets
  FILE ${PROJECT_NAME}/${PROJECT_NAME}Targets.cmake
  NAMESPACE ${PROJECT_NAME}::
)
configure_file(cmake/${PROJECT_NAME}Config.cmake
  "${PROJECT_NAME}/${PROJECT_NAME}Config.cmake"
  COPYONLY
)

set(ConfigPackageLocation "${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME}")
install(
  EXPORT ${PROJECT_NAME}Targets
  FILE ${PROJECT_NAME}Targets.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${ConfigPackageLocation}
  COMPONENT Development
)
install(
  FILES cmake/${PROJECT_NAME}Config.cmake
  DESTINATION ${ConfigPackageLocation}
  COMPONENT Development
)

#==============================================================================
# Tests:
#==============================================================================

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)

  add_executable(ccfits_cookbook cookbook.cxx)
  set_target_properties(ccfits_cookbook
    PROPERTIES OUTPUT_NAME cookbook
  ) 
  include_directories(${CMAKE_SOURCE_DIR} "${CMAKE_SOURCE_DIR}/../")
  target_link_libraries(ccfits_cookbook ${PROJECT_NAME} ${CFITSIO_LIBRARIES})
  add_test(NAME ccfits_test COMMAND ccfits_cookbook)

  set(TEST_FILES file1.pha)
  file(COPY ${TEST_FILES} DESTINATION ${CCfits_BINARY_DIR})
  #file(
  #  COPY ${CMAKE_CURRENT_SOURCE_DIR}/testprog.tpt
  #  DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
  #)
endif()
