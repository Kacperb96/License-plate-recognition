#ifndef POTENTIAL_PLATE_H
#define POTENTIAL_PLATE_H

// biblioteki openCV
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/core/core.hpp>

// inne biblioteki
#include <string>
#include <iostream>

using namespace std;
using namespace cv;

class PossiblePlate {
public:
    // zmienne 
    Mat plate;
    Mat img_thresh;
    Mat img_gray;

    string chars;

    RotatedRect plate_location;

    // funkcja statyczna
    static bool descending_sort(const PossiblePlate &_left, const PossiblePlate &_right) {
        return(_left.chars.length() > _right.chars.length());
    }
};

#endif