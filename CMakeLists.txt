cmake_minimum_required(VERSION 3.30)
project(blahdio)

option(BLAHDIO_ENABLE_FLAC "Enable FLAC support" ON)
option(BLAHDIO_ENABLE_MP3 "Enable MP3 support" ON)
option(BLAHDIO_ENABLE_WAV "Enable WAV support" ON)
option(BLAHDIO_ENABLE_WAVPACK "Enable WavPack support" ON)
option(BLAHDIO_BUILD_TESTS "Build tests" OFF)

find_package(tl-expected REQUIRED CONFIG)
find_package(utf8cpp REQUIRED CONFIG)

add_library(blahdio)
add_library(blahdio::blahdio ALIAS blahdio)

target_sources(blahdio PUBLIC
	FILE_SET HEADERS
	BASE_DIRS
		${CMAKE_CURRENT_SOURCE_DIR}/include
	FILES
		${CMAKE_CURRENT_SOURCE_DIR}/include/blahdio/audio_data_format.h
		${CMAKE_CURRENT_SOURCE_DIR}/include/blahdio/audio_reader.h
		${CMAKE_CURRENT_SOURCE_DIR}/include/blahdio/audio_streamer.h
		${CMAKE_CURRENT_SOURCE_DIR}/include/blahdio/audio_writer.h
		${CMAKE_CURRENT_SOURCE_DIR}/include/blahdio/audio_type.h
		${CMAKE_CURRENT_SOURCE_DIR}/include/blahdio/expected.h
		${CMAKE_CURRENT_SOURCE_DIR}/include/blahdio/library_info.h
)

target_sources(blahdio PRIVATE
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

target_include_directories(blahdio PRIVATE include/blahdio src)

target_link_libraries(blahdio PUBLIC tl::expected)
target_link_libraries(blahdio PRIVATE $<BUILD_INTERFACE:utf8::cpp>)

if (BLAHDIO_ENABLE_WAVPACK)
	find_package(wavpack REQUIRED CONFIG)
	target_link_libraries(blahdio PUBLIC WavPack::WavPack)
	target_sources(blahdio PRIVATE
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

if (BLAHDIO_ENABLE_FLAC)
	target_sources(blahdio PRIVATE
		src/read/flac/flac_reader.h
		src/read/flac/flac_reader.cpp
	)
endif()

if (BLAHDIO_ENABLE_MP3)
	target_sources(blahdio PRIVATE
		src/read/mp3/mp3_reader.h
		src/read/mp3/mp3_reader.cpp
	)
endif()

if (BLAHDIO_ENABLE_WAV)
	target_sources(blahdio PRIVATE
		src/read/wav/wav_reader.h
		src/read/wav/wav_reader.cpp
		src/write/wav/wav_writer.h
		src/write/wav/wav_writer.cpp
	)
endif()

if (BLAHDIO_ENABLE_FLAC OR BLAHDIO_ENABLE_MP3 OR BLAHDIO_ENABLE_WAV)
	find_package(dr_libs REQUIRED CONFIG)
	find_package(miniaudio REQUIRED CONFIG)
	target_link_libraries(blahdio PRIVATE
		$<BUILD_INTERFACE:dr_libs::dr_libs>
		$<BUILD_INTERFACE:miniaudio::miniaudio>
	)
	target_sources(blahdio PRIVATE
		src/mackron/blahdio_dr_libs.h
		src/mackron/blahdio_dr_libs.cpp
		src/mackron/blahdio_miniaudio.cpp
	)
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

if (BLAHDIO_BUILD_TESTS)
	include(CTest)
	enable_testing()
	add_subdirectory(tests)
	add_test(NAME tests
		COMMAND blahdio_tests
	)
endif()

include(CMakePackageConfigHelpers)
install(TARGETS blahdio EXPORT blahdioTargets FILE_SET HEADERS)
install(EXPORT blahdioTargets FILE blahdioTargets.cmake NAMESPACE blahdio:: DESTINATION lib/cmake/blahdio)
configure_package_config_file(
    "${CMAKE_CURRENT_LIST_DIR}/cmake/blahdioConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/blahdioConfig.cmake"
    INSTALL_DESTINATION lib/cmake/blahdio
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/blahdioConfig.cmake" DESTINATION lib/cmake/blahdio)