// Author: Michele Sprocatti

#ifndef UTIL_H
#define UTIL_H

#include <opencv2/opencv.hpp>
#include "category.h"
#include "ball.h"
#include "table.h"

/**
 * @brief Compute the center between two points.
 * @param p1 first point.
 * @param p2 second point.
 * @return the point which is the center of p1 and p2.
 */
cv::Point2f getCenter(const cv::Point2f &p1, const cv::Point2f &p2);

/**
 * @brief Compute the color of a specific category.
 * @param category element of type Category.
 * @return the color associated to the category in input.
 */
cv::Vec3b getColorFromCategory(const Category &category);

/**
 * @brief Rotate the corners of the table clockwise.
 * @param corners vector of four corners.
 */
void rotateCornersClockwise(cv::Vec<cv::Point2f, 4> &corners);

/**
 * @brief Compute an interval min/max for the radius with respect to the distance and the perspective of the table
 * @param minRadius parameter to store the minimum radius.
 * @param maxRadius parameter to store the maximum radius.
 * @param imgCorners corners of the table in the frame
 */
void radiusInterval(float &minRadius, float &maxRadius, const cv::Vec<cv::Point2f, 4>  &imgCorners);

/**
 * @brief calculate the most frequent value of Hue in the input image.
 * @param img input image in BGR format.
 * @return Vec2b the color interval corresponding to the most frequent Hue.
 * @throw invalid_argument if img is empty.
 */
cv::Vec2b mostFrequentHueColor(const cv::Mat &img);

/**
 * @brief compute intersection of two lines if there is one.
 * @param line1 first line.
 * @param line2 second line.
 * @param intersection output point that corresponds to the intersection if there is one otherwise
 * 						it contains Point(-1,-1).
 */
void computeIntersection(const cv::Vec3f &line1, const cv::Vec3f &line2, cv::Point2f &intersection);

/**
 * @brief Gives the equation of the line passing through two points in the form  ax + by + c = 0.
 * @param x1 x-coordinate of the first point.
 * @param y1 y-coordinate of the first point.
 * @param x2 x-coordinate of the second point.
 * @param y2 y-coordinate of the second point.
 * @param a output parameter: coefficient a.
 * @param b output parameter: coefficient b.
 * @param c output parameter: coefficient c.
 */
void equationFormula(float x1, float y1, float x2, float y2, float &a, float &b, float &c);

/**
 * @brief Create a Output Image object.
 * @param frame input image.
 * @param minimapWithBalls minimap that must be superimposed onto the input image.
 * @param res output image containing the input image with superimposition of the minimap.
 * @throw invalid_argument if frame or minimapWithBalls are empty or if the two images have different number of channels.
 */
void createOutputImage(const cv::Mat &frame, const cv::Mat &minimapWithBalls, cv::Mat &res);

/**
 * @brief do the clustering by using only color information on the input image.
 * @param inputImage image to be clustered.
 * @param colors vector containing the different colors for the different clusters,
 * the size of the vector is the number of output clusters.
 * @param clusteredImage output image: original image clustered.
 * @throw invalid_argument if the input image is empty or if colors is empty
 * 							or if inputImage has a number of channels different from 3.
 */
void kMeansClustering(const cv::Mat &inputImage, const std::vector<cv::Vec3b> &colors, cv::Mat &clusteredImage);

/**
 * @brief push the elements of the first vector in the right vector according to the category.
 * @param gt input vector containing the elements to be separated.
 * @param white output vector containing the white elements.
 * @param black output vector containing the black elements.
 * @param solid output vector containing the solid elements.
 * @param striped output vector containing the striped elements.
 * @throw invalid_argument if gt is empty.
 */
void separateResultGT(const std::vector<std::pair<cv::Rect_<int>, Category>> &gt,
                      std::vector<std::pair<cv::Rect, Category>> &white,
                      std::vector<std::pair<cv::Rect, Category>> &black,
                      std::vector<std::pair<cv::Rect, Category>> &solid,
                      std::vector<std::pair<cv::Rect, Category>> &striped);

/**
 * @brief push the elements of the first vector in the right vector according to the category.
 * @param balls input pointer to a vector a balls that needs to be separated.
 * @param white output vector containing the white balls.
 * @param black output vector containing the black balls.
 * @param solid output vector containing the solid balls.
 * @param striped output vector containing the striped balls.
 * @throw invalid_argument if balls is nullptr or if balls point to an empty vector.
 */
void separateResultBalls(cv::Ptr<std::vector<Ball>> balls, std::vector<Ball> &white, std::vector<Ball> &black,
							std::vector<Ball> &solid, std::vector<Ball> &striped);

/**
 * @brief Draw the bounding boxes of the balls and the table boundaries on the output image.
 * @param img input image.
 * @param table table containing the boundaries and the balls.
 * @param output output image containing the input image with the bounding boxes.
 */
void drawBoundingBoxes(const cv::Mat &img, Table &table, cv::Mat &output);

/**
 * @brief Helper function which enlarges a rectangle by a given amount of pixels on all sides.
 * @param rect rectangle to enlarge.
 * @param px number of pixels to add to each side.
 */
void enlargeRect(cv::Rect &rect, int px);

/**
 * @brief Helper function which shrinks a rectangle by a given amount of pixels on all sides.
 * @param rect rectangle to shrink.
 * @param px number of pixels to remove to each side.
 */
void shrinkRect(cv::Rect &rect, int px);

#endif //UTIL_H
