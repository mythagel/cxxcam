
IF(UNIX)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra")
ENDIF()

#INCLUDE_DIRECTORIES( )
ADD_LIBRARY(rs274ngc STATIC rs274ngc_errors.cc rs274ngc_pre.cc block.cpp)
TARGET_LINK_LIBRARIES(rs274ngc m)

ADD_EXECUTABLE(driver driver2.cc)
TARGET_LINK_LIBRARIES(driver rs274ngc)

