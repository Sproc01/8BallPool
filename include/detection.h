// Author: Michele Sprocatti

#ifndef DETECTION_H
#define DETECTION_H

#include <opencv2/opencv.hpp>
#include "ball.h"
#include "table.h"

void detectTable(const cv::Mat &frame, cv::Vec<cv::Point2f, 4> &tableCorners, cv::Vec2b &colorTable);
Category classificationBall(const cv::Mat& img, double radius);
void detectBalls(const cv::Mat &frame, const Table &table, std::vector<Ball> &balls);

#endif // DETECTION_H
