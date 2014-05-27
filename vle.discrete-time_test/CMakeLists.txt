CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(MyProject CXX C)
SET (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
ENABLE_TESTING()

SET(MODEL_MAJOR 0)
SET(MODEL_MINOR 1)
SET(MODEL_PATCH 0)
SET(MODEL_NAME "${PROJECT_NAME}-${MODEL_MAJOR}.${MODEL_MINOR}.${MODEL_PATCH}")
SET(VLE_NAME "${PROJECT_NAME}-${MODEL_MAJOR}.${MODEL_MINOR}")

##
## Options for compilation of package
##

OPTION(WITH_TEST "will build the test [default: ON]" ON)
OPTION(WITH_DOC "will compile doc and install it [default: OFF]" OFF)
OPTION(WITH_WARNINGS "will compile with g++ warnings [default: ON]" ON)

if (CMAKE_BUILD_TYPE STREQUAL "")
  SET(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build" FORCE)
endif ()

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_COMPILER_IS_GNUCXX)
  if (NOT WITH_WARNINGS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
  else (NOT WITH_WARNINGS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
  endif (NOT WITH_WARNINGS)
  if ("${CMAKE_BUILD_TYPE}" EQUAL "Debug" OR "${CMAKE_BUILD_TYPE}" EQUAL "RelWithDebInfo")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb3")
  endif ()
endif ()


##
## Modules
##

INCLUDE(CheckIncludeFileCXX)
INCLUDE(CheckIncludeFile)
INCLUDE(CheckLibraryExists)
INCLUDE(CMakeDetermineCCompiler)

##
## Check libraries with pkgconfig
##

set(VLE_DEBUG 0)
find_package(VLE REQUIRED)

##
## Check VLE's packages
##

set (VleCheckPackage_DIR "${CMAKE_SOURCE_DIR}/cmake")
FIND_PACKAGE(VleCheckPackage REQUIRED)
set (VleCheckAndDeclareGeneratedModels_DIR
  "${CMAKE_SOURCE_DIR}/cmake")
FIND_PACKAGE(VleCheckAndDeclareGeneratedModels REQUIRED)
SET(VLE_ABI_VERSION 1.1)

VLE_CHECK_PACKAGE(VLE_DISCRETE_TIME vle.discrete-time)
if (NOT VLE_DISCRETE_TIME_FOUND)
  message(SEND_ERROR "Missing vle.discrete-time")
endif (NOT VLE_DISCRETE_TIME_FOUND)

VLE_CHECK_PACKAGE(TEMPVAL vle.temporal-values)
if (NOT TEMPVAL_FOUND)
  message(SEND_ERROR "Missing vle.temporal-values")
endif (NOT TEMPVAL_FOUND)


VLE_CHECK_PACKAGE(FSA vle.extension.fsa)
if (NOT FSA_FOUND)
  message(SEND_ERROR "Missing vle.extension.fsa")
endif (NOT FSA_FOUND)

VLE_CHECK_PACKAGE(DSDEVS vle.extension.dsdevs)
if (NOT DSDEVS_FOUND)
  message(SEND_ERROR "Missing vle.extension.dsdevs")
endif (NOT DSDEVS_FOUND)

VLE_CHECK_PACKAGE(RECORD_READER record.reader)
if (NOT RECORD_READER_FOUND)
  message(SEND_ERROR "Missing record.reader")
endif (NOT RECORD_READER_FOUND)

VLE_CHECK_PACKAGE(RECORD_TESTER record.tester)
if (NOT RECORD_TESTER_FOUND)
  message(SEND_ERROR "Missing record.tester")
endif (NOT RECORD_TESTER_FOUND)


##
## Find boost
##

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREAD ON)
find_package(Boost COMPONENTS unit_test_framework date_time)

##
## Generate the doxygen
##

FIND_PACKAGE(Doxygen)
IF (DOXYGEN AND WITH_DOC)
  SET(DOXYGEN_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")
  SET(DOXYGEN_OUTPUT_MODELING_DIR "${PROJECT_BINARY_DIR}/doxygen/modeling")
  SET(DOXYGEN_OUTPUT_SOURCE_DIR "${PROJECT_BINARY_DIR}/doxygen/sources")
  CONFIGURE_FILE("cmake/doxygen-modeling.conf.in"
    "${PROJECT_BINARY_DIR}/doxygen-modeling.conf")
  CONFIGURE_FILE("cmake/doxygen-sources.conf.in"
    "${PROJECT_BINARY_DIR}/doxygen-sources.conf")
  FILE(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/doxygen")
  FILE(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/doxygen/modeling")
  FILE(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/doxygen/sources")

  ADD_CUSTOM_COMMAND(
    OUTPUT "${PROJECT_BINARY_DIR}/doxygen/modeling/index.html"
    DEPENDS "${PROJECT_BINARY_DIR}/doxygen-modeling.conf"
    COMMAND "${DOXYGEN}"
    ARGS "${PROJECT_BINARY_DIR}/doxygen-modeling.conf")

  ADD_CUSTOM_COMMAND(
    OUTPUT "${PROJECT_BINARY_DIR}/doxygen/sources/index.html"
    DEPENDS "${PROJECT_BINARY_DIR}/doxygen-sources.conf"
    COMMAND "${DOXYGEN}"
    ARGS "${PROJECT_BINARY_DIR}/doxygen-sources.conf")

  ADD_CUSTOM_TARGET(doc_modeling ALL DEPENDS
    "${PROJECT_BINARY_DIR}/doxygen-modeling.conf"
    "${PROJECT_BINARY_DIR}/doxygen/modeling/index.html" VERBATIM)
  ADD_CUSTOM_TARGET(doc_sources ALL DEPENDS
    "${PROJECT_BINARY_DIR}/doxygen-sources.conf"
    "${PROJECT_BINARY_DIR}/doxygen/sources/index.html" VERBATIM)

  INSTALL(DIRECTORY "${PROJECT_BINARY_DIR}/doxygen/modeling/html" DESTINATION
    "doc/html/modeling")
  INSTALL(DIRECTORY "${PROJECT_BINARY_DIR}/doxygen/sources/html" DESTINATION
    "doc/html/sources")
ENDIF (DOXYGEN AND WITH_DOC)

##
## Define function to simplify the definition of simulations plugins and test
##

FUNCTION(DeclareDevsDynamics name sources)
  INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/src
    ${VLE_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS})
  LINK_DIRECTORIES(
    ${VLE_LIBRARY_DIRS}
    ${Boost_LIBRARY_DIRS})
  ADD_LIBRARY(${name} MODULE ${sources})
  TARGET_LINK_LIBRARIES(${name} ${VLE_LIBRARIES} ${Boost_LIBRARIES})
  INSTALL(TARGETS ${name}
    RUNTIME DESTINATION plugins/simulator
    LIBRARY DESTINATION plugins/simulator)
ENDFUNCTION(DeclareDevsDynamics name sources)

FUNCTION(DeclareDiscreteTimeDynamics name sources)
  INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/src
    ${VLE_DISCRETE_TIME_INCLUDE_DIRS}
    ${TEMPVAL_INCLUDE_DIRS}
    ${VLE_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS})
  LINK_DIRECTORIES(
    ${VLE_LIBRARY_DIRS}
    ${Boost_LIBRARY_DIRS})
  ADD_LIBRARY(${name} MODULE ${sources})
  TARGET_LINK_LIBRARIES(${name} ${VLE_DISCRETE_TIME_LIBRARIES}
    ${TEMPVAL_LIBRARIES} ${VLE_LIBRARIES} ${Boost_LIBRARIES})
  INSTALL(TARGETS ${name}
    RUNTIME DESTINATION plugins/simulator
    LIBRARY DESTINATION plugins/simulator)
ENDFUNCTION(DeclareDiscreteTimeDynamics name sources)

FUNCTION(DeclareFsaDynamics name sources)
  INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/src
    ${FSA_INCLUDE_DIRS}
    ${VLE_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS})
  ADD_LIBRARY(${name} MODULE ${sources})
  TARGET_LINK_LIBRARIES(${name} ${FSA_LIBRARIES}
    ${VLE_LIBRARIES} ${Boost_LIBRARIES})
  INSTALL(TARGETS ${name}
    RUNTIME DESTINATION plugins/simulator
    LIBRARY DESTINATION plugins/simulator)
ENDFUNCTION(DeclareFsaDynamics name sources)

FUNCTION(DeclareDsDevsDynamics name sources)
  INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/src
    ${DSDEVS_INCLUDE_DIRS}
    ${VLE_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS})
  LINK_DIRECTORIES(
    ${VLE_LIBRARY_DIRS}
    ${Boost_LIBRARY_DIRS})
  ADD_LIBRARY(${name} MODULE ${sources})
  TARGET_LINK_LIBRARIES(${name} ${DSDEVS_LIBRARIES}
    ${VLE_LIBRARIES} ${Boost_LIBRARIES})
  INSTALL(TARGETS ${name}
    RUNTIME DESTINATION plugins/simulator
    LIBRARY DESTINATION plugins/simulator)
ENDFUNCTION(DeclareDsDevsDynamics name sources)

FUNCTION(DeclareVleTest name sources)
  message ("RECORD_TESTER_INCLUDE_DIRS ${RECORD_TESTER_INCLUDE_DIRS}")
  INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/src
    ${RECORD_READER_INCLUDE_DIRS}
    ${RECORD_TESTER_INCLUDE_DIRS}
    ${VLE_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS})
  LINK_DIRECTORIES(
    ${VLE_LIBRARY_DIRS}
    ${Boost_LIBRARY_DIRS})
  ADD_EXECUTABLE(${name} ${sources})
  TARGET_LINK_LIBRARIES(${name}
    ${VLE_LIBRARIES}
    ${RECORD_READER_LIBRARIES}
    ${RECORD_TESTER_LIBRARIES}
    ${Boost_LIBRARIES}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${Boost_DATE_TIME_LIBRARY})
  ADD_TEST(${name} ${name})
ENDFUNCTION(DeclareVleTest name sources)

##
## Subdirectory
##

add_subdirectory(data)
if (WITH_DOC)
  add_subdirectory(doc)
endif ()
add_subdirectory(exp)
add_subdirectory(src)

if (Boost_UNIT_TEST_FRAMEWORK_FOUND AND WITH_TEST)
  add_subdirectory(test)
endif (Boost_UNIT_TEST_FRAMEWORK_FOUND AND WITH_TEST)

##
## CPack configuration
##

INSTALL(FILES Authors.txt Description.txt License.txt News.txt Readme.txt
  DESTINATION .)

INCLUDE(CMakeCPack.cmake)
