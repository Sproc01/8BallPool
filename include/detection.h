// Author: Michele Sprocatti

#ifndef DETECTION_H
#define DETECTION_H

#include <opencv2/opencv.hpp>
#include "ball.h"
#include "table.h"

void detectTable(const cv::Mat &frame, cv::Vec<cv::Point2f, 4> &tableCorners, cv::Vec2b &colorTable);
void detectBalls(const cv::Mat &frame, std::vector<Ball> &balls, const cv::Vec<cv::Point2f, 4> &tableCorners);
cv::Vec2b histogram(const cv::Mat &img);
void computeIntersection(const cv::Vec3f &line1, const cv::Vec3f &line2, cv::Point2f &intersection);
void equationFormula(float x1, float y1, float x2, float y2, float &a, float &b, float &c);

#endif // DETECTION_H
