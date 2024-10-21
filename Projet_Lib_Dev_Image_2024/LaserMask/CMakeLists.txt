find_package( OpenCV REQUIRED)

include_directories( ${OpenCV_INCLUDE_DIRS} )

# build rule for PlantsMask library
add_library(LaserMask LaserMask.hpp LaserMask.cpp)

# dependencies
target_link_libraries(LaserMask ${OpenCV_LIBS} )