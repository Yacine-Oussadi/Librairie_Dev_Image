find_package( OpenCV REQUIRED)

include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable(ProjetDevImages2024 main.cpp)

target_link_libraries(ProjetDevImages2024 LaserMask LinesCross PlantsMask ${OpenCV_LIBS})