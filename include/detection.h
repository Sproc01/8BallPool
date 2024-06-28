// Author: Michele Sprocatti

#ifndef DETECTION_H
#define DETECTION_H

#include <opencv2/opencv.hpp>
#include "ball.h"
#include "table.h"

// S>50, V>90 to be a color and not black or white
const int S_CHANNEL_COLOR_THRESHOLD = 50;
const int V_CHANNEL_COLOR_THRESHOLD = 80;

void detectTable(const cv::Mat &frame, cv::Vec<cv::Point2f, 4> &tableCorners, cv::Vec2b &colorTable);
void detectBalls(const cv::Mat &frame, std::vector<Ball> &balls, const cv::Vec<cv::Point2f, 4> &tableCorners, const cv::Scalar &colorTable);

#endif // DETECTION_H
