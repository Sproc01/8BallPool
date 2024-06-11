// Author: Michele Sprocatti

#ifndef SEGMENTATION_H
#define SEGMENTATION_H

#include <opencv2/opencv.hpp>
#include "ball.h"
#include "table.h"

void segmentTable(const cv::Mat &frame, const cv::Vec<cv::Point2f, 4> &tableCorners, cv::Mat &segmented);
void segmentBalls(const cv::Mat &frame, const std::vector<Ball> &balls, cv::Mat &segmented);

#endif // SEGMENTATION_H
