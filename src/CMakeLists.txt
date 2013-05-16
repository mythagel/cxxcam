ADD_SUBDIRECTORY(nef)

INCLUDE_DIRECTORIES(${CXXCAM_INCLUDE_DIR})
ADD_LIBRARY(cxxcam STATIC Axis.cpp GCode.cpp GCodeWord.cpp MachineState.cpp Spindle.cpp Tool.cpp GCodeBlock.cpp GCodeLine.cpp Machine.cpp Material.cpp Position.cpp Stock.cpp ToolTable.cpp Offset.cpp)
TARGET_LINK_LIBRARIES(cxxcam nef)
