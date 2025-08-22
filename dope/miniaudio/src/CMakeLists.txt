cmake_minimum_required(VERSION 3.30)
project(miniaudio C)
add_library(miniaudio INTERFACE)
add_library(miniaudio::miniaudio ALIAS miniaudio)
target_sources(miniaudio PUBLIC
	FILE_SET HEADERS
	FILES
		miniaudio.h
)
install(
	TARGETS miniaudio
	EXPORT miniaudio
	FILE_SET HEADERS
)
install(EXPORT miniaudio
    FILE miniaudioConfig.cmake
    NAMESPACE miniaudio::
    DESTINATION lib/cmake/miniaudio
)