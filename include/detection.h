// Author: Michele Sprocatti

#ifndef DETECTION_H
#define DETECTION_H

#include <opencv2/opencv.hpp>
#include "ball.h"
#include "table.h"

/**
 * @brief detect the corners of the table and its color in an image.
 * @param frame image where there is a table to be detected.
 * @param corners output vector containing the 4 corners found.
 * @param colorRange output vector containing a range for the table colors.
 * @throw runtime_error if it does not find enough lines.
 * @throw runtime_error if it does not find enough interceptions.
 * @throw runtime_error if too many interceptions.
 * @throw invalid_argument if frame is empty.
 */
void detectTable(const cv::Mat &frame, cv::Vec<cv::Point2f, 4> &tableCorners, cv::Vec2b &colorTable);

/**
 * @brief classify the ball inside the image passed as argument.
 * @param img image that contains only one ball centered in the center of the ball.
 * @param radius radius of the circle that correspond to the ball.
 * @return Category class of the ball.
 * @throw invalid_argument if img is empty or if the radius is <=0.
 */
Category classificationBall(const cv::Mat& img, double radius);

/**
 * @brief detect balls in an image given some information about the table.
 * @param frame image where there are the balls to be detected.
 * @param table initialized object that contains the corner and the color.
 * @param balls output vector of the balls detected.
 * @throw invalid_argument if frame is empty.
 */
void detectBalls(const cv::Mat &frame, const Table &table, std::vector<Ball> &balls);

#endif // DETECTION_H
