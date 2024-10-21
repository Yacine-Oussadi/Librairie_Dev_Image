find_package( OpenCV REQUIRED)

include_directories( ${OpenCV_INCLUDE_DIRS} )

# build rule for PlantsMask library
add_library(LinesCross LinesCross.hpp LinesCross.cpp)

# dependencies
target_link_libraries(LinesCross ${OpenCV_LIBS} )