#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

// Fonction qui detecte le point d'intersection entre deux lignes
Point intersection(float ro1, float theta1, float ro2, float theta2) {
    Point interPoint;
    float ct1 = cosf(theta1); // a
    float st1 = sinf(theta1); // b
    float ct2 = cosf(theta2); // c
    float st2 = sinf(theta2); // d

    float d = ct1 * st2 - st1 * ct2; // determinante

    if (d != 0.0f) {
        interPoint.x = (int)((st2 * ro1 - st1 * ro2) / d);
        interPoint.y = (int)((-ct2 * ro1 + ct1 * ro2) / d);
    } else {
        // Lignes parallèles, pas d'intersection
        interPoint.x = -1;
        interPoint.y = -1;
        std::cout << "Parallel lines, no intersection possible - returned coordinates (-1,-1)" << std::endl;
    }
    return interPoint;
}

// Fonction pour dessiner les lignes détectées
void drawLines(std::vector<Vec2f> lines, Mat &imgResult) {
    for (size_t i = 0; i < lines.size(); i++) {
        float rho = lines[i][0];
        float theta = lines[i][1];
        Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a * rho, y0 = b * rho;
        pt1.x = cvRound(x0 + 1000 * (-b));
        pt1.y = cvRound(y0 + 1000 * (a));
        pt2.x = cvRound(x0 - 1000 * (-b));
        pt2.y = cvRound(y0 - 1000 * (a));
        line(imgResult, pt1, pt2, Scalar(0, 0, 255), 3, LINE_AA); // Lignes en rouge
    }
}

// Fonction pour masquer les lignes détectées dans l'image
void masquerLignes(Mat &mask, std::vector<Vec2f> lines) {
    for (size_t i = 0; i < lines.size(); i++) {
        float rho = lines[i][0];
        float theta = lines[i][1];
        Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a * rho, y0 = b * rho;
        pt1.x = cvRound(x0 + 1000 * (-b));
        pt1.y = cvRound(y0 + 1000 * (a));
        pt2.x = cvRound(x0 - 1000 * (-b));
        pt2.y = cvRound(y0 - 1000 * (a));
         // Filtrer par longueur
	    double length = norm(pt1 - pt2);
	    if (length > 50) { // Ajustez ce seuil en fonction de vos besoins
		line(mask, pt1, pt2, Scalar(0), 1); // Masquer les lignes avec une épaisseur de 25 pixels
    }
        //line(mask, pt1, pt2, Scalar(0), 25); // Masquer les lignes avec une épaisseur de 15 pixels
    }
}

// Fonction pour extraire les régions connexes
void extraireVoisinsConnexes(Mat &mask, Mat &output, Point seedPoint, int connectivity = 8) {
    // Segmenter en utilisant un algorithme de floodFill (remplissage de région)
    Scalar newVal = Scalar(255);  // Remplir en blanc
    Scalar loDiff = Scalar(10), upDiff = Scalar(10);
    Rect ccomp;
    floodFill(mask, seedPoint, newVal, &ccomp, loDiff, upDiff, connectivity | FLOODFILL_MASK_ONLY);
    
    // Copier le résultat dans l'image de sortie
    output = mask.clone();
}

int main(int argc, char** argv) {
    // Charger l'image
    Mat img = imread("../im005.png");
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
    
    // Convertir l'image en niveaux de gris pour Canny (correction de l'erreur)
    Mat imggray;
    cvtColor(img, imggray, COLOR_BGR2GRAY);
    
     // Détection des bords avec Canny
    Mat edges;
    Canny(imggray, edges, 150, 150, 3);
    std::vector<Vec2f> lines;
    HoughLines(edges, lines, 1, CV_PI / 180, 250, 0, 0);
    
    // Masquer les lignes détectées
    masquerLignes(mask, lines);

    // Appliquer des opérations d'ouverture et de fermeture pour nettoyer l'image
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(7, 7));
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

    

   

    // Extraire les régions connexes autour d'un point de départ (exemple : centre de l'image)
    Point seedPoint(mask.cols / 2, mask.rows / 2);
    Mat regionsConnexes;
    extraireVoisinsConnexes(mask, regionsConnexes, seedPoint, 8);
    imshow("Régions connexes", regionsConnexes);

    // Détection et affichage du point d'intersection des deux premières lignes
    if (lines.size() >= 2) {
        Point intersectionPoint = intersection(lines[0][0], lines[0][1], lines[1][0], lines[1][1]);
        if (intersectionPoint.x != -1 && intersectionPoint.y != -1) {
            circle(imgResult, intersectionPoint, 5, Scalar(255, 0, 0), -1); // Marquer le point d'intersection en bleu
            std::cout << "Coordonnée du point d'intersection = (" << intersectionPoint.x << ", " << intersectionPoint.y << ")" << std::endl;
        }
    }

    // Afficher les résultats
    imshow("Lignes détectées et contours des plantes", imgResult);
    imshow("Masque après traitement", mask);

    waitKey(0);
    return 0;
}

