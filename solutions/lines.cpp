#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <vector>

using namespace cv;

// Fonction qui detecte le point d'intersection entre deux lignes
Point intersection(float ro1, float theta1, float ro2, float theta2){
    Point interPoint;
    
    std::cout << "Theta 1 = " << theta1 << std::endl;
    std::cout << "Theta 2 = " << theta2 << std::endl; 

    float ct1=cosf(theta1);     //a
    float st1=sinf(theta1);     //b
    float ct2=cosf(theta2);     //c
    float st2=sinf(theta2);     //d

    float d=ct1*st2-st1*ct2;    //determinative

    if(d!=0.0f) {   
        interPoint.x=(int)((st2*ro1-st1*ro2)/d);
        interPoint.y=(int)((-ct2*ro1+ct1*ro2)/d);
    } else { //lines are parallel, no intersection
        interPoint.x =-1;
        interPoint.y = -1;
        std::cout << "Parallel lines, no intersection possible - returned coordinates (-1,-1)" << std::endl;
    }

    return interPoint;
}

void drawLines(std::vector<Vec2f> lines, cv::Mat imgResult){
    for(size_t i = 0; i < lines.size(); i++){
        float rho = lines[i][0]; float theta = lines[i][1];
        Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;
        pt1.x = cvRound(x0 + 1000*(-b));
        pt1.y = cvRound(y0 + 1000*(a));
        pt2.x = cvRound(x0 - 1000*(-b));
        pt2.y = cvRound(y0 - 1000*(a));
        line(imgResult, pt1, pt2, Scalar(0,0,255), 3, LINE_AA);
    }
}


// Fonction de suppression des lignes avec le meme theta
std::vector<Vec2f> suppressionDoublons(const std::vector<Vec2f> lines){
    std::vector<Vec2f> result = lines;
    int size = lines.size();

    for(int i=0; i < size; i++){
        for (int j=i+1; j < size; j++){
            if(result[i][1] == result[j][1]){
                result.erase(result.begin() + j);
                size = size -1;
            }
        }   
    }

    return result;
}

// Recupération des coordonnées des droites à l'aide de hough lines
int main(int argc, char** argv){
    // Lecture d'une image depuis la dossier data
    std::string filename = "..\\..\\data\\im001.png";
    Mat img = imread(filename);
    if (img.empty()) {
        std::cout << "Error: no image to read " << std::endl;
        return -1;
    }

    // Obtention de l'image en nuances de gris
    Mat imggray;
    cvtColor(img, imggray, COLOR_BGR2GRAY);    

    // detection des bordures pour binariser l'image
    Mat contours;
    Canny(imggray, contours, 50, 200, 3);

    // recuperation des contours dans l'image d'affichage
    Mat imgCont, imgContP;
    cvtColor(contours, imgCont, COLOR_GRAY2BGR);
    imgContP = imgCont.clone();

    // Houghlines
    std::vector<Vec2f> lines;
    HoughLines(contours, lines, 1, CV_PI/180, 250, 0, 0);
    
    // Affichage du contenu après houghlines
    std::cout <<"Number of lines obtained from HoughLines() method = " << lines.size() << std::endl;
    for(int i = 0; i < lines.size(); i++){
        std::cout << "Line 1 : " << lines [i] << std::endl;
    }

    
    // Affichage du contenu après suppression des doublons
    lines = suppressionDoublons(lines);

    for(int i = 0; i < lines.size(); i++){
        std::cout << "Line 1 : " << lines [i] << std::endl;
    }

    // Dessin des lignes
    Mat imgResult = img.clone();
    drawLines(lines, imgResult);

    // Detection du point d'intersection
    Point intersectionPoint = intersection(lines[0][0], lines[0][1], lines[1][0], lines[1][1]);
    std::cout << "Coordonnée du point d'intersection = (" << intersectionPoint.x << ", " << intersectionPoint.y << ")" << std::endl;
    
    
    imshow("Image originale", img);
    imshow("Contours detectés", imgResult);
    waitKey(0);
    return 0;
}
