ADD_SUBDIRECTORY(svg_path)

FIND_PACKAGE(Boost 1.52.0 REQUIRED)
FIND_PACKAGE(Threads REQUIRED)

IF(UNIX)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
ENDIF()

SET(GEOM_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/deps/geom/include)
INCLUDE_DIRECTORIES(${CXXCAM_INCLUDE_DIR} ${GEOM_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
ADD_LIBRARY(cxxcam STATIC 
Axis.cpp 
GCode.cpp 
GCodeWord.cpp 
MachineState.cpp 
Spindle.cpp 
Tool.cpp 
GCodeBlock.cpp 
GCodeLine.cpp 
Machine.cpp 
Material.cpp 
Position.cpp 
Stock.cpp 
ToolTable.cpp 
Offset.cpp 
Limits.cpp 
Configuration.cpp 
Path.cpp 
Math.cpp 
Simulation.cpp 
Bbox.cpp
)
TARGET_LINK_LIBRARIES(cxxcam geom ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
