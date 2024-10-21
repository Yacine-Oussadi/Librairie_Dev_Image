#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <vector>
#include "PlantsMask.hpp"

/*TODO:
std::vector<cv::Rect> groupRectangles(std::vector<cv::Rect> rectangles){
    std::vector<cv::Rect> resultVector;

    for(int i = 0; i < rectangles.size(); i++){
        for(int j = i+1; j < rectangles.size(); j++){
            
        }
    }
}
*/

// Fonction pour extraire les régions connexes
void extraireVoisinsConnexes(cv::Mat &mask, cv::Mat &output, cv::Point seedPoint, int connectivity = 8) {
    // Segmenter en utilisant un algorithme de floodFill (remplissage de région)
    cv::Scalar newVal = cv::Scalar(255);  // Remplir en blanc
    cv::Scalar loDiff = cv::Scalar(10), upDiff = cv::Scalar(10);
    cv::Rect ccomp;
    floodFill(mask, seedPoint, newVal, &ccomp, loDiff, upDiff, connectivity | cv::FLOODFILL_MASK_ONLY);
    
    // Copier le résultat dans l'image de sortie
    output = mask.clone();
}

// Prend en parametres une image et renvoie un masque binaire des plantes 
cv::Mat plantsMask(cv::Mat img) {
    // Convertir en espace de couleur HSV
    cv::Mat hsv;
    cv::cvtColor(img, hsv, cv::COLOR_BGR2HSV);

    // Séparation des canaux Hue, Saturation, Value
    std::vector<cv::Mat> hsv_planes;
    cv::split(hsv, hsv_planes);

    cv::Mat hue = hsv_planes[0];
    cv::Mat value = hsv_planes[2];

    // Appliquer un filtre médian pour réduire le bruit tout en préservant les bords
    cv::medianBlur(hue, hue, 5);  // Filtrage médian sur le canal Hue
    cv::medianBlur(value, value, 5);  // Filtrage médian sur le canal Value

    // Seuillage sur les canaux Hue et Value pour isoler les plantes
    cv::Mat maskHue, maskValue;
    cv::inRange(hue, cv::Scalar(60), cv::Scalar(200), maskHue);  // Seuillage dans les teintes vertes (ajuster si nécessaire)
    cv::inRange(value, cv::Scalar(100), cv::Scalar(255), maskValue);  // Seuillage sur la luminosité

    // Combinaison des deux masques (teinte + luminosité)
    cv::Mat mask;
    cv::bitwise_and(maskHue, maskValue, mask);
    
    // Convertir l'image en niveaux de gris pour Canny (correction de l'erreur)
    cv::Mat imggray;
    cvtColor(img, imggray, cv::COLOR_BGR2GRAY);
    
    // Appliquer des opérations d'ouverture et de fermeture pour nettoyer l'image
    cv::Mat openingKernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(9, 9));
    cv::Mat closingKernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(11, 11));

    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, openingKernel);  // Ouverture pour éliminer les petits objets les lignes du laser
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, closingKernel); // Fermeture pour combler les trous

    // Afficher le masque résultant
    //cv::imshow("Masque après ouverture/fermeture", mask);

    // Détection des contours
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // Filtrer et dessiner les contours des plantes
    cv::Mat imgResult = img.clone();
    for (size_t i = 0; i < contours.size(); i++) {
        double area = cv::contourArea(contours[i]);

        // Ajuster les seuils pour éliminer les petites plantes ou les objets parasites
        if (area > 500 && area < 10000) {
            cv::drawContours(imgResult, contours, (int)i, cv::Scalar(0, 255, 0), 2);  // Contours en vert
        }
    }

    // Extraire les régions connexes autour d'un point de départ (exemple : centre de l'image)
    cv::Point seedPoint(mask.cols / 2, mask.rows / 2);
    cv::Mat regionsConnexes;

    extraireVoisinsConnexes(mask, regionsConnexes, seedPoint, 8);

    //Rpérage des centroides et des rectangles entourant les plantes connex
    cv::Mat labels;
    cv::Mat stats;
    cv::Mat centroids;

    cv::connectedComponentsWithStats(mask, labels, stats, centroids);
    std::vector<cv::Rect> rectangles;

    for(int i=0; i<stats.rows; i++)
    {
        int x = stats.at<int>(cv::Point(0, i));
        int y = stats.at<int>(cv::Point(1, i));
        int w = stats.at<int>(cv::Point(2, i));
        int h = stats.at<int>(cv::Point(3, i));
        cv::Rect rect(x,y,w,h);
        cv::rectangle(regionsConnexes, rect, cv::Scalar(255,0,0));
        rectangles.push_back(rect);
    }

    return regionsConnexes;
}
