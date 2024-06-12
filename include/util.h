//Author:

#ifndef UTIL_H
#define UTIL_H

#include <opencv2/opencv.hpp>

cv::Point2f getCenter(cv::Point2f p1, cv::Point2f p2);
cv::Vec2b histogram(const cv::Mat &img);
void computeIntersection(const cv::Vec3f &line1, const cv::Vec3f &line2, cv::Point2f &intersection);
void equationFormula(float x1, float y1, float x2, float y2, float &a, float &b, float &c);

#endif //UTIL_H
