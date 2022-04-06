#include "Preprocess.h"

using namespace std;
using namespace cv;

// przetwarzanie
void preprocess(Mat &img_original, Mat &img_gray, Mat &img_thresh) {

    // wyodrêbnia wartoœci tylko z oryginalnego obrazu ¿eby uzyskaæ img_gray
    img_gray = extract_value(img_original);  

    // maksymalizacja kontrastu
    Mat contrast_gray = contrast(img_gray);       

    Mat img_blurr;

    GaussianBlur(contrast_gray, img_blurr, SMOOTH_FILTER, 0);    

    adaptiveThreshold(img_blurr, img_thresh, 255.0, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, THRESH_SIZE, THRESH_WEIGHT);
}

Mat extract_value(Mat &img_original) {
    Mat img_hsv;
    vector<Mat> hsv_img_vector;
    Mat img_value;

    // konwersja z BGR na HSV
    cvtColor(img_original, img_hsv, COLOR_BGR2HSV);

    split(img_hsv, hsv_img_vector);

    img_value = hsv_img_vector[2];

    return(img_value);
}

Mat contrast(Mat &img_gray) {
    Mat img_top_hat;
    Mat img_black_hat;
    Mat img_gray_and_top_hat;
    Mat img_gray_and_top_hat_minus_black_hat;

    // getStructuringElement tworzy zera i jedynki
    Mat structuring_element = getStructuringElement(CV_SHAPE_RECT, Size(3, 3));

    // Przekszta³cenia morfologiczne to kilka prostych operacji opartych na kszta³cie obrazu
    // jest to zwykle wykonywane na obrazach binarnych
    morphologyEx(img_gray, img_top_hat, CV_MOP_TOPHAT, structuring_element);
    morphologyEx(img_gray, img_black_hat, CV_MOP_BLACKHAT, structuring_element);

    img_gray_and_top_hat = img_gray + img_top_hat;
    img_gray_and_top_hat_minus_black_hat = img_gray_and_top_hat - img_black_hat;

    return(img_gray_and_top_hat_minus_black_hat);
}


