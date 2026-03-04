PROJECT(CCfits)
CMAKE_MINIMUM_REQUIRED(VERSION 3.8)

# Allow the developer to select whether to build Dynamic or Static libraries
OPTION (BUILD_SHARED_LIBS "Build Shared Libraries" OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}")
set (LIB_DESTINATION "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}")
set (BIN_DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
set (INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include/CCfits")

# Define project version
SET(${PROJECT_NAME}_MAJOR_VERSION 2)
SET(${PROJECT_NAME}_MINOR_VERSION 6)
SET(${PROJECT_NAME}_VERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION})

SET(LIB_NAME CCfits)
SET (LIB_TYPE STATIC)
IF (BUILD_SHARED_LIBS)
  SET (LIB_TYPE SHARED)
ENDIF (BUILD_SHARED_LIBS)

#==============================================================================
# Source code:
#==============================================================================

FILE(GLOB H_FILES "*.h")
SET(H_FILES ${H_FILES} CCfits)

SET(SRC_FILES
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

# Try to get CFITSIO info (incl. dependencies) using pkg-config:
SET (PKG_CONFIG_USE_CMAKE_PREFIX_PATH "on")
FIND_PACKAGE(PkgConfig)
IF (${PkgConfig_FOUND})
   PKG_CHECK_MODULES(CFITSIO REQUIRED cfitsio)
   IF (CFITSIO_FOUND)
      MESSAGE(STATUS "Found CFITSIO:")
      MESSAGE(STATUS "  CFITSIO_INCLUDE_DIRS = ${CFITSIO_INCLUDE_DIRS}")
      MESSAGE(STATUS "  CFITSIO_LIBRARY_DIRS = ${CFITSIO_LIBRARY_DIRS}")
      MESSAGE(STATUS "  CFITSIO_LIBRARIES = ${CFITSIO_LIBRARIES}")
      INCLUDE_DIRECTORIES(${CFITSIO_INCLUDE_DIRS})
      LINK_DIRECTORIES(${CFITSIO_LIBRARY_DIRS})
   ENDIF (CFITSIO_FOUND)
# If pkg-config not found, try to find CFITSIO; dependencies e.g.
# zlib & curl are really only needed here if CFITSIO is static.
ELSE()
   FIND_PACKAGE(CFITSIO REQUIRED)
   FIND_PACKAGE(ZLIB REQUIRED)
   IF (NOT WIN32)
      FIND_PACKAGE(CURL)
   ENDIF (NOT WIN32)
ENDIF (${PkgConfig_FOUND})

#==============================================================================
# Target:
#==============================================================================

ADD_LIBRARY(${LIB_NAME} ${LIB_TYPE} ${H_FILES} ${SRC_FILES})
TARGET_COMPILE_FEATURES(${LIB_NAME} PRIVATE cxx_std_11)
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES
    VERSION ${${PROJECT_NAME}_VERSION}
)

# Visual Studio:
IF (MSVC)
   ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
   IF (BUILD_SHARED_LIBS)
      # MSVC shared is broken; better fix would be to "hide all symbols
      # for all compilers", and export public symbols with a macro.
      SET_PROPERTY(TARGET ${LIB_NAME} PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS ON)
   ENDIF (BUILD_SHARED_LIBS)
ENDIF(MSVC)

# Add link dependencies:
IF (${PkgConfig_FOUND})
   # pkg-config supplies the CFITSIO dependencies (zlib, curl)
   IF (CFITSIO_FOUND)
      TARGET_LINK_LIBRARIES(${LIB_NAME} ${CFITSIO_LIBRARIES})
   ENDIF (CFITSIO_FOUND)
ELSE()
   # pkg-config wasn't found, so add everything individually:
   INCLUDE_DIRECTORIES(${CFITSIO_INCLUDE_DIR})
   TARGET_LINK_LIBRARIES(${LIB_NAME} ${CFITSIO_LIBRARY})
   TARGET_LINK_LIBRARIES(${LIB_NAME} ${ZLIB_LIBRARIES})
   IF (CURL_FOUND)
      TARGET_LINK_LIBRARIES(${LIB_NAME} ${CURL_LIBRARIES})
   ENDIF (CURL_FOUND)
ENDIF (${PkgConfig_FOUND})

#==============================================================================
# Install:
#==============================================================================
include(GNUInstallDirs)
install(TARGETS ${LIB_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(FILES ${H_FILES} DESTINATION ${INCLUDE_INSTALL_DIR})

#==============================================================================
# Tests:
#==============================================================================
IF (TESTS)

    ENABLE_TESTING()

    ADD_EXECUTABLE(cookbook cookbook.cxx)
    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR} "${CMAKE_SOURCE_DIR}/../")
    TARGET_LINK_LIBRARIES(cookbook ${LIB_NAME} ${CFITSIO_LIBRARIES})
    ADD_TEST(cookbook cookbook)

    SET(TEST_FILES file1.pha)
    FILE(COPY ${TEST_FILES} DESTINATION ${CCfits_BINARY_DIR})
    #FILE(COPY ${CMAKE_SOURCE_DIR}/testprog.tpt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

ENDIF(TESTS)
