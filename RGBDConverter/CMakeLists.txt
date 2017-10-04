cmake_minimum_required(VERSION 2.8)

# for C++ 11
set(CMAKE_CXX_STANDARD 11)

project( RGBDConverter )
find_package( OpenCV )
find_package(Boost REQUIRED COMPONENTS system filesystem program_options)

# find_package( Zlib) always fails, so we give absolute path here.
set(ZLIB_DIRS /usr/include)
set(ZLIB_LIBS /usr/local/lib/libz.so.1)

include_directories( ${OpenCV_INCLUDE_DIRS} )
include_directories( ${BOOST_INCLUDE_DIR} )
include_directories( ${ZLIB_DIRS} )

file(GLOB SRC_LIST
    "*.h"
    "*.cpp"
)

add_executable( RGBDConverter ${SRC_LIST})

target_link_libraries( 
	RGBDConverter 
	${OpenCV_LIBS}
	${ZLIB_LIBS}
	${Boost_LIBRARIES})