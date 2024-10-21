#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <vector>

#include "LaserMask.hpp"

// Recupération des coordonnées des droites à l'aide de hough lines
cv::Mat laserMask(cv::Mat imageInput){

    // Conversion de l'image en HSV
    cv::Mat imgHSV;
    cv::cvtColor(imageInput, imgHSV, cv::COLOR_BGR2HSV);
    
    // Recuperation de trois cannaux de couleurs HSV
    std::vector<cv::Mat> colorChannels;
    cv::split(imgHSV, colorChannels);

    cv::Mat imgH = colorChannels[0];
    cv::Mat imgS = colorChannels[1];
    cv::Mat imgV = colorChannels[2];

    // Filtrage avec un filtre median sur le cannal S
    cv::Mat imgMedian;
    cv::Mat imgBinaire;

    cv::threshold(imgS, imgBinaire, 120, 130, 0);
    cv::medianBlur(imgBinaire, imgMedian, 3);

    // Ouverture et suppression du bruit engendré
    cv::Mat kernel = cv::Mat::ones(3, 3, CV_8U);
    cv::Mat imbOpening;

    cv::morphologyEx(imgMedian, imbOpening, cv::MORPH_CLOSE, kernel);

    // Display results
    /*
    cv::namedWindow("Binary image", cv::WINDOW_NORMAL); cv::imshow("Binary image", imgBinaire);
    cv::namedWindow("Hue MlinesMaskedian Filter", cv::WINDOW_NORMAL); cv::imshow("Hue Median Filter", imgMedian);
    cv::namedWindow("Opening", cv::WINDOW_NORMAL); cv::imshow("Opening", imbOpening);
    
    cv::waitKey(0);
    */
    return imbOpening;
}