#INCLUDE_DIRECTORIES(
#  ${CMAKE_SOURCE_DIR}/src
#  ${VLE_INCLUDE_DIRS}
#  ${Boost_INCLUDE_DIRS})

#LINK_DIRECTORIES(
#  ${VLE_LIBRARY_DIRS}
#  ${Boost_LIBRARY_DIRS})

#ADD_LIBRARY(RecurrenceRelationGen SHARED RecurrenceRelationGen.cpp)

#TARGET_LINK_LIBRARIES(RecurrenceRelationGen
#  ${VLE_LIBRARIES}
#  ${Boost_LIBRARIES}
#  recurrence-relation)

#INSTALL(TARGETS RecurrenceRelationGen
#  RUNTIME DESTINATION plugins/simulator
#  LIBRARY DESTINATION plugins/simulator
#  ARCHIVE DESTINATION plugins/simulator)