#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


//std::vector<cv::Rect> groupRectangles(std::vector<cv::Rect> rectangles);
void extraireVoisinsConnexes(cv::Mat &mask, cv::Mat &output, cv::Point seedPoint, int connectivity);
cv::Mat plantsMask(cv::Mat img);
std::vector<cv::Rect> extractRectangles(cv::Mat mask);
std::vector<cv::Point> extractCentroids(std::vector<cv::Rect> rectangles);