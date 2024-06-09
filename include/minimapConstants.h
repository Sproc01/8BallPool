//Author: Michela Schibuola

#ifndef MINIMAPCONSTANTS_H
#define MINIMAPCONSTANTS_H

#include <string>
#include <opencv2/opencv.hpp>

//path of the minimap
std::string MINIMAP_PATH = "../img/minimap.png";

//corners of the table in the minimap
cv::Point TOP_LEFT_MAP_CORNER = cv::Point(80, 78);
cv::Point TOP_RIGHT_MAP_CORNER = cv::Point(886, 78);
cv::Point BOTTOM_RIGHT_MAP_CORNER = cv::Point(886, 481);
cv::Point BOTTOM_LEFT_MAP_CORNER = cv::Point(80, 481);

//radius of the balls on the minimap
int MAP_BALL_RADIUS = 12;

//colors for the balls in the minimap
cv::Vec3d BLACK_BGR_COLOR = cv::Vec3d(0, 0, 0);
cv::Vec3d WHITE_BGR_COLOR = cv::Vec3d(0, 0, 0);
cv::Vec3d STRIPED_BGR_COLOR = cv::Vec3d(0, 255, 0);
cv::Vec3d SOLID_BGR_COLOR = cv::Vec3d(0, 0, 255);

#endif //MINIMAPCONSTANTS_H
