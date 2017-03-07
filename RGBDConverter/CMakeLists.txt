cmake_minimum_required(VERSION 2.8)

project( RGBDConverter )
find_package( OpenCV )
find_package( Zlib )
find_package(Boost COMPONENTS system REQUIRED)
find_package(Boost COMPONENTS filesystem REQUIRED)

include_directories( ${OpenCV_INCLUDE_DIRS} )
include_directories( ${ZLIB_INCLUDE_DIR} )
include_directories( ${BOOST_INCLUDE_DIR} )

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