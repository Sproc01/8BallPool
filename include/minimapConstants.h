//Author: Michela Schibuola

#ifndef MINIMAPCONSTANTS_H
#define MINIMAPCONSTANTS_H

#include <string>
#include <opencv2/opencv.hpp>

// TODO add color table, check the colors of the balls
//path of the minimap
const std::string MINIMAP_PATH = "../img/minimap.png";

//corners of the table in the minimap
const cv::Point2f TOP_LEFT_MAP_CORNER = cv::Point2f(65, 63);
const cv::Point2f TOP_RIGHT_MAP_CORNER = cv::Point2f(901, 63);
const cv::Point2f BOTTOM_RIGHT_MAP_CORNER = cv::Point2f(901, 496);
const cv::Point2f BOTTOM_LEFT_MAP_CORNER = cv::Point2f(65, 496);

//radius of the balls on the minimap
const double MAP_BALL_RADIUS = 12.6;

//colors for the balls in the minimap
const cv::Vec3d BLACK_BGR_COLOR = cv::Vec3d(0, 0, 0);       // black
const cv::Vec3d WHITE_BGR_COLOR = cv::Vec3d(255, 255, 255);     // white
const cv::Vec3d STRIPED_BGR_COLOR = cv::Vec3d(0, 255, 0);   // green
const cv::Vec3d SOLID_BGR_COLOR = cv::Vec3d(0, 0, 255);     // red

// S>70, V>100 to be a color and not black or white
const int S_CHANNEL_COLOR_THRESHOLD = 70;
const int V_CHANNEL_COLOR_THRESHOLD = 100;

#endif //MINIMAPCONSTANTS_H
