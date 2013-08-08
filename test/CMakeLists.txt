
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src)

MACRO(CXXCAM_TEST testname)
	ADD_EXECUTABLE(${testname} EXCLUDE_FROM_ALL "${testname}.cpp")
	TARGET_LINK_LIBRARIES(${testname} cxxcam)
	SET_TARGET_PROPERTIES(${testname} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
	ADD_TEST(${testname} "${MAKE_CURRENT_BINARY_DIR}/${testname}")
	ADD_DEPENDENCIES(check ${testname})
ENDMACRO()

find_package(Boost COMPONENTS unit_test_framework)
IF(${Boost_UNIT_TEST_FRAMEWORK_FOUND})
	MACRO(BOOST_TEST testname)
		ADD_EXECUTABLE(${testname} EXCLUDE_FROM_ALL "${testname}.cpp")
		TARGET_LINK_LIBRARIES(${testname} cxxcam ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
		SET_TARGET_PROPERTIES(${testname} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
		ADD_TEST(${testname} "${MAKE_CURRENT_BINARY_DIR}/${testname}")
		ADD_DEPENDENCIES(check ${testname})
	ENDMACRO()
ELSE()
	MESSAGE(STATUS "Boost UTF missing; Skipping boost tests.")
	MACRO(BOOST_TEST testname)
	ENDMACRO()
ENDIF()

FOREACH(test 
mill_basic 
lathe_basic 
spindle 
geom_basic 
geom_tools 
geom_cube 
geom_cube2 
geom_sphere 
geom_cube_io 
geom_sphere_io 
geom_cylinder 
geom_translate 
geom_explode 
geom_nonmanifold 
geom_validate_rotations 
geom_validate_boolean 
model 
pushpop_state 
limits 
axes 
configuration 
inverse_time 
path 
math 
simulation 
simulate_path 
simulate_path-test2 
simulate_path-nintynonmanifold 
simulate_rotary 
simulate_arc 
simulate_arc_rotary 
sim_low_angular_res 
simulate_cut_air 
fold_adjacent 
playjs 
helix 
bbox 
play 
ex_newsim 
)
	CXXCAM_TEST(${test})
ENDFOREACH()

