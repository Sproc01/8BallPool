//Author: Michela Schibuola

#ifndef TABLEORIENTATION_H
#define TABLEORIENTATION_H

#include <opencv2/opencv.hpp>

bool checkHorizontalTable(cv::Mat table_img, cv::Vec<cv::Point2f, 4> &corners);

#endif //TABLEORIENTATION_H
