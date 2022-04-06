#include "CharsIdentified.h"

using namespace std;
using namespace cv;

Ptr<ml::KNearest> KNN = ml::KNearest::create();

bool load_knn_and_train(void) {

    // wczytanie modelu knn
    Mat mat_clasify;              

    // otwarcie pliku
    FileStorage clasifications("classifications.xml", FileStorage::READ);        

    if (clasifications.isOpened() == false) {          // jeœli plik nie zosta³ otwarty poprawnie
        cout << "Nie mozna otworzyc pliku\n\n";        // wyœwietlono komunikat
        return(false);                                 // i zamkniêto program
    }

    clasifications["classifications"] >> mat_clasify;          // wczytanie pliku klasyfikacji jako zmienna Mat 
    clasifications.release();                                  // zamkniêcie pliku klasyfikacji

    // wczytanie treningowych obrazow
    Mat mat_training_images;                          // wczytanie wielu obrazów do pojedynczej zmiennej obrazu, jakby by³ to wektor
    FileStorage training_images("images.xml", FileStorage::READ);  // otwarcie pliku treningowych obrazow

    if (training_images.isOpened() == false) {         // jesli plik nie zostal otwarty poprawnie
        cout << "Nie mozna otworzyc pliku\n\n";        // wyswietlono komunikat
        return(false);                                 // i zamkniêto program
    }

    training_images["images"] >> mat_training_images;           // wczytanie obrazów do zmiennej Mat
    training_images.release();                                  // zamkniêcie pliku treningowych obrazow
        
 // wywolanie treningu, wazne ze oba parametry sa typu Mat
 // w rzeczywistosci sa to obrazy / liczby
    KNN->setDefaultK(1);
    KNN->train(mat_training_images, ml::ROW_SAMPLE, mat_clasify);

    return true;
}

vector<PossiblePlate> detect_chars_in_plates(vector<PossiblePlate> &potential_plate) {
    
    // to tylko do pokazania kroków
    int plate_counter = 0;	
    Mat img_contour;
    vector<vector<Point> > contours;

    // jeœli wektor bêdzie pusty
    if (potential_plate.empty()) {               
        return(potential_plate);              
    }
    // tutaj trzeba byæ pewnym ¿e mamy przynajmniej jedn¹ tablicê
    for (auto &possible_plate : potential_plate) {            

        // wstêpne przetwarzanie ¿eby uzyskaæ obraz w skali szaroœci i threshhold
        preprocess(possible_plate.plate, possible_plate.img_gray, possible_plate.img_thresh);    

        // powiêkszamy obraz o 60% dla lepszego rozpoznawania
        resize(possible_plate.img_thresh, possible_plate.img_thresh, Size(), 1.6, 1.6);

        // threshold ¿eby wyeliminowaæ szare strefy
        threshold(possible_plate.img_thresh, possible_plate.img_thresh, 0.0, 255.0, THRESH_BINARY | THRESH_OTSU);

        // ta funkcja najpierw wyszukuje wszystkie kontury, a nastêpnie uwzglêdnia tylko kontury, które mog¹ byæ znakami (jeszcze bez porównania z innymi znakami)
        vector<PotentialChar> possible_chars_in_plate_vector = possible_chars_in_plate(possible_plate.img_gray, possible_plate.img_thresh);

        // maj¹c wektor wszystkich mo¿liwych znaków znajduje grupy pasuj¹cych znaków w tablicy
        vector<vector<PotentialChar> > matching_chars_in_plate_vector = find_correct_chars_vector(possible_chars_in_plate_vector);
        
        // jeœli na tablicy nie znaleziono grup pasuj¹cych znaków
        if (matching_chars_in_plate_vector.size() == 0) {               

            // ustaw zmienn¹ jako pusty ci¹g
            possible_plate.chars = "";           
            
            // wróæ do pêtli for
            continue;                              
        }

        // dla ka¿dego wektora pasuj¹cych znaków w bie¿¹cej tablicy
        for (auto &vector_of_matching_chars : matching_chars_in_plate_vector) { 

            // sortuj znaki od lewej do prawej
            sort(vector_of_matching_chars.begin(), vector_of_matching_chars.end(), PotentialChar::sorting_chars);      
            
            // wyeliminuj wszelkie nak³adaj¹ce siê znaki
            vector_of_matching_chars = remove_same_chars(vector_of_matching_chars);                                    
        }

        // w obrêbie ka¿dej mo¿liwej tablicy
        // za³ó¿my ¿e najd³u¿szy wektor potencjalnych pasuj¹cych znaków jest rzeczywistym wektorem znaków
        unsigned short longest_vector_chars = 0;
        unsigned short index_of_longest_vector_chars = 0;

        // przejdŸ przez wszystkie wektory pasuj¹cych znaków
        // uzyskaj indeks tego z najwiêksz¹ liczb¹ znaków
        for (unsigned int i = 0; i < matching_chars_in_plate_vector.size(); i++) {
            if (matching_chars_in_plate_vector[i].size() > longest_vector_chars) {
                longest_vector_chars = matching_chars_in_plate_vector[i].size();
                index_of_longest_vector_chars = i;
            }
        }
        // za³ó¿my ¿e najd³u¿szy wektor pasuj¹cych znaków w tablicy jest rzeczywistym wektorem znaków
        vector<PotentialChar> longest_vector_of_matching_chars_plate = matching_chars_in_plate_vector[index_of_longest_vector_chars];

        // wykonaj rozpoznawanie znaków na najd³u¿szym wektorze pasuj¹cych znaków w tablicy
        possible_plate.chars = chars_in_plate(possible_plate.img_thresh, longest_vector_of_matching_chars_plate);

    }   // koniec du¿ej pêtli for

    return(potential_plate);
}

vector<PotentialChar> possible_chars_in_plate(Mat &img_gray, Mat &img_thresh) {

    // to bêdzie zwracana wartoœæ
    vector<PotentialChar> possible_chars_vector;                           
    Mat img_thresh_copy;

    vector<vector<Point> > contours;

    // zrobienie kopii bo findContours zmodyfikuje obraz
    img_thresh_copy = img_thresh.clone();				

    // znajduje wszystkie kontury w tablicy
    findContours(img_thresh_copy, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);       

    // dla ka¿dego konturu 
    for (auto &contour : contours) {                         
        PotentialChar possible_character(contour);

        // jeœli kontur jest mo¿liwym znakiem to nie jest to porównywany jeszcze z innymi znakami 
        if (if_possible_char(possible_character)) {  

            // dodaje do wektora mo¿liwych znaków
            possible_chars_vector.push_back(possible_character);     
        }
    }

    return(possible_chars_vector);
}


bool if_possible_char(PotentialChar &potential_char) {
    // ta funkcja jest pierwszym przejœciem które wstêpnie sprawdza kontur aby zobaczyæ czy mo¿e to byæ znak 
    // nie porównujemy jeszcze znaku z innymi znakami aby znaleŸæ grupê

    if (potential_char.boundingRect.area() > AREA_PIXEL &&
        potential_char.boundingRect.width > WIDTH_PIXEL && potential_char.boundingRect.height > HEIGHT_PIXEL &&
        MINIMUM_RATIO < potential_char.ratio_aspect && potential_char.ratio_aspect < MAXIMUM_RATIO) {
        return(true);
    }
    else {
        return(false);
    }
}

vector<vector<PotentialChar> > find_correct_chars_vector(const vector<PotentialChar> &potential_chars_vector) {
    // dziêki tej funkcji zaczynamy od wszystkich mo¿liwych znaków w jednym du¿ym wektorze
    // celem tej funkcji jest przeorganizowanie jednego du¿ego wektora znaków w wektor wektorów pasuj¹cych znaków
    // znaki które nie znajduj¹ siê w grupie dopasowañ nie musz¹ byæ dalej rozwa¿ane
    
    // to bêdzie zwracana wartoœæ
    vector<vector<PotentialChar> > matching_chars_vector;             

    // pêtla dla ka¿dego mo¿liwego znaku w jednym du¿ym wektorze znaków
    for (auto &possible_character : potential_chars_vector) {                

        // znajduje wszystkie znaki w du¿ym wektorze, które pasuj¹ do bie¿¹cego znaku
        vector<PotentialChar> vector_of_matching_chars = find_correct_chars(possible_character, potential_chars_vector);

        // dodaje bie¿¹cy znak do bie¿¹cego mo¿liwego wektora pasuj¹cych znaków
        vector_of_matching_chars.push_back(possible_character);

        // if dla aktualnego mo¿liwego wektora pasuj¹cych znaków
        // jeœli nie jest wystarczaj¹co d³ugi ¿eby stanowiæ mo¿liw¹ tablicê
        if (vector_of_matching_chars.size() < MIN_MATCHING_CHARS) {

            // skocz z powrotem na pocz¹tek pêtli for i spróbuj ponownie z nastêpnym znakiem
            continue;                                                   
        }

        // dodaje do naszego wektora wektor pasuj¹cych znaków
        matching_chars_vector.push_back(vector_of_matching_chars);   

        // usuwa bie¿¹cy wektor pasuj¹cych znaków z du¿ego wektora ¿eby nie u¿ywaæ tych samych znaków dwa razy
        // trzeba siê upewniæ ¿e jest nowy du¿y wektor bo nie chcemy zmieniaæ oryginalnego du¿ego wektora
        vector<PotentialChar> vector_of_possible_chars_with_matches_removed;

        for (auto &poss_chars : potential_chars_vector) {
            if (find(vector_of_matching_chars.begin(), vector_of_matching_chars.end(), poss_chars) == vector_of_matching_chars.end()) {
                vector_of_possible_chars_with_matches_removed.push_back(poss_chars);
            }
        }
        // deklarowanie nowego wektora znaków ¿eby otrzymaæ wynik z rekurencyji
        vector<vector<PotentialChar> > recursive_matching_chars_vector;

        // wywo³anie rekurencji
        recursive_matching_chars_vector = find_correct_chars_vector(vector_of_possible_chars_with_matches_removed);

        // dla ka¿dego wektora pasuj¹cych znaków znalezionych przez rekurencje
        for (auto &recursive_matching_cars : recursive_matching_chars_vector) {

            // dodaje do naszego oryginalnego wektora wektorów pasuj¹cych znaków
            matching_chars_vector.push_back(recursive_matching_cars);
        }

        // wyjœcie z pêtli
        break;
    }

    return(matching_chars_vector);
}


// celem tej funkcji jest podanie mo¿liwego znaku i du¿ego wektora mo¿liwych znaków,
// znajduje wszystkie znaki w du¿ym wektorze które pasuj¹ do pojedynczego mo¿liwego znaku i zwraca te pasuj¹ce znaki jako wektor
vector<PotentialChar> find_correct_chars(const PotentialChar &potential_char, const vector<PotentialChar> &chars_vector) {

    // to bêdzie wartoœæ zwracana
    vector<PotentialChar> matching_chars_vector;

    // pêtla dla ka¿dego znaku w du¿ym wektorze
    for (auto &possible_matching_chars : chars_vector) {

        // jeœli znak dla którego próbujemy znaleŸæ pasuje, jest dok³adnie tym samym znakiem
        // co znak w du¿ym wektorze który aktualnie sprawdzamy
        if (possible_matching_chars == potential_char) {

        // wtedy nie powinno siê umieszczaæ go w wektorze, który zakoñczy³by siê podwójnie w³¹czaj¹c w to bie¿¹cy znak
            continue;
        }

        // oblicza zmienne ¿eby sprawdziæ czy znaki pasuj¹
        double dbl_chars_distance = chars_distance(potential_char, possible_matching_chars);
        double dbl_chars_angle = chars_angle(potential_char, possible_matching_chars);
        double dbl_change_area = (double)abs(possible_matching_chars.boundingRect.area() - potential_char.boundingRect.area()) / (double)potential_char.boundingRect.area();
        double dbl_width_change = (double)abs(possible_matching_chars.boundingRect.width - potential_char.boundingRect.width) / (double)potential_char.boundingRect.width;
        double dbl_height_change = (double)abs(possible_matching_chars.boundingRect.height - potential_char.boundingRect.height) / (double)potential_char.boundingRect.height;

        // sprawdza czy znak pasuje
        if (dbl_chars_distance < (potential_char.diagonal_size * MAXIMUM_DIAGONAL) &&
            dbl_chars_angle < CHARS_ANGLE &&
            dbl_change_area < CHANGING_AREA &&
            dbl_width_change < WIDTH_CHANGE &&
            dbl_height_change < HEIGHT_CHANGE) {

            // jeœli znaki s¹ zgodne dodaje bie¿¹cy znak do wektora pasuj¹cych znaków
            matching_chars_vector.push_back(possible_matching_chars);      
        }
    }

    return(matching_chars_vector);  
}

// u¿yto twierdzenia Pitagorasa do obliczenia odleg³oœci miêdzy znakami
double chars_distance(const PotentialChar &first_char, const PotentialChar &second_char) {
    int x = abs(first_char.x_center - second_char.x_center);
    int y = abs(first_char.y_center - second_char.y_center);

    return(sqrt(pow(x, 2) + pow(y, 2)));
}

// tutaj u¿yto trygonometrii do obliczenia k¹ta miêdzy znakami
double chars_angle(const PotentialChar &first_char, const PotentialChar &second_char) {
    double adj = abs(first_char.x_center - second_char.x_center);
    double opp = abs(first_char.y_center - second_char.y_center);

    double radian_angle = atan(opp / adj);

    double degres_angle = radian_angle * (180.0 / CV_PI);

    return(degres_angle);
}

// jeœli mamy dwa znaki nak³adaj¹ce siê lub zbyt blisko siebie, ¿eby ewentualnie byæ oddzielnymi znakami, usuwamy wewnêtrzny(mniejszy) znak,
// zapobiega to temu ¿e dwa razy w³¹czamy ten sam znak jeœli dla tego samego znaku zostan¹ znalezione dwa kontury,
// na przyk³ad dla litery „O” pierœcieñ wewnêtrzny i pierœcieñ zewnêtrzny mo¿na znaleŸæ jako kontury ale powinniœmy zawrzeæ znak tylko raz
vector<PotentialChar> remove_same_chars(vector<PotentialChar> &matching_chars_vector) {
    vector<PotentialChar> matching_chars_with_inner_chars_removed(matching_chars_vector);

    for (auto &current_character : matching_chars_vector) {
        for (auto &other_character : matching_chars_vector) {

            // jeœli bie¿¹cy znak i inne znaki nie s¹ tym samym znakiem
            if (current_character != other_character) {     

                // jeœli obecny znak i inny znak maj¹ punkty œrodkowe w prawie tym samym miejscu
                if (chars_distance(current_character, other_character) < (current_character.diagonal_size * MINIMUM_DIAGONAL)) {

                    // jeœli dostaniemy siê tutaj, znaleŸliœmy nak³adaj¹ce siê znaki
                    // nastêpnie identyfikujemy, który znak jest mniejszy a jeœli ten znak nie zosta³ ju¿ usuniêty w poprzednim przebiegu usuwamy go

                    // if dla sprawdzenia jeœli obecny znak jest mniejszy od innego
                    if (current_character.boundingRect.area() < other_character.boundingRect.area()) {

                        // szuka znaku w wektorze za pomoc¹ iteratora
                        vector<PotentialChar>::iterator currentCharIterator = find(matching_chars_with_inner_chars_removed.begin(), matching_chars_with_inner_chars_removed.end(), current_character);
                        
                        // jeœli iterator siê nie zakoñczy³, to znak zosta³ znaleziony w wektorze
                        if (currentCharIterator != matching_chars_with_inner_chars_removed.end()) {

                            // usuwa znak
                            matching_chars_with_inner_chars_removed.erase(currentCharIterator);
                        }
                    }

                    else {
                    // jeœli inny znak jest mniejszy od obecnego
                    // patrzymy na znak w wektorze za pomoc¹ iteratora
         
                        vector<PotentialChar>::iterator otherCharIterator = find(matching_chars_with_inner_chars_removed.begin(), matching_chars_with_inner_chars_removed.end(), other_character);
                        
                        // jeœli iterator siê nie zakoñczy³ to znak nie zosta³ znaleziony w wektorze  
                        if (otherCharIterator != matching_chars_with_inner_chars_removed.end()) {

                            // usuwa znak
                            matching_chars_with_inner_chars_removed.erase(otherCharIterator);
                        }
                    }
                }
            }
        }
    }

    return(matching_chars_with_inner_chars_removed);
}

// tutaj jest funckja gdzie stosujemy rozpoznanie znaków
string chars_in_plate(Mat &img_thresh, vector<PotentialChar> &matching_chars_vector) {
    
    // to bêdzie znak w tablicy
    string char_in_plate;              

    Mat img_thresh_color;

    // sortuje z lewej do prawej
    sort(matching_chars_vector.begin(), matching_chars_vector.end(), PotentialChar::sorting_chars);
    
    // robi kolorow¹ wersjê threshold wiêc mo¿na narysowaæ kontur na kolorowym obrazie
    cvtColor(img_thresh, img_thresh_color, cv::COLOR_GRAY2BGR);      

    // dla ka¿dego znaku w tablicy
    for (auto &current_char : matching_chars_vector) {

        // rysuje zielony kontur dooko³a znaku
        rectangle(img_thresh_color, current_char.boundingRect, S_GREEN, 2);

        // ROI boundingRect 
        Mat img_ROI_copy = img_thresh(current_char.boundingRect);

        // kopiuje ROI wiêc nie bêdzie zmiany w oryginalnym obrazie
        Mat img_ROI = img_ROI_copy.clone();      

        Mat img_ROI_resized;

        // zmiana wymiarów potrzebna do rozpoznania znaków
        resize(img_ROI, img_ROI_resized, Size(IMAGE_WIDTH, IMAGE_HEIGHT));

        Mat ROI_float;

        // konwersja Mat na float potrzebna do findNearest
        img_ROI_resized.convertTo(ROI_float, CV_32FC1);         

        // zmieniamy macierz w jeden rz¹d 
        Mat ROI_flatten = ROI_float.reshape(1, 1);

        // deklaruje ¿e Mat ma wczytaæ obecny znak
        Mat m_current_char(0, 0, CV_32F);

        // wywo³anie findNearest
        KNN->findNearest(ROI_flatten, 1, m_current_char);

        // zmieniamy obecny znak z Mat na float
        float fltCurrentChar = (float)m_current_char.at<float>(0, 0); 

        // do³¹czanie bie¿¹cego znaku do string
        char_in_plate = char_in_plate + char(int(fltCurrentChar));        
    }

    return(char_in_plate);       
}

