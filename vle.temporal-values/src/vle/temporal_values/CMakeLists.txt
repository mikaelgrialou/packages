INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src ${VLE_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS} ${CMAKE_BINARY_DIR}/src)

LINK_DIRECTORIES(${VLE_LIBRARY_DIRS} ${Boost_LIBRARY_DIRS})

ADD_LIBRARY(temporal_values STATIC TemporalValues.hpp TemporalValues.cpp)

IF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
  if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_COMPILER_IS_GNUCXX)
    SET_TARGET_PROPERTIES(temporal_values PROPERTIES COMPILE_FLAGS -fPIC)
  endif ()
ENDIF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")

SET_TARGET_PROPERTIES(temporal_values PROPERTIES OUTPUT_NAME
  "temporal_values-${MODEL_MAJOR}.${MODEL_MINOR}")

TARGET_LINK_LIBRARIES(temporal_values ${VLE_LIBRARIES} ${Boost_LIBRARIES})

INSTALL(TARGETS temporal_values ARCHIVE DESTINATION lib)

install(FILES TemporalValues.hpp
  DESTINATION src/vle/temporal_values)

#CONFIGURE_FILE(Version.hpp.in
#  ${CMAKE_BINARY_DIR}/src/vle/extension/decision/Version.hpp)
#
#INSTALL(FILES
#  ${CMAKE_BINARY_DIR}/src/vle/extension/decision/Version.hpp
#  DESTINATION src/vle/extension/decision)
