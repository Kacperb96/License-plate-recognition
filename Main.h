#ifndef MY_MAIN
#define MY_MAIN

// biblioteki C++
#include<iostream>
#include<conio.h>  

// biblioteki openCV
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

// pliki nag³ówkowe
#include "PlateIdentified.h"
#include "PotentialPlate.h"
#include "CharsIdentified.h"

using namespace std;
using namespace cv;

// zmienne globalne (deklaracja scalar)
const Scalar S_WHITE = Scalar(255.0, 255.0, 255.0);
const Scalar S_BLACK = Scalar(0.0, 0.0, 0.0);
const Scalar S_RED = Scalar(0.0, 0.0, 255.0);
const Scalar S_GREEN = Scalar(0.0, 255.0, 0.0);
const Scalar S_YELLOW = Scalar(0.0, 255.0, 255.0);

// deklaracje funkcji
int main();
void rect_around_plate(Mat&, PossiblePlate&);
void chars_in_plates(Mat&, PossiblePlate&);

# endif