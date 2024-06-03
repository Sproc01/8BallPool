// Author: Michele Sprocatti

#ifndef SEGMENT_H
#define SEGMENT_H

#include <opencv2/opencv.hpp>

void segmentTable(const cv::Mat &frame);
void segmentBalls(const cv::Mat &frame);

#endif // SEGMENT_H