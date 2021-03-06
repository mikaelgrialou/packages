CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(vle.examples CXX C)
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

if (CMAKE_BUILD_TYPE STREQUAL "")
  SET(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build" FORCE)
endif ()

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

  if ("${CMAKE_BUILD_TYPE}" EQUAL "Debug" OR "${CMAKE_BUILD_TYPE}" EQUAL "RelWithDebInfo")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb3")
  endif ()
endif ()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH})

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

set(VleCheckPackage_DIR "${CMAKE_SOURCE_DIR}/cmake")
FIND_PACKAGE(VleCheckPackage REQUIRED)
SET(VLE_ABI_VERSION 1.1)

VLE_CHECK_PACKAGE(CELLDEVS vle.extension.celldevs)
VLE_CHECK_PACKAGE(CELLQSS vle.extension.cellqss)
VLE_CHECK_PACKAGE(DECISION vle.extension.decision)
VLE_CHECK_PACKAGE(DIFFERENCE_EQU vle.extension.difference-equation)
VLE_CHECK_PACKAGE(DIFFERENTIAL_EQU vle.extension.differential-equation)
VLE_CHECK_PACKAGE(DSDEVS vle.extension.dsdevs)
VLE_CHECK_PACKAGE(FSA vle.extension.fsa)
VLE_CHECK_PACKAGE(PETRINET vle.extension.petrinet)

if (NOT CELLDEVS_FOUND)
  message(SEND_ERROR "Missing vle.extension.celldevs")
endif (NOT CELLDEVS_FOUND)
if (NOT CELLQSS_FOUND)
  message(SEND_ERROR "Missing vle.extension.cellqss")
endif (NOT CELLQSS_FOUND)
if (NOT DECISION_FOUND)
  message(SEND_ERROR "Missing vle.extension.decision")
endif (NOT DECISION_FOUND)
if (NOT DIFFERENCE_EQU_FOUND)
  message(SEND_ERROR "Missing vle.extension.difference-equation")
endif (NOT DIFFERENCE_EQU_FOUND)
if (NOT DIFFERENTIAL_EQU_FOUND)
  message(SEND_ERROR "Missing vle.extension.differential-equation")
endif (NOT DIFFERENTIAL_EQU_FOUND)
if (NOT DSDEVS_FOUND)
  message(SEND_ERROR "Missing vle.extension.dsdevs")
endif (NOT DSDEVS_FOUND)
if (NOT FSA_FOUND)
  message(SEND_ERROR "Missing vle.extension.fsa")
endif (NOT FSA_FOUND)
if (NOT PETRINET_FOUND)
  message(SEND_ERROR "Missing vle.extension.petrinet")
endif (NOT PETRINET_FOUND)

##
## Find boost
##

SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)
FIND_PACKAGE(Boost COMPONENTS unit_test_framework date_time)

IF (Boost_UNIT_TEST_FRAMEWORK_FOUND)
  SET(HAVE_UNITTESTFRAMEWORK 1 CACHE INTERNAL "" FORCE)
ENDIF (Boost_UNIT_TEST_FRAMEWORK_FOUND)

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
## Define function to simplify the definition of simulations plugins.
##

FUNCTION(DeclareDevsDynamics name sources)
  ADD_LIBRARY(${name} MODULE ${sources})
  TARGET_LINK_LIBRARIES(${name} ${VLE_LIBRARIES}
    ${Boost_LIBRARIES})
  INSTALL(TARGETS ${name}
    RUNTIME DESTINATION plugins/simulator
    LIBRARY DESTINATION plugins/simulator)
ENDFUNCTION(DeclareDevsDynamics name sources)

FUNCTION(DeclareCellDevsDynamics name sources)
  ADD_LIBRARY(${name} MODULE ${sources})
  TARGET_LINK_LIBRARIES(${name} ${VLE_LIBRARIES} ${CELLDEVS_LIBRARIES}
    ${Boost_LIBRARIES})
  INSTALL(TARGETS ${name}
    RUNTIME DESTINATION plugins/simulator
    LIBRARY DESTINATION plugins/simulator)
ENDFUNCTION(DeclareCellDevsDynamics name sources)

FUNCTION(DeclareCellQssDynamics name sources)
  ADD_LIBRARY(${name} MODULE ${sources})
  TARGET_LINK_LIBRARIES(${name} ${VLE_LIBRARIES} ${CELLQSS_LIBRARIES}
    ${Boost_LIBRARIES})
  INSTALL(TARGETS ${name}
    RUNTIME DESTINATION plugins/simulator
    LIBRARY DESTINATION plugins/simulator)
ENDFUNCTION(DeclareCellQssDynamics name sources)

FUNCTION(DeclareDecisionDynamics name sources)
  ADD_LIBRARY(${name} MODULE ${sources})
  TARGET_LINK_LIBRARIES(${name} ${VLE_LIBRARIES} ${DECISION_LIBRARIES}
    ${Boost_LIBRARIES})
  INSTALL(TARGETS ${name}
    RUNTIME DESTINATION plugins/simulator
    LIBRARY DESTINATION plugins/simulator)
ENDFUNCTION(DeclareDecisionDynamics name sources)

FUNCTION(DeclareDifferenceEquationDynamics name sources)
  ADD_LIBRARY(${name} MODULE ${sources})
  TARGET_LINK_LIBRARIES(${name} ${VLE_LIBRARIES} ${DIFFERENCE_EQU_LIBRARIES}
    ${Boost_LIBRARIES})
  INSTALL(TARGETS ${name}
    RUNTIME DESTINATION plugins/simulator
    LIBRARY DESTINATION plugins/simulator)
ENDFUNCTION(DeclareDifferenceEquationDynamics name sources)

FUNCTION(DeclareFsaDifferenceEquationDynamics name sources)
  ADD_LIBRARY(${name} MODULE ${sources})
  TARGET_LINK_LIBRARIES(${name} ${VLE_LIBRARIES} ${DIFFERENCE_EQU_LIBRARIES}
    ${FSA_LIBRARIES} ${Boost_LIBRARIES})
  INSTALL(TARGETS ${name}
    RUNTIME DESTINATION plugins/simulator
    LIBRARY DESTINATION plugins/simulator)
ENDFUNCTION(DeclareFsaDifferenceEquationDynamics name sources)

FUNCTION(DeclareDifferentialEquationDynamics name sources)
  ADD_LIBRARY(${name} MODULE ${sources})
  TARGET_LINK_LIBRARIES(${name} ${VLE_LIBRARIES} ${DIFFERENTIAL_EQU_LIBRARIES}
    ${Boost_LIBRARIES})
  INSTALL(TARGETS ${name}
    RUNTIME DESTINATION plugins/simulator
    LIBRARY DESTINATION plugins/simulator)
ENDFUNCTION(DeclareDifferentialEquationDynamics name sources)

FUNCTION(DeclareDsDevsDynamics name sources)
  ADD_LIBRARY(${name} MODULE ${sources})
  TARGET_LINK_LIBRARIES(${name} ${VLE_LIBRARIES} ${DSDEVS_LIBRARIES}
    ${Boost_LIBRARIES})
  INSTALL(TARGETS ${name}
    RUNTIME DESTINATION plugins/simulator
    LIBRARY DESTINATION plugins/simulator)
ENDFUNCTION(DeclareDsDevsDynamics name sources)

FUNCTION(DeclareFsaDynamics name sources)
  ADD_LIBRARY(${name} MODULE ${sources})
  TARGET_LINK_LIBRARIES(${name} ${VLE_LIBRARIES} ${FSA_LIBRARIES}
    ${Boost_LIBRARIES})
  INSTALL(TARGETS ${name}
    RUNTIME DESTINATION plugins/simulator
    LIBRARY DESTINATION plugins/simulator)
ENDFUNCTION(DeclareFsaDynamics name sources)

FUNCTION(DeclarePetrinetDynamics name sources)
  ADD_LIBRARY(${name} MODULE ${sources})
  TARGET_LINK_LIBRARIES(${name} ${VLE_LIBRARIES} ${PETRINET_LIBRARIES}
    ${Boost_LIBRARIES})
  INSTALL(TARGETS ${name}
    RUNTIME DESTINATION plugins/simulator
    LIBRARY DESTINATION plugins/simulator)
ENDFUNCTION(DeclarePetrinetDynamics name sources)

##
## Subdirectory
##

ADD_SUBDIRECTORY(data)
IF(WITH_DOC)
  ADD_SUBDIRECTORY(doc)
ENDIF(WITH_DOC)
ADD_SUBDIRECTORY(exp)
ADD_SUBDIRECTORY(src)

IF (Boost_UNIT_TEST_FRAMEWORK_FOUND AND WITH_TEST)
  ADD_SUBDIRECTORY(test)
ENDIF (Boost_UNIT_TEST_FRAMEWORK_FOUND AND WITH_TEST)

##
## CPack configuration
##

INSTALL(FILES Authors.txt Description.txt License.txt News.txt Readme.txt
  DESTINATION .)

INCLUDE(CMakeCPack.cmake)
