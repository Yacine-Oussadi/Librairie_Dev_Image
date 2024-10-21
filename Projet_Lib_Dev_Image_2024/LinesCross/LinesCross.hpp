#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <vector>

void drawLines(std::vector<cv::Vec2f> lines, cv::Mat imgResult);
void drawPoint(cv::Point point, cv::Mat imgResult);

cv::Point crossPoint(float ro1, float th1, float ro2, float th2);
std::vector<cv::Vec2f> laserLines(cv::Mat inputImage);
cv::Point meanCrossPoint(std::vector<cv::Vec2f> lines);