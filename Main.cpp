#include "Main.h"

using namespace std;
using namespace cv;

int main() {

    // KNN trening - wczytanie
    bool KNN_training_successful = load_knn_and_train();         

    // jeœli KNN nie bêdzie wszytany prawid³owo wyœwietl komunikat i zakoñcz program
    if (KNN_training_successful == false) {                            
        cout << endl << endl << "KNN nie zostal wczytany" << endl << endl;
        return(0);                                                      
    }

    // obraz wejœciowy
    Mat img_original;          

    // wczytaj obraz ze œcie¿ki
    img_original = imread("mazda.jpg");        

    // jeœli œcie¿ka nie bêdzie prawid³owa
    if (img_original.empty()) {                             
        cout << "Obraz nie wczytany" << endl << endl;                                                
        return(0);                                           
    }

    // wykrywanie tablic
    vector<PossiblePlate> possible_plates_vector = detect_plates(img_original);

    // wykrywanie znaków w tablicy
    possible_plates_vector = detect_chars_in_plates(possible_plates_vector);

    // poka¿ obraz
    imshow("img_original", img_original);    

    // jeœli tablica nie zostanie wykryta
    if (possible_plates_vector.empty()) {                                               
        cout << endl << "Nie wykryto tablicy" << endl;     
    }

    else {       
        // jeœli otrzymany tutaj wektor mo¿liwych tablic ma co najmniej jedn¹ p³ytkê
        // posortuj wektor mo¿liwych tablic w kolejnoœci malej¹co (od najwiêkszej liczby znaków do najmniejszej liczby znaków)
        sort(possible_plates_vector.begin(), possible_plates_vector.end(), PossiblePlate::descending_sort);

        // za³ó¿my, ¿e tablica z najbardziej rozpoznawanymi znakami (pierwsza tablica posortowana wed³ug d³ugoœci ³añcucha w kolejnoœci malej¹cej) jest rzeczywist¹ tablic¹
        PossiblePlate lic_plate = possible_plates_vector.front();

        // poka¿ tablice (img_thresh i zwyk³a tablica)
        imshow("img_plate", lic_plate.plate);

        // jeœli nie wykryto ¿adnych znaków 
        if (lic_plate.chars.length() == 0) {
            cout << endl << "Nie wykryto znakow" << endl << endl;     
            return(0);                                                                             
        }

        // pokazuje prostok¹t wokó³ tablicy
        rect_around_plate(img_original, lic_plate);
        
        // wypisz tablicê rejestracyjn¹ 
        cout << "\n\n\n\n\n\n\n\n\n";
        cout << endl << "=========================================" << endl;
        cout << endl << "Znaki na tablicy rejestracyjnej = " << lic_plate.chars << endl;     
        cout << endl << "=========================================" << endl;
        cout << "\n\n\n\n\n\n\n\n\n";

        // wypisz tablicê na obrazie
        chars_in_plates(img_original, lic_plate);           

        // poka¿ ca³y obraz
        imshow("img_original_with_lic_plate", img_original);                       

        // zapisz obraz do pliku
        imwrite("imgOriginalScene.png", img_original);                  
    }

    // wstrzymuje obraz dopóki nie bêdzie naciœniêty klawisz
    waitKey(0);              

    return(0);
}

void rect_around_plate(Mat &img_original, PossiblePlate &lic_plate) {
    
    // punkt dwuwymiarowy
    Point2f rect_points[4];

    // uzyskanie 4 wierzcho³ków obróconego prostok¹ta
    lic_plate.plate_location.points(rect_points); 

    // pêtla do wypisania 4 czerwonych linii
    for (int i = 0; i < 4; i++) {
        line(img_original, rect_points[i], rect_points[(i + 1) % 4], S_RED, 2);
    }
}

// funkcja zawieraj¹ca czcionkê, po³o¿enie tekstu na obrazku itp.
void chars_in_plates(Mat &img_original, PossiblePlate &lic_plate) {
    
    // bêdzie to œrodek obszaru w którym zostanie zapisany tekst
    Point center_point;     

    // bêdzie to lewy dolny obszar obszaru w którym zostanie zapisany tekst
    Point low_left_point;               

    // wybór czcionki
    int font = FONT_HERSHEY_SIMPLEX;    

    // podstawowa skala czcionki na wysokoœci tablicy
    double font_size = (double)lic_plate.plate.rows / 40.0;

    // podstawowa gruboœæ czcionki
    int font_tickness = (int)round(font_size * 2.0);
    int baseline = 0;

    // wywo³anie getTextSize
    Size textSize = getTextSize(lic_plate.chars, font, font_size, font_tickness, &baseline);

    // poziome po³o¿enie pola tekstowego, jest takie samo jak tablica
    center_point.x = (int)lic_plate.plate_location.center.x;         

    // jeœli tablica rejestracyjna znajduje siê 3/4 od góry obrazu obrazu
    if (lic_plate.plate_location.center.y < (img_original.rows * 0.75)) {

        // wypisanie znaku pod tablic¹
        center_point.y = (int)round(lic_plate.plate_location.center.y) + (int)round((double)lic_plate.plate.rows * 1.0);
    }
    else {
        // jeœli tablica rejestracyjna znajduje siê 1/4 od do³u obrazu
        // wypisanie znaku nad tablic¹
        center_point.y = (int)round(lic_plate.plate_location.center.y) - (int)round((double)lic_plate.plate.rows * 0.6);
    }

    // oblicza lewy dolny róg obszaru tekstu
    low_left_point.x = (int)(center_point.x - (textSize.width / 2));

    // na podstawie tekstu oblicza szerokoœæ i wysokoœæ
    low_left_point.y = (int)(center_point.y + (textSize.height / 2));         

    // wypisz tekst na obrazie
    putText(img_original, lic_plate.chars, low_left_point, font, font_size, S_YELLOW, font_tickness);
}