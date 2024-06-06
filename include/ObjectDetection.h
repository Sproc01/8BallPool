// Author: Michele Sprocatti

#ifndef OBJECTDETECTION_H
#define OBJECTDETECTION_H

#include <opencv2/opencv.hpp>
#include "ball.h"
#include "table.h"

void detectTable(const cv::Mat &frame, std::vector<cv::Point> &tableCorners);
void detectBalls(const cv::Mat &frame, std::vector<Ball> &balls, const std::vector<cv::Point> &tableCorners);
cv::Vec2b histogram(const cv::Mat &img);
void intersection(const cv::Vec3f &line1, const cv::Vec3f &line2, cv::Point &intersection);
void equationFormula(float x1, float y1, float x2, float y2, float &m, float &q);

#endif // OBJECTDETECTION_H