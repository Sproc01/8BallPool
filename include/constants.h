//Author: Michela Schibuola

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>
#include <opencv2/opencv.hpp>

//path of the minimap
// const std::string MINIMAP_PATH = "../img/minimap.png";

//corners of the table in the minimap
const cv::Point2f TOP_LEFT_MAP_CORNER = cv::Point2f(65, 63);
const cv::Point2f TOP_RIGHT_MAP_CORNER = cv::Point2f(901, 63);
const cv::Point2f BOTTOM_RIGHT_MAP_CORNER = cv::Point2f(901, 496);
const cv::Point2f BOTTOM_LEFT_MAP_CORNER = cv::Point2f(65, 496);
const cv::Vec<cv::Point2f, 4> MAP_CORNERS = {TOP_LEFT_MAP_CORNER, TOP_RIGHT_MAP_CORNER, BOTTOM_RIGHT_MAP_CORNER, BOTTOM_LEFT_MAP_CORNER};

//information about the minimap image
const int MINIMAP_IMG_WIDTH = 967;
const int MINIMAP_IMG_HEIGHT = 560;

//radius of the balls on the minimap
const double MAP_BALL_RADIUS = 12.6;

//colors for the balls in the minimap
const cv::Vec3b BACKGROUND_BGR_COLOR = cv::Vec3b(0, 0, 0);      // black
const cv::Vec3b WHITE_BGR_COLOR = cv::Vec3b(255, 255, 255);		// white
const cv::Vec3b BLACK_BGR_COLOR = cv::Vec3b(100, 100, 100);     // gray
const cv::Vec3b SOLID_BGR_COLOR = cv::Vec3b(0, 0, 255);         // red
const cv::Vec3b STRIPED_BGR_COLOR = cv::Vec3b(255, 0, 0);       // blue
const cv::Vec3b PLAYING_FIELD_BGR_COLOR = cv::Vec3b(0, 255, 0); // green


//some real table dimensions (approximates)
const float POCKET_DIAMETER_CM = 15;
const float TABLE_LONGEST_EDGE_CM = 250;
const float BALL_RADIUS_CM = 2.86;
const float TABLE_DIAMETER_CM = 215;

// S>70, V>100 to be a color and not black or white
const int S_CHANNEL_COLOR_THRESHOLD = 70;
const int V_CHANNEL_COLOR_THRESHOLD = 100;

#endif //CONSTANTS_H
