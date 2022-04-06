#include "PotentialChar.h"

PotentialChar::PotentialChar(vector<Point> _contour) {
    contour = _contour;

    boundingRect = cv::boundingRect(contour);

    x_center = (boundingRect.x + boundingRect.x + boundingRect.width) / 2;
    y_center = (boundingRect.y + boundingRect.y + boundingRect.height) / 2;

    diagonal_size = sqrt(pow(boundingRect.width, 2) + pow(boundingRect.height, 2));

    ratio_aspect = (float)boundingRect.width / (float)boundingRect.height;
}

