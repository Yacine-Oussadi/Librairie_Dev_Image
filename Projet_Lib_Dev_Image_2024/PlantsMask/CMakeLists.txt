find_package( OpenCV REQUIRED)

include_directories( ${OpenCV_INCLUDE_DIRS} )

# build rule for PlantsMask library
add_library(PlantsMask PlantsMask.hpp PlantsMask.cpp)

# dependencies
target_link_libraries(PlantsMask ${OpenCV_LIBS} )