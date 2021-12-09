cmake_minimum_required(VERSION 3.16)
project(blahdio)

option(BLAHDIO_ENABLE_FLAC "Enable FLAC support" ON)
option(BLAHDIO_ENABLE_MP3 "Enable MP3 support" ON)
option(BLAHDIO_ENABLE_WAV "Enable WAV support" ON)
option(BLAHDIO_ENABLE_WAVPACK "Enable WavPack support" ON)
option(BLAHDIO_BUILD_TESTS "Build tests" OFF)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake/Modules)

include(CPM)

# Header-only libraries #########################################
if (BLAHDIO_ENABLE_FLAC OR BLAHDIO_ENABLE_MP3 OR BLAHDIO_ENABLE_WAV)
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
endif()

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
if (BLAHDIO_ENABLE_WAVPACK)
	set(WAVPACK_BUILD_COOLEDIT_PLUGIN OFF CACHE BOOL "Build CoolEdit plugin" FORCE)
	set(WAVPACK_BUILD_WINAMP_PLUGIN OFF CACHE BOOL "Build WinAmp plugin" FORCE)
	set(WAVPACK_BUILD_PROGRAMS OFF CACHE BOOL "Build programs" FORCE)
	set(WAVPACK_BUILD_DOCS OFF CACHE BOOL "Build documentation" FORCE)
	set(BUILD_TESTING OFF CACHE BOOL "Build tests" FORCE)

	# Disable ASM if building for arm64 (WavPack doesn't support it). I don't know
	# how to disable this for just one architecture. I assume it's not possible
	# because the WavPack CMakeLists fails to do it. So if we're building a fat binary
	# it will be disabled for all architectures
	if (CMAKE_SYSTEM_NAME STREQUAL Darwin AND "arm64" INLIST CMAKE_OSX_ARCHITECTURES)
		set(WAVPACK_ENABLE_ASM OFF CACHE BOOL "Enable assembly optimizations" FORCE)
	endif()

	CPMAddPackage("gh:dbry/wavpack#master")
	list(APPEND include_dirs ${wavpack_SOURCE_DIR}/include)
	list(APPEND link_libs wavpack)
endif()
#xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

list(APPEND PUBLIC_HEADERS
	include/blahdio/audio_data_format.h
	include/blahdio/audio_reader.h
	include/blahdio/audio_streamer.h
	include/blahdio/audio_writer.h
	include/blahdio/audio_type.h
	include/blahdio/library_info.h
)

list(APPEND SRC
	src/library_info.cpp

	src/read/audio_reader.cpp
	src/read/audio_reader_impl.h
	src/read/audio_reader_impl.cpp
	src/read/audio_streamer.cpp
	src/read/audio_streamer_impl.h
	src/read/audio_streamer_impl.cpp
	src/read/generic_reader.h
	src/read/typed_read_handler.h
	src/read/typed_read_handler.cpp

	src/read/binary/binary_file_reader.h
	src/read/binary/binary_file_reader.cpp
	src/read/binary/binary_memory_reader.h
	src/read/binary/binary_memory_reader.cpp
	src/read/binary/binary_reader.h
	src/read/binary/binary_reader.cpp
	src/read/binary/binary_stream_reader.h
	src/read/binary/binary_stream_reader.cpp
	
	src/write/audio_writer.cpp
	src/write/typed_write_handler.h
	src/write/typed_write_handler.cpp
)

if (BLAHDIO_ENABLE_FLAC)
	list(APPEND SRC
		src/read/flac/flac_reader.h
		src/read/flac/flac_reader.cpp
	)
endif()

if (BLAHDIO_ENABLE_MP3)
	list(APPEND SRC
		src/read/mp3/mp3_reader.h
		src/read/mp3/mp3_reader.cpp
	)
endif()

if (BLAHDIO_ENABLE_WAV)
	list(APPEND SRC
		src/read/wav/wav_reader.h
		src/read/wav/wav_reader.cpp
		src/write/wav/wav_writer.h
		src/write/wav/wav_writer.cpp
	)
endif()

if (BLAHDIO_ENABLE_WAVPACK)
	list(APPEND SRC
		src/read/wavpack/wavpack_file_reader.h
		src/read/wavpack/wavpack_file_reader.cpp
		src/read/wavpack/wavpack_memory_reader.h
		src/read/wavpack/wavpack_memory_reader.cpp
		src/read/wavpack/wavpack_reader.h
		src/read/wavpack/wavpack_reader.cpp
		src/read/wavpack/wavpack_stream_reader.h
		src/read/wavpack/wavpack_stream_reader.cpp
		src/write/wavpack/wavpack_writer.h
		src/write/wavpack/wavpack_writer.cpp
	)
endif()

if (BLAHDIO_ENABLE_FLAC OR BLAHDIO_ENABLE_MP3 OR BLAHDIO_ENABLE_WAV)
	list(APPEND SRC
		src/mackron/blahdio_dr_libs.h
		src/mackron/blahdio_dr_libs.cpp
		src/mackron/blahdio_miniaudio.cpp
	)
endif()

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
	MA_NO_WASAPI
	MA_NO_DSOUND
	MA_NO_WINMM
	MA_NO_ALSA
	MA_NO_PULSEAUDIO
	MA_NO_JACK
	MA_NO_COREAUDIO
	MA_NO_SNDIO
	MA_NO_AUDIO4
	MA_NO_OSS
	MA_NO_AAUDIO
	MA_NO_OPENSL
	MA_NO_WEBAUDIO
	MA_NO_NULL
	MA_NO_DECODING
	MA_NO_ENCODING
	MA_NO_WAV
	MA_NO_FLAC
	MA_NO_MP3
	MA_NO_DEVICE_IO
	MA_NO_THREADING
	MA_NO_GENERATION
	$<$<BOOL:${BLAHDIO_ENABLE_FLAC}>:BLAHDIO_ENABLE_FLAC>
	$<$<BOOL:${BLAHDIO_ENABLE_MP3}>:BLAHDIO_ENABLE_MP3>
	$<$<BOOL:${BLAHDIO_ENABLE_WAV}>:BLAHDIO_ENABLE_WAV>
	$<$<BOOL:${BLAHDIO_ENABLE_WAVPACK}>:BLAHDIO_ENABLE_WAVPACK>
)

if (BLAHDIO_BUILD_TESTS)

	include(CTest)
	enable_testing()

	add_subdirectory(tests)

	add_test(NAME tests
		COMMAND blahdio_tests
	)

endif()
