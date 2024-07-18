// Author:

#ifndef UTIL_H
#define UTIL_H

#include <opencv2/opencv.hpp>
#include "category.h"

cv::Point2f getCenter(cv::Point2f p1, cv::Point2f p2);
cv::Vec3b getColorFromCategory(Category category);
void rotateCornersClockwise(cv::Vec<cv::Point2f, 4> &corners);

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
 * @param minimap_with_balls minimap that must be superimposed onto the input image.
 * @param res output image containing the input image with superimposition of the minimap.
 * @throw invalid_argument if frame or minimap_with_balls are empty.
 */
void createOutputImage(const cv::Mat& frame, const cv::Mat& minimap_with_balls, cv::Mat& res);

/**
 * @brief do the clustering by using only color information on the input image.
 * @param inputImage image to be clustered.
 * @param colors vector containing the different colors for the different clusters,
 * the size of the vector is the number of output clusters.
 * @param clusteredImage output image: original image clustered.
 * @throw invalid_argument if the input image is empty or if colors is empty
 * 							or if inputImage has a number of channels different from 3.
 */
void kMeansClustering(const cv::Mat inputImage, const std::vector<cv::Vec3b> &colors, cv::Mat& clusteredImage);

#endif //UTIL_H
