#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <vector>


using namespace cv;

// 
Point intersection(float ro1, float theta1, float ro2, float theta2){
    Point interPoint;
    
    std::cout << "Theta 1 = " << theta1 << std::endl;
    std::cout << "Theta 2 = " << theta2 << std::endl; 

    float ct1=cosf(theta1);     //a
    float st1=sinf(theta1);     //b
    float ct2=cosf(theta2);     //c
    float st2=sinf(theta2);     //d

    float d=ct1*st2-st1*ct2;    //determinative (rearranged matrix for inverse)
    std::cout << ct1 << " " << ct2 << " " << st1 << " " << st2 << std::endl;
    std::cout << "d = " << d << std::endl;

    if(d!=0.0f) {   
        interPoint.x=(int)((st2*ro1-st1*ro2)/d);
        interPoint.y=(int)((-ct2*ro1+ct1*ro2)/d);
    } else { //lines are parallel and will NEVER intersect!
        interPoint.x =-1;
        interPoint.y = -1;
        std::cout << "Parallel lines, no intersection possible - returned coordinates (-1,-1)" << std::endl;
    }

    return interPoint;
}

// Recupération des coordonnées des droites à l'aide de hough lines
int main(int argc, char** argv){
    // Lecture d'une image depuis la dossier data
    std::string filename = "../../data/data/im001.png";
    Mat img = imread(filename);
    if (img.empty()) {
        std::cout << "Error: no image to read " << std::endl;
        return -1;
    }

    // Obtention de l'image en nuances de gris
    Mat imggray;
    cvtColor(img, imggray, COLOR_BGR2GRAY);    

    Mat contours;

    // detection des bordures
    Canny(imggray, contours, 50, 200, 3);

    // recuperation des contours dans l'image d'affichage
    Mat imgCont, imgContP;
    cvtColor(contours, imgCont, COLOR_GRAY2BGR);
    imgContP = imgCont.clone();

    // Houghlines
    std::vector<Vec2f> lines;
    HoughLines(contours, lines, 1, CV_PI/180, 250, 0, 0);
    
    std::cout <<"Number of lines obtained from HoughLines() method = " << lines.size() << std::endl;


    // Deleting multible occurences of lines with the same theta
    for(size_t i = 0; i < lines.size() ; i++){
        float theta = lines[i][1];
        for(size_t j = i+1; j < lines.size() ; j++){
            if(lines[j][i] == theta){
                lines.erase(lines.begin()+j);
            }
        }
    }

    std::cout <<"Number of lines obtained from HoughLines() method = " << lines.size() << std::endl;


    // Dessin des lignes
    for(size_t i = 0; i < lines.size(); i++){
        float rho = lines[i][0]; float theta = lines[i][1];
        Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;
        pt1.x = cvRound(x0 + 1000*(-b));
        pt1.y = cvRound(y0 + 1000*(a));
        pt2.x = cvRound(x0 - 1000*(-b));
        pt2.y = cvRound(y0 - 1000*(a));
        line(imgCont, pt1, pt2, Scalar(0,0,255), 3, LINE_AA);
    }

    //Point intersectionPoint = intersection(lines);
    //std::cout << "Coordonnée du point d'intersection = (" << intersectionPoint.x << ", " << intersectionPoint.y << ")" << std::endl;
    
    
    imshow("Image originale", img);
    imshow("Image gris", imggray);
    imshow("Contours detectés", imgCont);
    waitKey(0);
    return 0;
}
