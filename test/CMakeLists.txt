
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src)

MACRO(CXXCAM_TEST testname)
	ADD_EXECUTABLE(${testname} EXCLUDE_FROM_ALL "${testname}.cpp")
	TARGET_LINK_LIBRARIES(${testname} cxxcam)
	SET_TARGET_PROPERTIES(${testname} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
	ADD_TEST(${testname} "${MAKE_CURRENT_BINARY_DIR}/${testname}")
	ADD_DEPENDENCIES(check ${testname})
ENDMACRO()

FOREACH(test 
mill_basic 
lathe_basic 
spindle 
nef_basic 
nef_tools 
nef_cube 
nef_cube2 
nef_sphere 
nef_cube_io 
nef_sphere_io 
nef_cylinder 
nef_translate 
nef_explode 
nef_nonmanifold 
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
)
	CXXCAM_TEST(${test})
ENDFOREACH()

