//Author: Michela Schibuola

#ifndef TABLEORIENTATION_H
#define TABLEORIENTATION_H

#include <opencv2/opencv.hpp>

void orderTableCornersByOrientation(cv::Mat frame, cv::Vec<cv::Point, 4> &corners, cv::Vec2b background_color);

#endif //TABLEORIENTATION_H
