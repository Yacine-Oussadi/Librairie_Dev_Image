find_package( OpenCV REQUIRED)

include_directories( ${OpenCV_INCLUDE_DIRS} )

# build rule for PlantsMask library
add_library(Draw Draw.hpp Draw.cpp)

# dependencies
target_link_libraries(Draw ${OpenCV_LIBS} )