
IF(UNIX)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra")
ENDIF()

add_definitions(-DAA -DBB -DCC)

#INCLUDE_DIRECTORIES( )
ADD_LIBRARY(rs274ngc STATIC rs274ngc_errors.cc rs274ngc_pre.cc)
TARGET_LINK_LIBRARIES(rs274ngc m)
