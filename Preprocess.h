#ifndef PREPROCESS_H
#define PREPROCESS_H

// biblioteki openCV
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/imgproc/imgproc_c.h>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/core/core.hpp>

// inne biblioteki
#include <iostream>

using namespace std;
using namespace cv;

// zmienne globalne
const int THRESH_WEIGHT = 9;
const int THRESH_SIZE = 19;

// gauss
const Size SMOOTH_FILTER = Size(5, 5);

// deklaracje funkcji
Mat extract_value(Mat&);
Mat contrast(Mat&);

void preprocess(Mat&, Mat&, Mat&);

#endif