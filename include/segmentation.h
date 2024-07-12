// Author: Michele Sprocatti

#ifndef SEGMENTATION_H
#define SEGMENTATION_H

#include <opencv2/opencv.hpp>
#include "ball.h"
#include "table.h"


/**
 * @brief segment the table in the input image.
 * @param frame input image.
 * @param table initialized object containing information about the table in the input image.
 * @param segmented output image where the table is green.
 * @throw invalid_argument if frame is empty.
 */
void segmentTable(const cv::Mat &frame, const Table& table, cv::Mat& segmented);

/**
 * @brief segment the input image by highlight the balls.
 * @param frame input image.
 * @param balls vector of the balls in the image.
 * @param segmented output image where each category of the ball correspond to a different color.
 * @throw invalid_argument if frame is empty or if balls is empty.
 */
void segmentBalls(const cv::Mat &frame, const std::vector<Ball> &balls, cv::Mat &segmented);

#endif // SEGMENTATION_H
