// Author: Michele Sprocatti

#ifndef SEGMENT_H
#define SEGMENT_H

#include <opencv2/opencv.hpp>
#include "ball.h"
#include "table.h"

void segmentTable(cv::Mat &frame, const std::vector<cv::Point> &tableCorners);
void segmentBalls(cv::Mat &frame, const std::vector<Ball> &balls);

#endif // SEGMENT_H