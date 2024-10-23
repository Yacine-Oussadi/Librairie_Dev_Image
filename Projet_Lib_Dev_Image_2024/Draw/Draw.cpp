#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <vector>

#include "../Draw/Draw.hpp"

void drawCentroids(cv::Mat image, std::vector<cv::Point> centroids){
    for (int i = 0; i < centroids.size(); i++) {
        circle(image, centroids[i], 2, cv::Scalar(255, 0, 0), 3);
    }
}

void drawRectangles(cv::Mat image, std::vector<cv::Rect> rectangles){
    for(int i = 0; i < rectangles.size(); i++){
        // Petit rectangle = Adventice ou bruit
        if (rectangles[i].height * rectangles[i].width <= (100*100)){
            cv::rectangle(image, rectangles[i], cv::Scalar(0,0,255));
        }
        else{
            cv::rectangle(image, rectangles[i], cv::Scalar(0,255,0));
        }
    }
}

void drawLines(cv::Mat image, std::vector<cv::Vec2f> lines){
    for(size_t i = 0; i < lines.size(); i++){
        float rho = lines[i][0]; float theta = lines[i][1];
        cv::Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;
        pt1.x = cvRound(x0 + 1000*(-b));
        pt1.y = cvRound(y0 + 1000*(a));
        pt2.x = cvRound(x0 - 1000*(-b));
        pt2.y = cvRound(y0 - 1000*(a));
        line(image, pt1, pt2, cv::Scalar(0,0,255), 2, cv::LINE_AA);
    }
}

void drawCrosspoint(cv::Mat image, cv::Point crosspoint){
    cv::circle(image, crosspoint, 3, cv::Scalar(255, 255, 255), 3);
}