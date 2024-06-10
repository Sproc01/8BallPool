// Author: Michela Schibuola

#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include <opencv2/opencv.hpp>
#include <vector>
#include "ball.h"
#include "table.h"

cv::Mat minimapWithBalls(cv::Mat minimap, Table table, cv::Mat frame);

#endif //TRANSFORMATION_H
