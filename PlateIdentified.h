#ifndef PLATES_IDENTIFIED_H
#define PLATES_IDENTIFIED_H

// biblioteki
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/core/core.hpp>

// pliki nag³ówkowe
#include "PotentialChar.h"
#include "Preprocess.h"
#include "CharsIdentified.h"
#include "Main.h"
#include "PotentialPlate.h"

using namespace std;
using namespace cv;

// zmienne globalne
const double PADDING_FACTOR_WIDTH = 1.3;
const double PADDING_FACTOR_HEIGHT = 1.5;

// deklaracje funkcji
vector<PossiblePlate> detect_plates(Mat &);
vector<PotentialChar> possible_chars(Mat &);

PossiblePlate extract_plate(Mat &, vector<PotentialChar> &);

# endif