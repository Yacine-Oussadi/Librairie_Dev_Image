#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <vector>
#include "PlantsMask.hpp"

// Fonction qui regroupe les rectangles qui s'intersectent en un seul rectangle
void mergeRectangles(std::vector<cv::Rect>& rectangles, bool recursiveMerge = false, std::function<bool(const cv::Rect& r1, const cv::Rect& r2)> mergeFn = nullptr) {
    static auto defaultFn = [](const cv::Rect& r1, const cv::Rect& r2) {
        return (r1.x < (r2.x + r2.width) && (r1.x + r1.width) > r2.x && r1.y < (r2.y + r2.height) && (r1.y + r1.height) > r2.y);
    };

    static auto innerMerger = [](std::vector<cv::Rect>& rectangles, std::function<bool(const cv::Rect& r1, const cv::Rect& r2)>& mergeFn) {
        std::vector<std::vector<std::vector<cv::Rect>::const_iterator>> groups;
        std::vector<cv::Rect> mergedRectangles;
        bool merged = false;

        static auto findIterator = [&](std::vector<cv::Rect>::const_iterator& iteratorToFind) {
            for (auto groupIterator = groups.begin(); groupIterator != groups.end(); ++groupIterator) {
                auto foundIterator = std::find(groupIterator->begin(), groupIterator->end(), iteratorToFind);
                if (foundIterator != groupIterator->end()) {
                    return groupIterator;
                }
            }
            return groups.end();
        };

        for (auto rect1_iterator = rectangles.begin(); rect1_iterator != rectangles.end(); ++rect1_iterator) {
            auto groupIterator = findIterator(rect1_iterator);

            if (groupIterator == groups.end()) {
                groups.push_back({rect1_iterator});
                groupIterator = groups.end() - 1;
            }

            for (auto rect2_iterator = rect1_iterator + 1; rect2_iterator != rectangles.end(); ++rect2_iterator) {
                if (mergeFn(*rect1_iterator, *rect2_iterator)) {
                    groupIterator->push_back(rect2_iterator);
                    merged = true;
                }
            }
        }

        for (auto groupIterator = groups.begin(); groupIterator != groups.end(); ++groupIterator) {
            auto groupElement = groupIterator->begin();

            int x1 = (*groupElement)->x;
            int x2 = (*groupElement)->x + (*groupElement)->width;
            int y1 = (*groupElement)->y;
            int y2 = (*groupElement)->y + (*groupElement)->height;

            while (++groupElement != groupIterator->end()) {
                if (x1 > (*groupElement)->x)
                    x1 = (*groupElement)->x;
                if (x2 < (*groupElement)->x + (*groupElement)->width)
                    x2 = (*groupElement)->x + (*groupElement)->width;
                if (y1 >(*groupElement)->y)
                    y1 = (*groupElement)->y;
                if (y2 < (*groupElement)->y + (*groupElement)->height)
                    y2 = (*groupElement)->y + (*groupElement)->height;
            }

            mergedRectangles.push_back(cv::Rect(cv::Point(x1, y1), cv::Point(x2, y2)));
        }

        rectangles = mergedRectangles;
        return merged;
    };

    if (!mergeFn)
        mergeFn = defaultFn;

    while (innerMerger(rectangles, mergeFn) && recursiveMerge);
}

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
    cv::Mat closingKernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(9, 9));

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

    return regionsConnexes;
}


std::vector<cv::Rect> extractRectangles(cv::Mat mask){
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
        rectangles.push_back(rect);
        //std::cout << "Rectangle " << i << " = " << rect << std::endl;
    }

    // Elimination du rectangle englobant l'image
    rectangles.erase(rectangles.begin());

    // regroupement des rectangles qui s'intersectent
    //std::cout << "Number of rectangles before merging = "  << rectangles.size() << std::endl;
    mergeRectangles(rectangles);
    //std::cout << "Number of rectangles after merging = "  << rectangles.size() << std::endl;

    //elimination des rectangles de très petites taille
    std::vector<cv::Rect> resultRectangles;
    
    for(int i = 0; i < rectangles.size(); i++){
        if(rectangles[i].height * rectangles[i].width >= (15*15)){
            resultRectangles.push_back(rectangles[i]);
        }
    }

    return resultRectangles;
}

std::vector<cv::Point> extractCentroids(std::vector<cv::Rect> rectangles){
    std::vector<cv::Point> centroids;

    for (int i = 0; i < rectangles.size(); i++)
    {   
        int x = rectangles[i].x + (rectangles[i].width/2);
        int y = rectangles[i].y + (rectangles[i].height/2);
        
        centroids.push_back(cv::Point(x,y));
    }
    
    return centroids;
}