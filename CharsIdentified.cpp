#include "CharsIdentified.h"

using namespace std;
using namespace cv;

Ptr<ml::KNearest> KNN = ml::KNearest::create();

bool load_knn_and_train(void) {

    // wczytanie modelu knn
    Mat mat_clasify;              

    // otwarcie pliku
    FileStorage clasifications("classifications.xml", FileStorage::READ);        

    if (clasifications.isOpened() == false) {          // je�li plik nie zosta� otwarty poprawnie
        cout << "Nie mozna otworzyc pliku\n\n";        // wy�wietlono komunikat
        return(false);                                 // i zamkni�to program
    }

    clasifications["classifications"] >> mat_clasify;          // wczytanie pliku klasyfikacji jako zmienna Mat 
    clasifications.release();                                  // zamkni�cie pliku klasyfikacji

    // wczytanie treningowych obrazow
    Mat mat_training_images;                          // wczytanie wielu obraz�w do pojedynczej zmiennej obrazu, jakby by� to wektor
    FileStorage training_images("images.xml", FileStorage::READ);  // otwarcie pliku treningowych obrazow

    if (training_images.isOpened() == false) {         // jesli plik nie zostal otwarty poprawnie
        cout << "Nie mozna otworzyc pliku\n\n";        // wyswietlono komunikat
        return(false);                                 // i zamkni�to program
    }

    training_images["images"] >> mat_training_images;           // wczytanie obraz�w do zmiennej Mat
    training_images.release();                                  // zamkni�cie pliku treningowych obrazow
        
 // wywolanie treningu, wazne ze oba parametry sa typu Mat
 // w rzeczywistosci sa to obrazy / liczby
    KNN->setDefaultK(1);
    KNN->train(mat_training_images, ml::ROW_SAMPLE, mat_clasify);

    return true;
}

vector<PossiblePlate> detect_chars_in_plates(vector<PossiblePlate> &potential_plate) {
    
    // to tylko do pokazania krok�w
    int plate_counter = 0;	
    Mat img_contour;
    vector<vector<Point> > contours;

    // je�li wektor b�dzie pusty
    if (potential_plate.empty()) {               
        return(potential_plate);              
    }
    // tutaj trzeba by� pewnym �e mamy przynajmniej jedn� tablic�
    for (auto &possible_plate : potential_plate) {            

        // wst�pne przetwarzanie �eby uzyska� obraz w skali szaro�ci i threshhold
        preprocess(possible_plate.plate, possible_plate.img_gray, possible_plate.img_thresh);    

        // powi�kszamy obraz o 60% dla lepszego rozpoznawania
        resize(possible_plate.img_thresh, possible_plate.img_thresh, Size(), 1.6, 1.6);

        // threshold �eby wyeliminowa� szare strefy
        threshold(possible_plate.img_thresh, possible_plate.img_thresh, 0.0, 255.0, THRESH_BINARY | THRESH_OTSU);

        // ta funkcja najpierw wyszukuje wszystkie kontury, a nast�pnie uwzgl�dnia tylko kontury, kt�re mog� by� znakami (jeszcze bez por�wnania z innymi znakami)
        vector<PotentialChar> possible_chars_in_plate_vector = possible_chars_in_plate(possible_plate.img_gray, possible_plate.img_thresh);

        // maj�c wektor wszystkich mo�liwych znak�w znajduje grupy pasuj�cych znak�w w tablicy
        vector<vector<PotentialChar> > matching_chars_in_plate_vector = find_correct_chars_vector(possible_chars_in_plate_vector);
        
        // je�li na tablicy nie znaleziono grup pasuj�cych znak�w
        if (matching_chars_in_plate_vector.size() == 0) {               

            // ustaw zmienn� jako pusty ci�g
            possible_plate.chars = "";           
            
            // wr�� do p�tli for
            continue;                              
        }

        // dla ka�dego wektora pasuj�cych znak�w w bie��cej tablicy
        for (auto &vector_of_matching_chars : matching_chars_in_plate_vector) { 

            // sortuj znaki od lewej do prawej
            sort(vector_of_matching_chars.begin(), vector_of_matching_chars.end(), PotentialChar::sorting_chars);      
            
            // wyeliminuj wszelkie nak�adaj�ce si� znaki
            vector_of_matching_chars = remove_same_chars(vector_of_matching_chars);                                    
        }

        // w obr�bie ka�dej mo�liwej tablicy
        // za��my �e najd�u�szy wektor potencjalnych pasuj�cych znak�w jest rzeczywistym wektorem znak�w
        unsigned short longest_vector_chars = 0;
        unsigned short index_of_longest_vector_chars = 0;

        // przejd� przez wszystkie wektory pasuj�cych znak�w
        // uzyskaj indeks tego z najwi�ksz� liczb� znak�w
        for (unsigned int i = 0; i < matching_chars_in_plate_vector.size(); i++) {
            if (matching_chars_in_plate_vector[i].size() > longest_vector_chars) {
                longest_vector_chars = matching_chars_in_plate_vector[i].size();
                index_of_longest_vector_chars = i;
            }
        }
        // za��my �e najd�u�szy wektor pasuj�cych znak�w w tablicy jest rzeczywistym wektorem znak�w
        vector<PotentialChar> longest_vector_of_matching_chars_plate = matching_chars_in_plate_vector[index_of_longest_vector_chars];

        // wykonaj rozpoznawanie znak�w na najd�u�szym wektorze pasuj�cych znak�w w tablicy
        possible_plate.chars = chars_in_plate(possible_plate.img_thresh, longest_vector_of_matching_chars_plate);

    }   // koniec du�ej p�tli for

    return(potential_plate);
}

vector<PotentialChar> possible_chars_in_plate(Mat &img_gray, Mat &img_thresh) {

    // to b�dzie zwracana warto��
    vector<PotentialChar> possible_chars_vector;                           
    Mat img_thresh_copy;

    vector<vector<Point> > contours;

    // zrobienie kopii bo findContours zmodyfikuje obraz
    img_thresh_copy = img_thresh.clone();				

    // znajduje wszystkie kontury w tablicy
    findContours(img_thresh_copy, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);       

    // dla ka�dego konturu 
    for (auto &contour : contours) {                         
        PotentialChar possible_character(contour);

        // je�li kontur jest mo�liwym znakiem to nie jest to por�wnywany jeszcze z innymi znakami 
        if (if_possible_char(possible_character)) {  

            // dodaje do wektora mo�liwych znak�w
            possible_chars_vector.push_back(possible_character);     
        }
    }

    return(possible_chars_vector);
}


bool if_possible_char(PotentialChar &potential_char) {
    // ta funkcja jest pierwszym przej�ciem kt�re wst�pnie sprawdza kontur aby zobaczy� czy mo�e to by� znak 
    // nie por�wnujemy jeszcze znaku z innymi znakami aby znale�� grup�

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
    // dzi�ki tej funkcji zaczynamy od wszystkich mo�liwych znak�w w jednym du�ym wektorze
    // celem tej funkcji jest przeorganizowanie jednego du�ego wektora znak�w w wektor wektor�w pasuj�cych znak�w
    // znaki kt�re nie znajduj� si� w grupie dopasowa� nie musz� by� dalej rozwa�ane
    
    // to b�dzie zwracana warto��
    vector<vector<PotentialChar> > matching_chars_vector;             

    // p�tla dla ka�dego mo�liwego znaku w jednym du�ym wektorze znak�w
    for (auto &possible_character : potential_chars_vector) {                

        // znajduje wszystkie znaki w du�ym wektorze, kt�re pasuj� do bie��cego znaku
        vector<PotentialChar> vector_of_matching_chars = find_correct_chars(possible_character, potential_chars_vector);

        // dodaje bie��cy znak do bie��cego mo�liwego wektora pasuj�cych znak�w
        vector_of_matching_chars.push_back(possible_character);

        // if dla aktualnego mo�liwego wektora pasuj�cych znak�w
        // je�li nie jest wystarczaj�co d�ugi �eby stanowi� mo�liw� tablic�
        if (vector_of_matching_chars.size() < MIN_MATCHING_CHARS) {

            // skocz z powrotem na pocz�tek p�tli for i spr�buj ponownie z nast�pnym znakiem
            continue;                                                   
        }

        // dodaje do naszego wektora wektor pasuj�cych znak�w
        matching_chars_vector.push_back(vector_of_matching_chars);   

        // usuwa bie��cy wektor pasuj�cych znak�w z du�ego wektora �eby nie u�ywa� tych samych znak�w dwa razy
        // trzeba si� upewni� �e jest nowy du�y wektor bo nie chcemy zmienia� oryginalnego du�ego wektora
        vector<PotentialChar> vector_of_possible_chars_with_matches_removed;

        for (auto &poss_chars : potential_chars_vector) {
            if (find(vector_of_matching_chars.begin(), vector_of_matching_chars.end(), poss_chars) == vector_of_matching_chars.end()) {
                vector_of_possible_chars_with_matches_removed.push_back(poss_chars);
            }
        }
        // deklarowanie nowego wektora znak�w �eby otrzyma� wynik z rekurencyji
        vector<vector<PotentialChar> > recursive_matching_chars_vector;

        // wywo�anie rekurencji
        recursive_matching_chars_vector = find_correct_chars_vector(vector_of_possible_chars_with_matches_removed);

        // dla ka�dego wektora pasuj�cych znak�w znalezionych przez rekurencje
        for (auto &recursive_matching_cars : recursive_matching_chars_vector) {

            // dodaje do naszego oryginalnego wektora wektor�w pasuj�cych znak�w
            matching_chars_vector.push_back(recursive_matching_cars);
        }

        // wyj�cie z p�tli
        break;
    }

    return(matching_chars_vector);
}


// celem tej funkcji jest podanie mo�liwego znaku i du�ego wektora mo�liwych znak�w,
// znajduje wszystkie znaki w du�ym wektorze kt�re pasuj� do pojedynczego mo�liwego znaku i zwraca te pasuj�ce znaki jako wektor
vector<PotentialChar> find_correct_chars(const PotentialChar &potential_char, const vector<PotentialChar> &chars_vector) {

    // to b�dzie warto�� zwracana
    vector<PotentialChar> matching_chars_vector;

    // p�tla dla ka�dego znaku w du�ym wektorze
    for (auto &possible_matching_chars : chars_vector) {

        // je�li znak dla kt�rego pr�bujemy znale�� pasuje, jest dok�adnie tym samym znakiem
        // co znak w du�ym wektorze kt�ry aktualnie sprawdzamy
        if (possible_matching_chars == potential_char) {

        // wtedy nie powinno si� umieszcza� go w wektorze, kt�ry zako�czy�by si� podw�jnie w��czaj�c w to bie��cy znak
            continue;
        }

        // oblicza zmienne �eby sprawdzi� czy znaki pasuj�
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

            // je�li znaki s� zgodne dodaje bie��cy znak do wektora pasuj�cych znak�w
            matching_chars_vector.push_back(possible_matching_chars);      
        }
    }

    return(matching_chars_vector);  
}

// u�yto twierdzenia Pitagorasa do obliczenia odleg�o�ci mi�dzy znakami
double chars_distance(const PotentialChar &first_char, const PotentialChar &second_char) {
    int x = abs(first_char.x_center - second_char.x_center);
    int y = abs(first_char.y_center - second_char.y_center);

    return(sqrt(pow(x, 2) + pow(y, 2)));
}

// tutaj u�yto trygonometrii do obliczenia k�ta mi�dzy znakami
double chars_angle(const PotentialChar &first_char, const PotentialChar &second_char) {
    double adj = abs(first_char.x_center - second_char.x_center);
    double opp = abs(first_char.y_center - second_char.y_center);

    double radian_angle = atan(opp / adj);

    double degres_angle = radian_angle * (180.0 / CV_PI);

    return(degres_angle);
}

// je�li mamy dwa znaki nak�adaj�ce si� lub zbyt blisko siebie, �eby ewentualnie by� oddzielnymi znakami, usuwamy wewn�trzny(mniejszy) znak,
// zapobiega to temu �e dwa razy w��czamy ten sam znak je�li dla tego samego znaku zostan� znalezione dwa kontury,
// na przyk�ad dla litery �O� pier�cie� wewn�trzny i pier�cie� zewn�trzny mo�na znale�� jako kontury ale powinni�my zawrze� znak tylko raz
vector<PotentialChar> remove_same_chars(vector<PotentialChar> &matching_chars_vector) {
    vector<PotentialChar> matching_chars_with_inner_chars_removed(matching_chars_vector);

    for (auto &current_character : matching_chars_vector) {
        for (auto &other_character : matching_chars_vector) {

            // je�li bie��cy znak i inne znaki nie s� tym samym znakiem
            if (current_character != other_character) {     

                // je�li obecny znak i inny znak maj� punkty �rodkowe w prawie tym samym miejscu
                if (chars_distance(current_character, other_character) < (current_character.diagonal_size * MINIMUM_DIAGONAL)) {

                    // je�li dostaniemy si� tutaj, znale�li�my nak�adaj�ce si� znaki
                    // nast�pnie identyfikujemy, kt�ry znak jest mniejszy a je�li ten znak nie zosta� ju� usuni�ty w poprzednim przebiegu usuwamy go

                    // if dla sprawdzenia je�li obecny znak jest mniejszy od innego
                    if (current_character.boundingRect.area() < other_character.boundingRect.area()) {

                        // szuka znaku w wektorze za pomoc� iteratora
                        vector<PotentialChar>::iterator currentCharIterator = find(matching_chars_with_inner_chars_removed.begin(), matching_chars_with_inner_chars_removed.end(), current_character);
                        
                        // je�li iterator si� nie zako�czy�, to znak zosta� znaleziony w wektorze
                        if (currentCharIterator != matching_chars_with_inner_chars_removed.end()) {

                            // usuwa znak
                            matching_chars_with_inner_chars_removed.erase(currentCharIterator);
                        }
                    }

                    else {
                    // je�li inny znak jest mniejszy od obecnego
                    // patrzymy na znak w wektorze za pomoc� iteratora
         
                        vector<PotentialChar>::iterator otherCharIterator = find(matching_chars_with_inner_chars_removed.begin(), matching_chars_with_inner_chars_removed.end(), other_character);
                        
                        // je�li iterator si� nie zako�czy� to znak nie zosta� znaleziony w wektorze  
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

// tutaj jest funckja gdzie stosujemy rozpoznanie znak�w
string chars_in_plate(Mat &img_thresh, vector<PotentialChar> &matching_chars_vector) {
    
    // to b�dzie znak w tablicy
    string char_in_plate;              

    Mat img_thresh_color;

    // sortuje z lewej do prawej
    sort(matching_chars_vector.begin(), matching_chars_vector.end(), PotentialChar::sorting_chars);
    
    // robi kolorow� wersj� threshold wi�c mo�na narysowa� kontur na kolorowym obrazie
    cvtColor(img_thresh, img_thresh_color, cv::COLOR_GRAY2BGR);      

    // dla ka�dego znaku w tablicy
    for (auto &current_char : matching_chars_vector) {

        // rysuje zielony kontur dooko�a znaku
        rectangle(img_thresh_color, current_char.boundingRect, S_GREEN, 2);

        // ROI boundingRect 
        Mat img_ROI_copy = img_thresh(current_char.boundingRect);

        // kopiuje ROI wi�c nie b�dzie zmiany w oryginalnym obrazie
        Mat img_ROI = img_ROI_copy.clone();      

        Mat img_ROI_resized;

        // zmiana wymiar�w potrzebna do rozpoznania znak�w
        resize(img_ROI, img_ROI_resized, Size(IMAGE_WIDTH, IMAGE_HEIGHT));

        Mat ROI_float;

        // konwersja Mat na float potrzebna do findNearest
        img_ROI_resized.convertTo(ROI_float, CV_32FC1);         

        // zmieniamy macierz w jeden rz�d 
        Mat ROI_flatten = ROI_float.reshape(1, 1);

        // deklaruje �e Mat ma wczyta� obecny znak
        Mat m_current_char(0, 0, CV_32F);

        // wywo�anie findNearest
        KNN->findNearest(ROI_flatten, 1, m_current_char);

        // zmieniamy obecny znak z Mat na float
        float fltCurrentChar = (float)m_current_char.at<float>(0, 0); 

        // do��czanie bie��cego znaku do string
        char_in_plate = char_in_plate + char(int(fltCurrentChar));        
    }

    return(char_in_plate);       
}

