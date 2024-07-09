// Author:

#ifndef UTIL_H
#define UTIL_H

#include <opencv2/opencv.hpp>
#include "category.h"

cv::Point2f getCenter(cv::Point2f p1, cv::Point2f p2);
cv::Vec3b getColorFromCategory(Category category);
void rotateCornersClockwise(cv::Vec<cv::Point2f, 4> &corners);
cv::Vec2b mostFrequentColor(const cv::Mat &img);
void computeIntersection(const cv::Vec3f &line1, const cv::Vec3f &line2, cv::Point2f &intersection);
void equationFormula(float x1, float y1, float x2, float y2, float &a, float &b, float &c);
void createOutputImage(const cv::Mat& frame, const cv::Mat& minimap_with_balls, cv::Mat& res);
void kMeansClustering(const cv::Mat &inputImage, cv::Mat& clusteredImage, int clusterCount);

#endif //UTIL_H
