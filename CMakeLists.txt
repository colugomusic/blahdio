cmake_minimum_required(VERSION 3.18)
project(blahdio)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake/Modules)

include(CPM)
CPMUsePackageLock(package-lock.cmake)

# Header-only libraries #########################################
CPMAddPackage(
	NAME dr_libs
	GITHUB_REPOSITORY "mackron/dr_libs"
	GIT_TAG master
	DOWNLOAD_ONLY YES
)

CPMAddPackage(
	NAME miniaudio
	GITHUB_REPOSITORY "mackron/miniaudio"
	GIT_TAG master
	DOWNLOAD_ONLY YES
)

CPMAddPackage(
	NAME utfcpp
	GITHUB_REPOSITORY "nemtrif/utfcpp"
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
set(WAVPACK_BUILD_WINAMP_PLUGIN OFF CACHE BOOL "Build WinAmp plugin" FORCE)
set(WAVPACK_BUILD_PROGRAMS OFF CACHE BOOL "Build programs" FORCE)
set(WAVPACK_BUILD_DOCS OFF CACHE BOOL "Build documentation" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "Build tests" FORCE)
CPMAddPackage("gh:colugomusic/wavpack#master")
list(APPEND include_dirs ${wavpack_SOURCE_DIR}/include)
list(APPEND link_libs wavpack)
#xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

list(APPEND PUBLIC_HEADERS
    include/blahdio/audio_data_format.h
    include/blahdio/audio_reader.h
    include/blahdio/audio_writer.h
    include/blahdio/audio_type.h
)

list(APPEND SRC
	src/mackron/blahdio_dr_libs.h
	src/mackron/blahdio_dr_libs.cpp
	src/mackron/blahdio_miniaudio.h
	src/mackron/blahdio_miniaudio.cpp
	
    src/read/audio_reader.cpp
	src/read/generic_dr_libs_reader.h
	src/read/generic_dr_libs_reader.cpp
	src/read/generic_reader.h
	src/read/typed_read_handler.h
	src/read/typed_read_handler.cpp

	src/read/binary/binary_file_reader.h
	src/read/binary/binary_file_reader.cpp
	src/read/binary/binary_memory_reader.h
	src/read/binary/binary_memory_reader.cpp
	src/read/binary/binary_reader.h
	src/read/binary/binary_reader.cpp

	src/read/flac/flac_reader.h
	src/read/flac/flac_reader.cpp

	src/read/mp3/mp3_reader.h
	src/read/mp3/mp3_reader.cpp

	src/read/wav/wav_reader.h
	src/read/wav/wav_reader.cpp

	src/read/wavpack/wavpack_file_reader.h
	src/read/wavpack/wavpack_file_reader.cpp
	src/read/wavpack/wavpack_memory_reader.h
	src/read/wavpack/wavpack_memory_reader.cpp
	src/read/wavpack/wavpack_reader.h
	src/read/wavpack/wavpack_reader.cpp
	
    src/write/audio_writer.cpp
)

# VS source groups ##############################################
source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/include/blahdio PREFIX "public headers" FILES ${PUBLIC_HEADERS})
source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/src PREFIX "src" FILES ${SRC})
#xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

add_library(${PROJECT_NAME} STATIC ${PUBLIC_HEADERS} ${SRC})

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
