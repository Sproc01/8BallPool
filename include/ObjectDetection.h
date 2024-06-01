// Author: Michele Sprocatti

#ifndef OBJECTDETECTION_H
#define OBJECTDETECTION_H

#include <opencv2/opencv.hpp>

void detectTable(cv::Mat &frame);
void detectBalls(cv::Mat &frame);

#endif // OBJECTDETECTION_H