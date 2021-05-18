cmake_minimum_required(VERSION 3.18)
project(blahdio_tests)

CPMAddPackage(
	NAME catch2
	GITHUB_REPOSITORY "catchorg/Catch2"
	GIT_TAG v2.x
	DOWNLOAD_ONLY
)

list(APPEND TESTS_SRC
	src/main.cpp
	src/util.h
	src/util.cpp

	src/write_read_compare.cpp
)

add_executable(${PROJECT_NAME} ${TESTS_SRC})

target_include_directories(${PROJECT_NAME} PRIVATE
	${catch2_SOURCE_DIR}/single_include
	${CMAKE_CURRENT_LIST_DIR}/../include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	blahdio
)

set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 17
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
	DIR_TEST_FILES="test_files"
)
