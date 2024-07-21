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
 * @throw invalid_argument if frame is empty or if frame has less than 3 channels.
 */
void segmentTable(const cv::Mat &frame, const Table& table, cv::Mat& segmented);

/**
 * @brief segment the input image by highlight the balls.
 * @param frame input image.
 * @param balls pointer to a vector of the balls in the image.
 * @param segmented output image where each category of the ball correspond to a different color.
 * @throw invalid_argument if frame is empty, if frame has less than 3 channels, if balls is nullptr, if balls point to an empty vector.
 */
void segmentBalls(const cv::Mat &frame, cv::Ptr<std::vector<Ball>> balls, cv::Mat &segmented);

#endif // SEGMENTATION_H
