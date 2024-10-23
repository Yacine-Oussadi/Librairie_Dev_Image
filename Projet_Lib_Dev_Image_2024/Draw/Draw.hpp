#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <vector>


void drawCentroids(cv::Mat image, std::vector<cv::Point> centroids);

void drawRectangles(cv::Mat image, std::vector<cv::Rect> rectangles);

void drawLines(cv::Mat image, std::vector<cv::Vec2f> lines);

void drawCrosspoint(cv::Mat image, cv::Point crosspoint);