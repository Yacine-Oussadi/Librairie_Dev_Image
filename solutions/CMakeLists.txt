Cmake_minimum_required(VERSION 2.8)
project( projet2024Techmed )
find_package( OpenCV REQUIRED)
include_directories( ${OpenCV_INCLUDE_DIRS} )
add_executable( lines lines.cpp)
target_link_libraries( lines ${OpenCV_LIBS} )