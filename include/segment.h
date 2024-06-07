// Author: Michele Sprocatti

#ifndef SEGMENT_H
#define SEGMENT_H

#include <opencv2/opencv.hpp>
#include "ball.h"
#include "table.h"

void segmentTable(const cv::Mat &frame, const std::vector<cv::Point> &tableCorners, cv::Mat &segmented);
void segmentBalls(const cv::Mat &frame, const std::vector<Ball> &balls, cv::Mat &segmented);

#endif // SEGMENT_H