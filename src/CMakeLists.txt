ADD_SUBDIRECTORY(nef)

FIND_PACKAGE(Boost 1.52.0 REQUIRED)

IF(UNIX)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
ENDIF()

INCLUDE_DIRECTORIES(${CXXCAM_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
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
)
TARGET_LINK_LIBRARIES(cxxcam nef)
