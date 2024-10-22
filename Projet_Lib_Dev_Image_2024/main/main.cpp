#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <vector>
#include <fstream>

#include "../LinesCross/LinesCross.hpp"
#include "../LaserMask/LaserMask.hpp"
#include "../PlantsMask/PlantsMask.hpp"
#include "../Draw/Draw.hpp"

int main(int argc, char** argv){

    if(argc < 3){
        std::cout << "Arguments insuffisants" << std::endl;
        return -1;
    }

    // path to the data directory containing the images
    std::string dataDirectoryPath = argv[1];

    char lastLetter = dataDirectoryPath.back();

    if(lastLetter != '/'){
        dataDirectoryPath = dataDirectoryPath + '/';
    }

    std::cout << "Data directory path p: " << dataDirectoryPath << std::endl;

    // Path to output Directory containing the results
    std::string outputDirectoryPath = argv[2];

    lastLetter = outputDirectoryPath.back();

    if(lastLetter != '/'){
        outputDirectoryPath = outputDirectoryPath + '/';
    }
    
    std::cout << "Data directory path p: " << dataDirectoryPath << std::endl;

    std::string header = "image_path, intersection, centroids_P, centroids_A\n";
    std::fstream myfile;

    myfile.open(outputDirectoryPath+"/results.csv", std::ios::out);
    if (!myfile) {
        std::cout << "File not created!" << std::endl;
    }

    //first line of the csv
    myfile << header;

    // Processing
    for(int image_index = 1; image_index <= 8; image_index++){
        cv::Mat img = cv::imread(dataDirectoryPath+"im00"+std::to_string(image_index)+".png");
        if (img.empty()) {
            std::cout << "Error: no image to read " << std::endl;
            return -1;
        }

        cv::Mat imgCpy = img.clone();

        // Recupération du masque des plantes
        cv::Mat mask = plantsMask(imgCpy);

        // Recuperation du masque des lasers
        cv::Mat binaryLaserMask = laserMask(img);

        // Detection des lignes
        std::vector<cv::Vec2f> lines = laserLines(binaryLaserMask);
        
        // Recupération du point d'intersection moyen
        cv::Point crossPoint = meanCrossPoint(lines);

        // Recuperation des recctangles entourant les plantes et adventices ainsi que les centroides
        std::vector<cv::Rect> boundingBoxes = extractRectangles(mask);
        std::vector<cv::Point> centroids = extractCentroids(boundingBoxes);

        // Dessiner les centroides, lignes, rectangles et le point d'intersection
        drawLines(imgCpy, lines);
        drawCrosspoint(imgCpy, crossPoint);
        drawCentroids(imgCpy, centroids);
        drawRectangles(imgCpy, boundingBoxes);

        // Sauvegarde des données
        std::string filename = "im00"+std::to_string(image_index)+".png";
        std::string strCrosspoint = "(" + std::to_string(crossPoint.x) + " _ " + std::to_string(crossPoint.y) + ")";
        std::string centroids_P;
        std::string centroids_A;

        for(int i = 0; i < boundingBoxes.size(); i++){

            std::string x = std::to_string(boundingBoxes[i].x + (boundingBoxes[i].width/2));
            std::string y = std::to_string(boundingBoxes[i].y + (boundingBoxes[i].height/2));

            if (boundingBoxes[i].height * boundingBoxes[i].width <= (100*100)){
                centroids_A.append("("+ x +" _ "+y+") ");
            }
            else{
                centroids_P.append("("+ x +" _ "+y+") ");
            }
        }

        std::string dataLine = filename+", "+strCrosspoint+", "+ centroids_P + ", "+ centroids_A+"\n";
        myfile << dataLine;

        bool check = cv::imwrite(outputDirectoryPath+"/im00"+std::to_string(image_index)+"_M.png", mask);

        if (check == false) {
            std::cout << "Could not save the resulting masks - Check the output directory path (argv[2])" << std::endl;

            // wait for any key to be pressed
            std::cin.get();

            return -1;
        }

        check = cv::imwrite(outputDirectoryPath+"/im00"+std::to_string(image_index)+"_R.png", imgCpy);

        if (check == false) {
            std::cout << "Could not save the resulting image - Check the output directory path (argv[2])" << std::endl;

            // wait for any key to be pressed
            std::cin.get();

            return -1;
        }
        cv::waitKey(0);
    }

    myfile.close();
    return 0;
}