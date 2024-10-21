#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <vector>

#include "../LinesCross/LinesCross.hpp"
#include "../LaserMask/LaserMask.hpp"
#include "../PlantsMask/PlantsMask.hpp"

int main(int argc, char** argv){

    // path to the data directory containing the images
    std::string directoryPath = argv[1];
    char lastLetter = directoryPath.back();
    
    if(lastLetter != '/'){
        directoryPath = directoryPath + '/';
    }
    
    std::cout << "Data directory path p: " << directoryPath << std::endl;
    for(int image_index = 1; image_index <= 8; image_index++){
        cv::Mat img = cv::imread(directoryPath+"im00"+std::to_string(image_index)+".png");
        if (img.empty()) {
            std::cout << "Error: no image to read " << std::endl;
            return -1;
        }
        cv::Mat imgCpy = img.clone();

        // Recuperation du masque des lasers
        cv::Mat binaryLaserMask = laserMask(img);

        // Detection des intersection des lignes
        std::vector<cv::Vec2f> lines = laserLines(binaryLaserMask);
        for(int i = 0; i < lines.size() ; i++){
            std::cout << "Line " << i << " : " << lines[i] << std::endl;
        }

        // Recupération du point d'intersection moyen
        cv::Point crossPoint = meanCrossPoint(lines);

        std::cout << "Point d'intersection P = " << crossPoint << std::endl;

        drawPoint(crossPoint, img);

        cv::imshow("lines and crosspoint on the image "+std::to_string(image_index), img);
        cv::waitKey(0);

        // Recupération du masque des plantes
        cv::Mat mask = plantsMask(imgCpy);
        cv::imshow("Masque des plantes", mask);
        cv::waitKey(0);
    }

    return 0;
}