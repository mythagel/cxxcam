FIND_PACKAGE(Boost 1.52.0 REQUIRED)
FIND_PACKAGE(Threads REQUIRED)

IF(UNIX)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
ENDIF()

SET(GEOM_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/deps/geom/include)
INCLUDE_DIRECTORIES(${CXXCAM_INCLUDE_DIR} ${GEOM_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
ADD_LIBRARY(cxxcam STATIC 
Axis.cpp 
GCodeWord.cpp 
Spindle.cpp 
GCodeLine.cpp 
Material.cpp 
Position.cpp 
Offset.cpp 
Limits.cpp 
Path.cpp 
Math.cpp 
Bbox.cpp
)
TARGET_LINK_LIBRARIES(cxxcam ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
