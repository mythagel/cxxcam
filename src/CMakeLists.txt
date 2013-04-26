# Boost is not a direct dependency
# Required for CGAL to play nice with C++11
FIND_PACKAGE(Boost 1.52.0 REQUIRED)
FIND_PACKAGE(CGAL REQUIRED)
INCLUDE(${CGAL_USE_FILE})

IF(UNIX)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++11")
ENDIF()

INCLUDE_DIRECTORIES(${PROJECT_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})

ADD_LIBRARY(${PROJECT_NAME} SHARED Axis.cpp GCode.cpp GCodeWord.cpp MachineState.cpp nef_polyhedron.cpp Spindle.cpp Tool.cpp GCodeBlock.cpp GCodeLine.cpp Machine.cpp Material.cpp Position.cpp Stock.cpp ToolTable.cpp)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${CGAL_LIBRARIES} ${CGAL_3RD_PARTY_LIBRARIES})
