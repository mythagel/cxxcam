# 1.52 Required for CGAL to play nice with C++11
FIND_PACKAGE(Boost 1.52.0 REQUIRED)
FIND_PACKAGE(CGAL REQUIRED) # 4.4 Minimim version
INCLUDE(${CGAL_USE_FILE})

IF(UNIX)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -frounding-math")
ENDIF()

add_definitions(-DCGAL_HAS_THREADS)

INCLUDE_DIRECTORIES(${CXXCAM_INCLUDE_DIR}/geom ${Boost_INCLUDE_DIRS})
ADD_LIBRARY(geom STATIC polyhedron.cpp primitives.cpp ops.cpp CSGPrimitives3D.cpp Point.cpp io.cpp explode.cpp translate.cpp query.cpp aabb_tree.cpp)
TARGET_LINK_LIBRARIES(geom ${CGAL_LIBRARIES} ${CGAL_3RD_PARTY_LIBRARIES} ${Boost_LIBRARIES})
