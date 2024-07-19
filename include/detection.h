// Author: Michele Sprocatti

#ifndef DETECTION_H
#define DETECTION_H

#include <opencv2/opencv.hpp>
#include "ball.h"
#include "table.h"

/**
 * @brief detect the corners of the table and its color in an image.
 * @param frame image where there is a table to be detected, BGR format requested.
 * @param corners output vector containing the 4 corners found.
 * @param colorRange output vector containing a range for the table colors.
 * @throw runtime_error if it does not find enough lines or if it does not find enough interceptions.
 * @throw invalid_argument if frame is empty or if frame has a number of channels different from 3.
 */
void detectTable(const cv::Mat &frame, cv::Vec<cv::Point2f, 4> &tableCorners, cv::Vec2b &colorTable);

/**
 * @brief detect balls in an image given some information about the table.
 * @param frame image where there are the balls to be detected, BGR format requested.
 * @param table initialized object that contains the corner and the color, the balls are added in this function.
 * @param frameRect output image containing the input image with the rectangles superimposed.
 * @throw invalid_argument if frame is empty or if frame has a number of channels different from 3.
 */
void detectBalls(const cv::Mat &frame, Table &table, cv::Mat &frameRect);

#endif // DETECTION_H
