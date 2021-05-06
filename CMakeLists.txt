cmake_minimum_required(VERSION 3.18)
project(blahdio)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake/Modules)

include(CPM)
CPMUsePackageLock(package-lock.cmake)

# Header-only libraries #########################################
CPMAddPackage("gh:mackron/dr_libs#master")
CPMAddPackage("gh:mackron/miniaudio#master")

CPMAddPackage(
	NAME utfcpp
	GITHUB_REPOSITORY nemtrif/utfcpp
	GIT_TAG master
	DOWNLOAD_ONLY YES
)

list(APPEND include_dirs
    ${dr_libs_SOURCE_DIR}
    ${miniaudio_SOURCE_DIR}
    ${utfcpp_SOURCE_DIR}/source
)
#xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

# WavPack #######################################################
set(WAVPACK_BUILD_COOLEDIT_PLUGIN OFF CACHE BOOL "Build CoolEdit plugin" FORCE)
set(WAVPACK_BUILD_PROGRAMS OFF CACHE BOOL "Build programs" FORCE)
CPMAddPackage("gh:colugomusic/wavpack#master")
list(APPEND include_dirs ${wavpack_SOURCE_DIR}/include)
list(APPEND link_libs wavpack)
#xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

list(APPEND SRC
    include/blahdio/audio_data_format.h
    include/blahdio/audio_reader.h
    include/blahdio/audio_writer.h
    include/blahdio/audio_type.h

	src/generic_reader.h
    src/audio_reader.cpp
    src/audio_writer.cpp
    src/dr_libs/dr_libs_impl.cpp
	src/dr_libs/dr_libs_utils.h
	src/dr_libs/dr_libs_utils.cpp
	src/miniaudio/blahdio_miniaudio.h
	src/miniaudio/blahdio_miniaudio.cpp
	src/binary/binary_file_reader.h
	src/binary/binary_file_reader.cpp
	src/binary/binary_memory_reader.h
	src/binary/binary_memory_reader.cpp
	src/binary/binary_reader.h
	src/binary/binary_reader.cpp
	src/wavpack/wavpack_file_reader.h
	src/wavpack/wavpack_file_reader.cpp
	src/wavpack/wavpack_memory_reader.h
	src/wavpack/wavpack_memory_reader.cpp
	src/wavpack/wavpack_reader.h
	src/wavpack/wavpack_reader.cpp
)

add_library(${PROJECT_NAME} STATIC ${SRC})

target_include_directories(${PROJECT_NAME} PUBLIC
    include
    ${include_dirs}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${link_libs}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
	UTF_CPP_CPLUSPLUS=201703L
)