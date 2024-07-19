// Author: Michela Schibuola

#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include <opencv2/opencv.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/mat.hpp>
#include <vector>
#include "ball.h"

cv::Mat computeTransformation(const cv::Mat& img, cv::Vec<cv::Point2f, 4>  &img_corners);
cv::Mat drawMinimap(cv::Mat &minimap_with_track, const cv::Mat &transform, cv::Ptr<std::vector<Ball>> balls);

#endif //TRANSFORMATION_H
