// Author: Michela Schibuola

#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include <opencv2/opencv.hpp>
#include <vector>
#include "ball.h"
#include "table.h"

cv::Mat drawMinimap(cv::Mat &minimap_with_track, cv::Mat transform, std::vector<Ball> balls);
cv::Mat computeTransformation(const cv::Mat& img, const cv::Mat& segmented, cv::Vec<cv::Point2f, 4>  &img_corners);

#endif //TRANSFORMATION_H
