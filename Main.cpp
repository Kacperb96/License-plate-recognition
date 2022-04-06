#include "Main.h"

using namespace std;
using namespace cv;

int main() {

    // KNN trening - wczytanie
    bool KNN_training_successful = load_knn_and_train();         

    // je�li KNN nie b�dzie wszytany prawid�owo wy�wietl komunikat i zako�cz program
    if (KNN_training_successful == false) {                            
        cout << endl << endl << "KNN nie zostal wczytany" << endl << endl;
        return(0);                                                      
    }

    // obraz wej�ciowy
    Mat img_original;          

    // wczytaj obraz ze �cie�ki
    img_original = imread("mazda.jpg");        

    // je�li �cie�ka nie b�dzie prawid�owa
    if (img_original.empty()) {                             
        cout << "Obraz nie wczytany" << endl << endl;                                                
        return(0);                                           
    }

    // wykrywanie tablic
    vector<PossiblePlate> possible_plates_vector = detect_plates(img_original);

    // wykrywanie znak�w w tablicy
    possible_plates_vector = detect_chars_in_plates(possible_plates_vector);

    // poka� obraz
    imshow("img_original", img_original);    

    // je�li tablica nie zostanie wykryta
    if (possible_plates_vector.empty()) {                                               
        cout << endl << "Nie wykryto tablicy" << endl;     
    }

    else {       
        // je�li otrzymany tutaj wektor mo�liwych tablic ma co najmniej jedn� p�ytk�
        // posortuj wektor mo�liwych tablic w kolejno�ci malej�co (od najwi�kszej liczby znak�w do najmniejszej liczby znak�w)
        sort(possible_plates_vector.begin(), possible_plates_vector.end(), PossiblePlate::descending_sort);

        // za��my, �e tablica z najbardziej rozpoznawanymi znakami (pierwsza tablica posortowana wed�ug d�ugo�ci �a�cucha w kolejno�ci malej�cej) jest rzeczywist� tablic�
        PossiblePlate lic_plate = possible_plates_vector.front();

        // poka� tablice (img_thresh i zwyk�a tablica)
        imshow("img_plate", lic_plate.plate);

        // je�li nie wykryto �adnych znak�w 
        if (lic_plate.chars.length() == 0) {
            cout << endl << "Nie wykryto znakow" << endl << endl;     
            return(0);                                                                             
        }

        // pokazuje prostok�t wok� tablicy
        rect_around_plate(img_original, lic_plate);
        
        // wypisz tablic� rejestracyjn� 
        cout << "\n\n\n\n\n\n\n\n\n";
        cout << endl << "=========================================" << endl;
        cout << endl << "Znaki na tablicy rejestracyjnej = " << lic_plate.chars << endl;     
        cout << endl << "=========================================" << endl;
        cout << "\n\n\n\n\n\n\n\n\n";

        // wypisz tablic� na obrazie
        chars_in_plates(img_original, lic_plate);           

        // poka� ca�y obraz
        imshow("img_original_with_lic_plate", img_original);                       

        // zapisz obraz do pliku
        imwrite("imgOriginalScene.png", img_original);                  
    }

    // wstrzymuje obraz dop�ki nie b�dzie naci�ni�ty klawisz
    waitKey(0);              

    return(0);
}

void rect_around_plate(Mat &img_original, PossiblePlate &lic_plate) {
    
    // punkt dwuwymiarowy
    Point2f rect_points[4];

    // uzyskanie 4 wierzcho�k�w obr�conego prostok�ta
    lic_plate.plate_location.points(rect_points); 

    // p�tla do wypisania 4 czerwonych linii
    for (int i = 0; i < 4; i++) {
        line(img_original, rect_points[i], rect_points[(i + 1) % 4], S_RED, 2);
    }
}

// funkcja zawieraj�ca czcionk�, po�o�enie tekstu na obrazku itp.
void chars_in_plates(Mat &img_original, PossiblePlate &lic_plate) {
    
    // b�dzie to �rodek obszaru w kt�rym zostanie zapisany tekst
    Point center_point;     

    // b�dzie to lewy dolny obszar obszaru w kt�rym zostanie zapisany tekst
    Point low_left_point;               

    // wyb�r czcionki
    int font = FONT_HERSHEY_SIMPLEX;    

    // podstawowa skala czcionki na wysoko�ci tablicy
    double font_size = (double)lic_plate.plate.rows / 40.0;

    // podstawowa grubo�� czcionki
    int font_tickness = (int)round(font_size * 2.0);
    int baseline = 0;

    // wywo�anie getTextSize
    Size textSize = getTextSize(lic_plate.chars, font, font_size, font_tickness, &baseline);

    // poziome po�o�enie pola tekstowego, jest takie samo jak tablica
    center_point.x = (int)lic_plate.plate_location.center.x;         

    // je�li tablica rejestracyjna znajduje si� 3/4 od g�ry obrazu obrazu
    if (lic_plate.plate_location.center.y < (img_original.rows * 0.75)) {

        // wypisanie znaku pod tablic�
        center_point.y = (int)round(lic_plate.plate_location.center.y) + (int)round((double)lic_plate.plate.rows * 1.0);
    }
    else {
        // je�li tablica rejestracyjna znajduje si� 1/4 od do�u obrazu
        // wypisanie znaku nad tablic�
        center_point.y = (int)round(lic_plate.plate_location.center.y) - (int)round((double)lic_plate.plate.rows * 0.6);
    }

    // oblicza lewy dolny r�g obszaru tekstu
    low_left_point.x = (int)(center_point.x - (textSize.width / 2));

    // na podstawie tekstu oblicza szeroko�� i wysoko��
    low_left_point.y = (int)(center_point.y + (textSize.height / 2));         

    // wypisz tekst na obrazie
    putText(img_original, lic_plate.chars, low_left_point, font, font_size, S_YELLOW, font_tickness);
}