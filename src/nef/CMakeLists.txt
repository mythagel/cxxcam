# Boost is not a direct dependency
# Required for CGAL to play nice with C++11
FIND_PACKAGE(Boost 1.52.0 REQUIRED)
FIND_PACKAGE(CGAL REQUIRED)
INCLUDE(${CGAL_USE_FILE})

IF(UNIX)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -frounding-math")
ENDIF()

INCLUDE_DIRECTORIES(${CXXCAM_INCLUDE_DIR}/nef ${Boost_INCLUDE_DIRS})
ADD_LIBRARY(nef STATIC polyhedron.cpp primitives.cpp ops.cpp CSGPrimitives3D.cpp Point.cpp)
TARGET_LINK_LIBRARIES(nef ${CGAL_LIBRARIES} ${CGAL_3RD_PARTY_LIBRARIES})
