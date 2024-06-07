// Author: Michela Schibuola

#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include <opencv2/opencv.hpp>
#include <vector>
#include "ball.h"
#include "table.h"

enum Orientation
{
    HORIZONTAL = 0,
    VERTICAL
};

Orientation detectTableOrientation(Table table, cv::Mat &frame);
void computeTransformation(std::vector<cv::Point> corners);

#endif //TRANSFORMATION_H
