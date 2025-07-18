cmake_minimum_required(VERSION 3.16)
project(blahdio)

option(BLAHDIO_ENABLE_FLAC "Enable FLAC support" ON)
option(BLAHDIO_ENABLE_MP3 "Enable MP3 support" ON)
option(BLAHDIO_ENABLE_WAV "Enable WAV support" ON)
option(BLAHDIO_ENABLE_WAVPACK "Enable WavPack support" ON)
option(BLAHDIO_BUILD_TESTS "Build tests" OFF)

set(debug_build $<OR:$<CONFIG:Debug>,$<STREQUAL:${CMAKE_BUILD_TYPE},Debug}>>)
set(prebuilt-lib-config $<IF:${debug_build},debug,release>)

find_package(dr_libs REQUIRED)
find_package(miniaudio REQUIRED)
find_package(tl-expected REQUIRED)
find_package(utf8cpp REQUIRED)
if (BLAHDIO_ENABLE_WAVPACK)
	find_package(wavpack REQUIRED)
endif()

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

add_library(blahdio STATIC ${SRC})

target_sources(blahdio INTERFACE
	FILE_SET HEADERS
		BASE_DIRS
			include
		FILES
			include/blahdio/audio_data_format.h
			include/blahdio/audio_reader.h
			include/blahdio/audio_streamer.h
			include/blahdio/audio_writer.h
			include/blahdio/audio_type.h
			include/blahdio/expected.h
			include/blahdio/library_info.h
)

target_include_directories(blahdio PUBLIC
	include
)

target_include_directories(blahdio PRIVATE
	src
)

target_link_libraries(blahdio PUBLIC tl::expected utf8cpp)

if (BLAHDIO_ENABLE_WAVPACK)
	target_link_libraries(blahdio PUBLIC wavpack::wavpack)
endif()

if (BLAHDIO_ENABLE_FLAC OR BLAHDIO_ENABLE_MP3 OR BLAHDIO_ENABLE_WAV)
	target_link_libraries(blahdio PUBLIC dr_libs::dr_libs miniaudio::miniaudio)
endif()

set_target_properties(blahdio PROPERTIES
	CXX_STANDARD 20
)

target_compile_definitions(blahdio PRIVATE
	UTF_CPP_CPLUSPLUS=202002L
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

install(TARGETS blahdio
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	FRAMEWORK DESTINATION lib
	FILE_SET HEADERS
)

if (BLAHDIO_BUILD_TESTS)
	include(CTest)
	enable_testing()
	add_subdirectory(tests)
	add_test(NAME tests
		COMMAND blahdio_tests
	)
endif()
