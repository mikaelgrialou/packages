include_directories(${CMAKE_SOURCE_DIR}/src ${VLE_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS} ${CELLDEVS_INCLUDE_DIRS} ${CELLQSS_INCLUDE_DIRS}
  ${DECISION_INCLUDE_DIRS} ${DIFFERENCE_EQU_INCLUDE_DIRS}
  ${DIFFERENTIAL_EQU_INCLUDE_DIRS} ${DSDEVS_INCLUDE_DIRS} ${FSA_INCLUDE_DIRS}
  ${PETRINET_INCLUDE_DIRS})

link_directories(${VLE_LIBRARY_DIRS} ${CELLDEVS_LIBRARY_DIRS}
  ${CELLQSS_LIBRARY_DIRS} ${DECISION_LIBRARY_DIRS}
  ${DIFFERENCE_EQU_LIBRARY_DIRS} ${DIFFERENTIAL_EQU_LIBRARY_DIRS}
  ${DSDEVS_LIBRARY_DIRS} ${FSA_LIBRARY_DIRS} ${PETRINET_LIBRARY_DIRS})

DeclareDevsDynamics(DecisionCounter DecisionCounter.cpp)
DeclareDevsDynamics(DecisionRetarder DecisionRetarder.cpp)
DeclareDevsDynamics(DecisionStart DecisionStart.cpp)
DeclareDecisionDynamics(OnlyAgent OnlyAgent.cpp)
DeclareDecisionDynamics(OnlyAgentPrecedenceConstraint
  OnlyAgentPrecedenceConstraint.cpp)
DeclareDecisionDynamics(OnlyAgentWakeUp OnlyAgentWakeUp.cpp)
DeclareDecisionDynamics(SimpleAgent SimpleAgent.cpp)
