#include "segment.h"
#include "ball.h"
#include "table.h"
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

void segmentTable(Mat &frame, const vector<Point> &tableCorners)
{
    fillConvexPoly(frame, tableCorners, Scalar(0, 255, 0));
    //polylines(frame, tableCorners, true, Scalar(0, 255, 0), 2);
}

void segmentBalls(Mat &frame, const vector<Ball> &balls)
{
    float radius;
    Point center;
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
        Rect b = ball.getBbox();
        radius = b.width / 2;
        center = Point(b.tl().x + radius, b.tl().y + radius);
        circle(frame, center, radius, c, 2);
    }
}