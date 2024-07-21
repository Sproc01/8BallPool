// Author: Michela Schibuola

#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include <opencv2/opencv.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/mat.hpp>
#include <vector>
#include "ball.h"

/**
 * @brief Compute the transformation matrix.
 * @param img original image used to check the orientation of the table.
 * @param img_corners corners of the table in the original image.
 * @return transformation matrix.
 * @throw invalid_argument if the image in input is empty
 */
cv::Mat computeTransformation(const cv::Mat& img, cv::Vec<cv::Point2f, 4>  &img_corners);

/**
 * @brief Draw the balls and their tracking on the minimap.
 * @param minimap_with_track minimap image in which the tracking lines are kept.
 * @param transform transformation matrix.
 * @param balls vector of balls containing their positions in the original image.
 * @return minimap image with tracking lines and balls.
 * @throw invalid_argument if the image in input is empty
 * @throw invalid_argument if the transformation matrix in input is empty
 * @throw invalid_argument if the balls pointer is a null pointer
 */
cv::Mat drawMinimap(cv::Mat &minimap_with_track, const cv::Mat &transform, cv::Ptr<std::vector<Ball>> balls);

#endif //TRANSFORMATION_H
