
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src)

MACRO(MAKE_TEST testname)
	ADD_EXECUTABLE(${testname} EXCLUDE_FROM_ALL "${testname}.cpp")
	TARGET_LINK_LIBRARIES(${testname} cxxcam)
	SET_TARGET_PROPERTIES(${testname} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
	ADD_TEST(${testname} "${MAKE_CURRENT_BINARY_DIR}/${testname}")
	ADD_DEPENDENCIES(check ${testname})
ENDMACRO()

FOREACH(test mill_basic lathe_basic spindle nef_basic nef_models)
	MAKE_TEST(${test})
ENDFOREACH()

