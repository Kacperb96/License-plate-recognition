#ifndef POTENTIAL_CHAR_H
#define POTENTIAL_CHAR_H

#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/core/core.hpp>

using namespace std;
using namespace cv;

class PotentialChar {
public:
    // zmienne
    Rect boundingRect;
    vector<Point> contour;

    int x_center;
    int y_center;

    double diagonal_size;
    double ratio_aspect;

    // operatory
    bool operator == (const PotentialChar& other_possible_char) const {
        if (this->contour == other_possible_char.contour) return true;
        else return false;
    }

    bool operator != (const PotentialChar& other_possible_char) const {
        if (this->contour != other_possible_char.contour) return true;
        else return false;
    }

    // delkaracja funkcji
    PotentialChar(vector<Point> _contour);

    // funckja statyczna sortuj¹ca znaki od lewej do prawej
    static bool sorting_chars(const PotentialChar& left_char, const PotentialChar& right_char) {
        return(left_char.x_center < right_char.x_center);
    }
};

#endif