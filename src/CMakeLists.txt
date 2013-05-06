ADD_SUBDIRECTORY(nef)

IF(UNIX)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
ENDIF()

INCLUDE_DIRECTORIES(${CXXCAM_INCLUDE_DIR})
ADD_LIBRARY(cxxcam STATIC Axis.cpp GCode.cpp GCodeWord.cpp MachineState.cpp Spindle.cpp Tool.cpp GCodeBlock.cpp GCodeLine.cpp Machine.cpp Material.cpp Position.cpp Stock.cpp ToolTable.cpp)
TARGET_LINK_LIBRARIES(cxxcam nef)
