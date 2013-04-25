IF(UNIX)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++11")
ENDIF()

ADD_LIBRARY(${PROJECT_NAME} SHARED Axis.cpp GCode.cpp GCodeWord.cpp MachineState.cpp nef_polyhedron.cpp Spindle.cpp Tool.cpp GCodeBlock.cpp GCodeLine.cpp Machine.cpp Material.cpp Position.cpp Stock.cpp ToolTable.cpp)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} CGAL gmp mpfr boost_thread-mt boost_system-mt)
