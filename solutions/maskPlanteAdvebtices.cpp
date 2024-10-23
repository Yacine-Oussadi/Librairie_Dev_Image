#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

// Fonction qui regroupe les rectangles proches
vector<Rect> regrouperContours(const vector<Rect>& rectangles, double seuilDistance) {
    vector<Rect> groupesRectangles;

    for (size_t i = 0; i < rectangles.size(); ++i) {
        Rect r = rectangles[i];
        bool ajouté = false;

        // Vérifier si le rectangle se trouve à une distance inférieure au seuil de n'importe quel autre rectangle
        for (size_t j = 0; j < groupesRectangles.size(); ++j) {
            double distance = norm(Point(r.x, r.y) - Point(groupesRectangles[j].x, groupesRectangles[j].y));
            if (distance < seuilDistance) {
                // Unir les rectangles proches
                groupesRectangles[j] = groupesRectangles[j] | r;
                ajouté = true;
                break;
            }
        }

        if (!ajouté) {
            // Si aucun rectangle proche n'est trouvé, on ajoute un nouveau rectangle
            groupesRectangles.push_back(r);
        }
    }

    return groupesRectangles;
}

int main(int argc, char** argv) {
    // Charger l'image
    Mat img = imread("../im001.png");
    if (img.empty()) {
        cerr << "Erreur lors de l'ouverture de l'image!" << endl;
        return -1;
    }

    // Conversion en espace de couleur HSV
    Mat hsv;
    cvtColor(img, hsv, COLOR_BGR2HSV);

    // Séparation des canaux Hue et Value
    vector<Mat> hsv_planes;
    split(hsv, hsv_planes);
    Mat hue = hsv_planes[0];
    Mat value = hsv_planes[2];

    // Appliquer un filtre médian pour réduire le bruit
    medianBlur(hue, hue, 5);
    medianBlur(value, value, 5);

    // Seuillage sur les canaux Hue et Value
    Mat maskHue, maskValue;
    inRange(hue, Scalar(60), Scalar(200), maskHue);
    inRange(value, Scalar(100), Scalar(255), maskValue);

    // Combinaison des deux masques (Hue + Value)
    Mat mask;
    bitwise_and(maskHue, maskValue, mask);

    // Afficher le masque des plantes détectées
    imshow("Plantes détectées (vert)", mask);
    waitKey(0);

    // Convertir l'image en niveaux de gris et détecter les bords avec Canny
    Mat imggray;
    cvtColor(img, imggray, COLOR_BGR2GRAY);
    Mat edges;
    Canny(imggray, edges, 150, 150, 3);
    std::vector<Vec2f> lines;
    HoughLines(edges, lines, 1, CV_PI / 180, 250, 0, 0);

    // Appliquer des opérations d'ouverture et de fermeture pour nettoyer le masque
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(9, 9));
    morphologyEx(mask, mask, MORPH_OPEN, kernel);
    morphologyEx(mask, mask, MORPH_CLOSE, kernel);

    // Détection des contours des adventices
    vector<vector<Point>> contours;
    findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    
    // Image résultante
    Mat imgResult = img.clone();
    
    // Vecteur pour stocker les rectangles des petites régions rouges
    vector<Rect> groupesRectangles; 

    // Boucle sur les contours détectés
    for (size_t i = 0; i < contours.size(); i++) {
        double area = contourArea(contours[i]);

        if (area < 300) {
            // Dessiner les petites régions en rouge
            drawContours(imgResult, contours, (int)i, Scalar(0, 0, 255), -1);
            groupesRectangles.push_back(boundingRect(contours[i])); // Ajouter le rectangle pour le contour rouge
        } 
        if(area>350){
            // Dessiner les grandes régions en vert
            drawContours(imgResult, contours, (int)i, Scalar(0, 255, 0), -1);
        }
    }

    // Regrouper les rectangles des régions rouges
    double seuilDistance = 100.0; // Distance seuil pour regrouper les contours
    groupesRectangles = regrouperContours(groupesRectangles, seuilDistance);

    // Dessiner les rectangles autour des groupes de contours rouges
    for (const Rect& rect : groupesRectangles) {
        rectangle(imgResult, rect, Scalar(0, 0, 255), 2); // Rectangle bleu autour des groupes
    }

    // Afficher l'image avec les contours des adventices
    imshow("Adventices détectées (rouge)", imgResult);
    waitKey(0);
    imshow("aprés ouverture et fermeture", kernel);
    waitKey(0);

    return 0;
}

