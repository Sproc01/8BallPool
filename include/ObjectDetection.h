// Author: Michele Sprocatti

#ifndef OBJECTDETECTION_H
#define OBJECTDETECTION_H

#include <opencv2/opencv.hpp>
#include "ball.h"
#include "table.h"

void detectTable(cv::Mat &frame);
void detectBalls(cv::Mat &frame, std::vector<Ball> &balls);

#endif // OBJECTDETECTION_H