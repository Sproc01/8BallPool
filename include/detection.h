// Author: Michele Sprocatti

#ifndef DETECTION_H
#define DETECTION_H

#include <opencv2/opencv.hpp>
#include "ball.h"
#include "table.h"

void detectTable(const cv::Mat &frame, cv::Vec<cv::Point2f, 4> &tableCorners, cv::Vec2b &colorTable);
void detectBalls(const cv::Mat &frame, std::vector<Ball> &balls, const cv::Vec<cv::Point2f, 4> &tableCorners, const cv::Scalar &colorTable);

#endif // DETECTION_H
