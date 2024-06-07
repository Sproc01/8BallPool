//Author: Michela Schibuola

#ifndef MINIMAPCONSTANTS_H
#define MINIMAPCONSTANTS_H

#include <string>
#include <opencv2/opencv.hpp>

std::string MAP_PATH = "img/minimap.png";

cv::Point TOP_LEFT_MAP_CORNER = cv::Point(80, 78);
cv::Point TOP_RIGHT_MAP_CORNER = cv::Point(886, 78);
cv::Point COTTOM_RIGHT_MAP_CORNER = cv::Point(886, 481);
cv::Point BOTTOM_LEFT_MAP_CORNER = cv::Point(80, 481);

int BALL_RADIUS = 30;

cv::Vec3d BLACK_RGB_COLOR = cv::Scalar(0, 0, 0);
cv::Vec3d WHITE_RGB_COLOR = cv::Scalar(0, 0, 0);
cv::Vec3d STRIPED_RGB_COLOR = cv::Scalar(0, 255, 0);
cv::Vec3d SOLID_RGB_COLOR = cv::Scalar(0, 0, 255);

#endif //MINIMAPCONSTANTS_H
