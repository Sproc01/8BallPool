// Author: Michele Sprocatti

#ifndef OBJECTDETECTION_H
#define OBJECTDETECTION_H

#include <opencv2/opencv.hpp>

void detectTable(const cv::Mat &frame);
void detectBalls(const cv::Mat &frame);

#endif // OBJECTDETECTION_H