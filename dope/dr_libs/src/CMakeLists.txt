cmake_minimum_required(VERSION 3.30)
project(dr_libs C)
add_library(dr_libs INTERFACE)
add_library(dr_libs::dr_libs ALIAS dr_libs)
target_sources(dr_libs PUBLIC
	FILE_SET HEADERS
	FILES
		dr_flac.h
		dr_mp3.h
		dr_wav.h
)
install(
	TARGETS dr_libs
	EXPORT dr_libs
	FILE_SET HEADERS
)
install(EXPORT dr_libs
    FILE dr_libsConfig.cmake
    NAMESPACE dr_libs::
    DESTINATION lib/cmake/dr_libs
)