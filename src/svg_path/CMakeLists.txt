
IF(UNIX)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra")
ENDIF()

INCLUDE_DIRECTORIES(${CXXCAM_INCLUDE_DIR}/svg_path)
ADD_LIBRARY(svg_path STATIC basic.cpp numberlist.cpp path.cpp transform.cpp)
