// Author: Michela Schibuola

#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include <opencv2/opencv.hpp>
#include <vector>
#include "ball.h"
#include "table.h"

cv::Mat computeTransformation(Table table);
std::vector<cv::Point> computeBallsPositions(std::vector<Ball> &balls, cv::Mat transform);
void drawBallsOnMap(cv::Mat &map_img, std::vector<cv::Point> balls_map, std::vector<Ball> balls);

#endif //TRANSFORMATION_H
