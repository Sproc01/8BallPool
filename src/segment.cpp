#include "segment.h"
#include "ball.h"
#include "table.h"
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

void segmentTable(const Mat &frame, const vector<Point> &tableCorners)
{
    polylines(frame, tableCorners, true, Scalar(0, 255, 0), 2);
}

void segmentBalls(const Mat &frame, const vector<Ball> &balls)
{
    Scalar c = Scalar(0, 0, 0);
    for (const Ball &ball : balls)
    {
        if(ball.getCategory() == Category::BLACK_BALL)
            c = Scalar(0, 0, 0);
        else if(ball.getCategory() == Category::WHITE_BALL)
            c = Scalar(255, 255, 255);
        else if(ball.getCategory() == Category::SOLID_BALL)
            c = Scalar(0, 0, 255);
        else if(ball.getCategory() == Category::STRIPED_BALL)
            c = Scalar(255, 0, 0);
        //circle(frame, ball.getCenter(), ball.getRadius(), c, 2);
    }
}