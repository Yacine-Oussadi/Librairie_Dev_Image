#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <cmath>

using namespace cv;
using namespace std;

void showHistogram(const Mat& img, const String& name)
{
    // Calculer l'histogramme
    int histSize = 256;
    float range[] = { 0, 256 };
    const float* histRange = { range };
    Mat hist;
    calcHist(&img, 1, 0, Mat(), hist, 1, &histSize, &histRange);

    // Normaliser l'histogramme
    normalize(hist, hist, 0, 512, NORM_MINMAX);

    // Créer une image pour l'histogramme
    int hist_w = 512, hist_h = 400;
    Mat histImage(hist_h, hist_w, CV_8UC3, Scalar(0,0,0));

    // Dessiner l'histogramme
    for(int i = 0; i < histSize; i++)
    {
        line(histImage, Point(i*2, hist_h), Point(i*2, hist_h - cvRound(hist.at<float>(i))),
             Scalar(255, 255, 255), 2, 8, 0);
    }

    // Afficher l'histogramme
    imshow(name, histImage);
}

Mat binarizeByHue(const Mat& src, int lowHue, int highHue)
{
    Mat hsv, mask;
    
    // Convertir l'image en HSV
    cvtColor(src, hsv, COLOR_BGR2HSV);
    
    // Définir la plage de teinte
    Scalar lowerBound(lowHue, 30, 30);
    Scalar upperBound(highHue, 255, 255);
    
    // Créer un masque binaire basé sur la plage de teinte
    inRange(hsv, lowerBound, upperBound, mask);
    
    return mask;
}

int main() {
    // Read an image file
    string filename = "../im002.png";
    Mat im = imread(filename);

    if (im.empty()) {
        cout << "Erreur lors du chargement de l'image" << endl;
        return -1;
    }

    // Convert to grayscale image
    Mat img;
    cvtColor(im, img, COLOR_BGR2GRAY);

    // Afficher l'histogramme de l'image originale
    showHistogram(img, "Histogramme original");

    // Appliquer l'égalisation d'histogramme
    Mat img_equalized;
    equalizeHist(img, img_equalized);

    // Afficher l'histogramme de l'image égalisée
    showHistogram(img_equalized, "Histogramme égalisé");

    // Binariser l'image basée sur la teinte
    Mat binarized = binarizeByHue(im, 90, 120);  // Exemple: binariser les teintes vertes
    

    

    // Afficher les résultats
    imshow("Image originale", im);
    imshow("Image en niveaux de gris", img);
    imshow("Image binarisée par teinte", binarized);
   
 
    waitKey(0);
    destroyAllWindows();

    return 0;
}



