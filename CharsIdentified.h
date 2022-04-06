#ifndef CHARS_IDENTIFIED_H
#define CHARS_IDENTIFIED_H

// biblioteki opencv
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/ml/ml.hpp>

// pliki nag³ówkowe
#include "PotentialPlate.h"
#include "PotentialChar.h"
#include "Preprocess.h"
#include "Main.h"

using namespace std;
using namespace cv;

// zmienne globalne
const double MINIMUM_RATIO = 0.25;
const double MAXIMUM_RATIO = 1.0;

const int AREA_PIXEL = 80;

const int WIDTH_PIXEL = 2;
const int HEIGHT_PIXEL = 8;

// sta³e porównuj¹ce znaki
const double CHANGING_AREA = 0.5;

const double WIDTH_CHANGE = 0.8;
const double HEIGHT_CHANGE = 0.2;

const double CHARS_ANGLE = 12.0;

// minimalna i maksymalna przek¹tna
const double MINIMUM_DIAGONAL = 0.3;
const double MAXIMUM_DIAGONAL = 5.0;

// zmienne extern
extern const bool show_steps;
extern Ptr<ml::KNearest> KNN;

// pozosta³e zmienne
const int IMAGE_WIDTH = 20;
const int IMAGE_HEIGHT = 30;

const int MIN_MATCHING_CHARS = 3;

const int CONTOUR_AREA = 100;

// deklaracje funkcji 
bool if_possible_char(PotentialChar&);
bool load_knn_and_train(void);

double chars_distance(const PotentialChar&, const PotentialChar&);
double chars_angle(const PotentialChar&, const PotentialChar&);

vector<PotentialChar> remove_same_chars(vector<PotentialChar>&);
vector<vector<PotentialChar> > find_correct_chars_vector(const vector<PotentialChar>&);
vector<PossiblePlate> detect_chars_in_plates(vector<PossiblePlate> &);
vector<PotentialChar> find_correct_chars(const PotentialChar&, const vector<PotentialChar>&);
vector<PotentialChar> possible_chars_in_plate(Mat &, Mat &);

string chars_in_plate(Mat&, vector<PotentialChar>&);

#endif