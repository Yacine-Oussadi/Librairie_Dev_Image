#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

// Fonction pour dessiner les lignes détectées
void drawLines(std::vector<Vec2f> lines, Mat &imgResult){
    for(size_t i = 0; i < lines.size(); i++){
        float rho = lines[i][0]; 
        float theta = lines[i][1];
        Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a * rho, y0 = b * rho;
        pt1.x = cvRound(x0 + 1000 * (-b));
        pt1.y = cvRound(y0 + 1000 * (a));
        pt2.x = cvRound(x0 - 1000 * (-b));
        pt2.y = cvRound(y0 - 1000 * (a));
        line(imgResult, pt1, pt2, Scalar(0,0,255), 3, LINE_AA); // Lignes rouges
    }
}

// Fonction pour masquer les lignes détectées dans l'image
void masquerLignes(Mat &mask, std::vector<Vec2f> lines) {
    for(size_t i = 0; i < lines.size(); i++) {
        float rho = lines[i][0]; 
        float theta = lines[i][1];
        Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a * rho, y0 = b * rho;
        pt1.x = cvRound(x0 + 1000 * (-b));
        pt1.y = cvRound(y0 + 1000 * (a));
        pt2.x = cvRound(x0 - 1000 * (-b));
        pt2.y = cvRound(y0 - 1000 * (a));
        line(mask, pt1, pt2, Scalar(0), 15); // Masquer les lignes avec une épaisseur de 15 pixels
    }
}

// Fonction pour extraire les régions connexes
void extraireVoisinsConnexes(Mat &mask, Mat &output, Point seedPoint, int connectivity=8){
    // Segmenter en utilisant un algorithme de floodFill (remplissage de région)
    Scalar newVal = Scalar(255);  // Remplir en blanc
    Scalar loDiff = Scalar(10), upDiff = Scalar(10);
    Rect ccomp;
    floodFill(mask, seedPoint, newVal, &ccomp, loDiff, upDiff, connectivity | FLOODFILL_MASK_ONLY);
    
    // Copier le résultat dans l'image de sortie
    output = mask.clone();
}

int main(int argc, char** argv){
    // Charger l'image
    Mat img = imread("../im002.png");
    if (img.empty()) {
        cerr << "Erreur lors de l'ouverture de l'image!" << endl;
        return -1;
    }

    // Convertir en espace de couleur HSV
    Mat hsv;
    cvtColor(img, hsv, COLOR_BGR2HSV);

    // Séparation des canaux Hue, Saturation, Value
    vector<Mat> hsv_planes;
    split(hsv, hsv_planes);
    Mat hue = hsv_planes[0];
    Mat value = hsv_planes[2];

    // Appliquer un filtre médian pour réduire le bruit tout en préservant les bords
    medianBlur(hue, hue, 5);  // Filtrage médian sur le canal Hue
    medianBlur(value, value, 5);  // Filtrage médian sur le canal Value

    // Seuillage sur les canaux Hue et Value pour isoler les plantes
    Mat maskHue, maskValue;
    inRange(hue, Scalar(60), Scalar(200), maskHue);  // Seuillage dans les teintes vertes (ajuster si nécessaire)
    inRange(value, Scalar(100), Scalar(255), maskValue);  // Seuillage sur la luminosité

    // Combinaison des deux masques (teinte + luminosité)
    Mat mask;
    bitwise_and(maskHue, maskValue, mask);

    // Appliquer des opérations d'ouverture et de fermeture pour nettoyer l'image
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
    morphologyEx(mask, mask, MORPH_OPEN, kernel);  // Ouverture pour éliminer les petits objets
    morphologyEx(mask, mask, MORPH_CLOSE, kernel); // Fermeture pour combler les trous

    // Afficher le masque résultant
    imshow("Masque après ouverture/fermeture", mask);

    // Détection des contours
    vector<vector<Point>> contours;
    findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    // Filtrer et dessiner les contours des plantes
    Mat imgResult = img.clone();
    for (size_t i = 0; i < contours.size(); i++) {
        double area = contourArea(contours[i]);

        // Ajuster les seuils pour éliminer les petites plantes ou les objets parasites
        if (area > 500 && area < 10000) {
            drawContours(imgResult, contours, (int)i, Scalar(0, 255, 0), 2);  // Contours en vert
        }
    }
    // Convertir l'image en niveaux de gris pour Canny (correction de l'erreur)
    Mat imggray;
    cvtColor(img, imggray, COLOR_BGR2GRAY);

    // Masquer les lignes détectées
    Mat edges;
    Canny(imggray, edges, 150, 150, 3);
    std::vector<Vec2f> lines;
    HoughLines(edges, lines, 1, CV_PI/180, 250, 0, 0);
    masquerLignes(mask, lines);

    // Extraire les régions connexes autour d'un point de départ
    Point seedPoint(mask.cols / 2, mask.rows / 2);
    Mat regionsConnexes;
    extraireVoisinsConnexes(mask, regionsConnexes, seedPoint, 8);
    imshow("Régions connexes", regionsConnexes);

    // Afficher les résultats
    imshow("Lignes détectées et contours des plantes", imgResult);
    imshow("Masque après traitement", mask);

    waitKey(0);
    return 0;
}

