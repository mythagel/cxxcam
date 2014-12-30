
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/deps/geom/include ${PROJECT_SOURCE_DIR}/src)

MACRO(CXXCAM_TEST testname)
	ADD_EXECUTABLE(${testname} EXCLUDE_FROM_ALL "${testname}.cpp")
	TARGET_LINK_LIBRARIES(${testname} cxxcam svg_path)
	SET_TARGET_PROPERTIES(${testname} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
	ADD_TEST(${testname} "${MAKE_CURRENT_BINARY_DIR}/${testname}")
	ADD_DEPENDENCIES(check ${testname})
ENDMACRO()

FOREACH(test 
spindle 
test_limits 
axes 
path 
math 
fold_adjacent 
helix 
bbox 
ex_trochoid 
ex_rate 
test_svg_parser_numberlist
test_svg_path
test_svg_transform
)
	CXXCAM_TEST(${test})
ENDFOREACH()

