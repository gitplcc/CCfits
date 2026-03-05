#.rst:
# Findcfitsio
# -----------
#
# Finds the cfitsio library
#
# This will define the following variables::
#
#   cfitsio_FOUND    - True if the system has the Foo library
#   cfitsio_VERSION  - The version of the Foo library which was found
#
# and the following imported targets::
#
#   CFITSIO::cfitsio - The cfitsio library
#=============================================================================

set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)
find_package(PkgConfig QUIET)
if(${PkgConfig_FOUND})
  pkg_check_modules(cfitsio REQUIRED IMPORTED_TARGET cfitsio)
  if(cfitsio_FOUND)
    add_library(CFITSIO::cfitsio ALIAS PkgConfig::cfitsio)
  endif()
else()
  find_path(cfitsio_INCLUDE_DIR
    NAMES fitsio.h
    PATH_SUFFIXES cfitsio
    REQUIRED
  )
  find_library(cfitsio_LIBRARY NAMES cfitsio REQUIRED)
  file(
    STRINGS "${cfitsio_INCLUDE_DIR}/fitsio.h" cfitsio_VERSION_LINE
    REGEX "#define[ \t]+CFITSIO_VERSION"
  )
  string(
    REGEX MATCH "[0-9]+\.[0-9]+\.[0-9]+$"
    cfitsio_VERSION
    ${cfitsio_VERSION_LINE}
  )

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(cfitsio
    FOUND_VAR cfitsio_FOUND
    REQUIRED_VARS
      cfitsio_LIBRARY
      cfitsio_INCLUDE_DIR
    VERSION_VAR cfitsio_VERSION
  )
  if(cfitsio_FOUND AND NOT TARGET CFITSIO::cfitsio)
    add_library(CFITSIO::cfitsio UNKNOWN IMPORTED)
    set_target_properties(CFITSIO::cfitsio PROPERTIES
      IMPORTED_LOCATION "${cfitsio_LIBRARY}"
      INTERFACE_COMPILE_OPTIONS "-D_REENTRANT"
      INTERFACE_INCLUDE_DIRECTORIES "${cfitsio_INCLUDE_DIR}"
    )
    # zlib & curl only needed if CFITSIO is static
    find_package(ZLIB REQUIRED QUIET)
    find_package(CURL QUIET)
    add_dependencies(CFITSIO::cfitsio ZLIB::ZLIB)
    if(CURL_FOUND)
      add_dependencies(CFITSIO::cfitsio CURL::libcurl)
    endif()
  endif()
endif()
