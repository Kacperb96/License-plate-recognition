#include "PlateIdentified.h"

using namespace std;
using namespace cv;

vector<PossiblePlate> detect_plates(Mat &img_original) {

    // to b�dzie warto�� zwracana
    vector<PossiblePlate> possible_plates_vector;

    // zmienne Mat, obraz szaro�ci i threshold
    Mat img_gray;
    Mat img_thresh;

    // CV_8UC3 jest macierz�/obrazem liczb ca�kowitych bez znaku
    Mat img_contours(img_original.size(), CV_8UC3, S_BLACK);

    RNG rng;

    // usuwa wszytkie stworzone okna
    destroyAllWindows();

    // wst�pne przetwarzanie �eby uzyska� obrazy w skali szaro�ci i threshold
    preprocess(img_original, img_gray, img_thresh);        

    // znajduje wszystkie mo�liwe znaki na obrazku
    // ta funkcja najpierw wyszukuje wszystkie kontury a nast�pnie uwzgl�dnia tylko kontury, kt�re mog� by� znakami jeszcze bez por�wnania z innymi znakami
    vector<PotentialChar> possible_chars_vector = possible_chars(img_thresh);

    // maj�c wektor wszystkich mo�liwych znak�w znajduje grupy pasuj�cych znak�w
    // w kolejnych krokach ka�da grupa pasuj�cych znak�w b�dzie pr�bowa�a zosta� rozpoznana jako tablica
    vector<vector<PotentialChar> > matching_chars_vector = find_correct_chars_vector(possible_chars_vector);	

    // p�tla for dla ka�dej grupy pasuj�cych znak�w
    for (auto &matching_chars_vector : matching_chars_vector) {   

        // wydobycie tablicy
        PossiblePlate possible_plate = extract_plate(img_original, matching_chars_vector);        

        // je�li tablica zosta�a odnaleziona
        if (possible_plate.plate.empty() == false) {                                         
            
            // dodaj do wektora mo�liwych tablic
            possible_plates_vector.push_back(possible_plate);                             
        }
    }

    std::cout << endl << possible_plates_vector.size() << "Mozliwa tablica odnaleziona "<< endl << endl;

    return possible_plates_vector;
}


vector<PotentialChar> possible_chars(cv:: Mat& img_thresh) {

    // to b�dzie zwracana warto��
    vector<PotentialChar> possible_chars_vector;       

    Mat img_contour(img_thresh.size(), CV_8UC3, S_BLACK);
    int possible_chars_counter = 0;

    // tworzenie kopii
    Mat img_thresh_copy = img_thresh.clone();

    vector<vector<Point> > contours;

    // szuka wszystkich kontur�w
    findContours(img_thresh_copy, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);       

    // p�tla for dla ka�dego konturu
    for (unsigned int i = 0; i < contours.size(); i++) {    

        PotentialChar possible_char(contours[i]);

        // je�li kontur jest mo�liwym znakiem nie jest to por�wnywane z innymi znakami jeszcze
        if (if_possible_char(possible_char)) {     

            // inkrementuje licznik mo�liwych znak�w
            possible_chars_counter++;    

            // dodaje do wektora mo�liwe znaki
            possible_chars_vector.push_back(possible_char);     
        }
    }

    return(possible_chars_vector);
}

PossiblePlate extract_plate(Mat &img_original, vector<PotentialChar> &matching_chars_vector) {
    
    // to b�dzie zwracana warto��
    PossiblePlate possiblePlate;           

    // sortuje znaki od lewej do prawej 
    sort(matching_chars_vector.begin(), matching_chars_vector.end(), PotentialChar::sorting_chars);

    // obliczanie punktu centralnego tablicy
    double plate_center_x = (double)(matching_chars_vector[0].x_center + matching_chars_vector[matching_chars_vector.size() - 1].x_center) / 2.0;
    double plate_center_y = (double)(matching_chars_vector[0].y_center + matching_chars_vector[matching_chars_vector.size() - 1].y_center) / 2.0;
    Point2d center_plate_point(plate_center_x, plate_center_y);

    // obliczanie wysoko�ci i szeroko�ci tablicy
    int plate_width = (int)((matching_chars_vector[matching_chars_vector.size() - 1].boundingRect.x + matching_chars_vector[matching_chars_vector.size() - 1].boundingRect.width - matching_chars_vector[0].boundingRect.x) * PADDING_FACTOR_WIDTH);

    double total_char_height = 0;

    for (auto &matching_char : matching_chars_vector) {
        total_char_height = total_char_height + matching_char.boundingRect.height;
    }

    double average_char_height = (double)total_char_height / matching_chars_vector.size();

    int plate_height = (int)(average_char_height * PADDING_FACTOR_HEIGHT);

    // obliczanie k�ta obszaru tablicy
    double opposite = matching_chars_vector[matching_chars_vector.size() - 1].y_center - matching_chars_vector[0].y_center;
    double hypotenuse = chars_distance(matching_chars_vector[0], matching_chars_vector[matching_chars_vector.size() - 1]);
    double angle_radian = asin(opposite / hypotenuse);
    double angle_degrees = angle_radian * (180.0 / CV_PI);

    // przypisz obr�con� zmienn� RotatedRect mo�liwej tablicy
    possiblePlate.plate_location = RotatedRect(center_plate_point, Size2f((float)plate_width, (float)plate_height), (float)angle_degrees);

    // ostatnie kroki to wykonanie rotacji
    Mat rotation_mat;         
    Mat img_rotade;
    Mat img_cropped;

    // uzyskanie macierzy rotacji dla naszego obliczonego k�ta 
    rotation_mat = getRotationMatrix2D(center_plate_point, angle_degrees, 1.0);         

    // obraca ca�y obraz
    warpAffine(img_original, img_rotade, rotation_mat, img_original.size());          

    // wycina cz�� tablicy
    getRectSubPix(img_rotade, possiblePlate.plate_location.size, possiblePlate.plate_location.center, img_cropped);

    // kopiuje przyci�ty obraz tablicy do odpowiedniej zmiennej sk�adowej mo�liwej tablicy
    possiblePlate.plate = img_cropped;            

    return(possiblePlate);
}

