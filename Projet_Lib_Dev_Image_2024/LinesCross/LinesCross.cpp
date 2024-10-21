#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <vector>

#include "LinesCross.hpp"

void drawLines(std::vector<cv::Vec2f> lines, cv::Mat imgResult){
    for(size_t i = 0; i < lines.size(); i++){
        float rho = lines[i][0]; float theta = lines[i][1];
        cv::Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;
        pt1.x = cvRound(x0 + 1000*(-b));
        pt1.y = cvRound(y0 + 1000*(a));
        pt2.x = cvRound(x0 - 1000*(-b));
        pt2.y = cvRound(y0 - 1000*(a));
        line(imgResult, pt1, pt2, cv::Scalar(0,0,255), 3, cv::LINE_AA);
    }
}

void drawPoint(cv::Point point, cv::Mat imgResult){
    circle(imgResult, point, 3, cv::Scalar(255, 0, 0), 3);
}

cv::Point crossPoint(float ro1, float th1, float ro2, float th2){
    cv::Point interPoint;

    // Cosinus et sinus pour les deux angles theta 1 et theta 2
    float ct1=cosf(th1);
    float st1=sinf(th1);
    float ct2=cosf(th2);
    float st2=sinf(th2);

    // Determinant
    float d=ct1*st2-st1*ct2;

    if(d!=0.0f) {
        // Determinant != 0 lignes non parallèles
        interPoint.x=(int)((st2*ro1-st1*ro2)/d);
        interPoint.y=(int)((-ct2*ro1+ct1*ro2)/d);
    } else {
        // Lignes parallèles pas d'intersection possible
        interPoint.x =-1;
        interPoint.y = -1;
        std::cout << "Parallel lines, no intersection possible - returned coordinates (-1,-1)" << std::endl;
    }

    return interPoint;
}

/* 
    Prend en paramètre une image binaire et renvoie les lignes des lasers
*/
std::vector<cv::Vec2f> laserLines(cv::Mat inputImage){
    std::vector<cv::Vec2f> result;

    cv::HoughLines(inputImage, result, 1, CV_PI/180, 200, 0, 0);
    
    return result;
}

/* 
    Separe les lignes en deux groupes distincts selon l'angle theta
    Renvoi le point d'intersection moyen de toutes les lignes des deux groupes
*/ 
cv::Point meanCrossPoint(std::vector<cv::Vec2f> lines){
    cv::Point result;

    // Separation des resultats de houglines en deux groupes de lignes distincts
    std::vector<cv::Vec2f> lines_1;
    std::vector<cv::Vec2f> lines_2;

    // Premier élement de comparaison
    lines_1.push_back(lines[0]);

    for(int i = 1; i < lines.size(); i++){
        if((lines[i][1] <= (lines_1[0][1]+0.3)) && (lines[i][1] >= (lines_1[0][1]-0.3))){
            lines_1.push_back(lines[i]);
        }
        else{
            lines_2.push_back(lines[i]);
        }
    }


    // Calcul des coordonnées des points d'intersection entre les deux groupes
    std::vector<cv::Point> pointsVector;

    for(int i = 0; i < lines_1.size(); i++){
        float ro1 = lines_1[i][0];
        float th1 = lines_1[i][1];
        
        for(int j = 0; i < lines_2.size(); i++){
            cv::Point interPoint;

            float ro2 = lines_2[j][0];
            float th2 = lines_2[j][1];

            interPoint = crossPoint(ro1, th1, ro2, th2);

            pointsVector.push_back(interPoint);
        }
    }

    // Cacul du point d'intersection moyen
    int meanX = 0, meanY = 0;
    
    for(int i = 0; i < pointsVector.size() ; i++){
        meanX += pointsVector[i].x;
        meanY += pointsVector[i].y;
    }

    meanX = meanX / (int)pointsVector.size();
    meanY = meanY / (int)pointsVector.size();
    
    result.x = meanX;
    result.y = meanY;
    
    return result;
}