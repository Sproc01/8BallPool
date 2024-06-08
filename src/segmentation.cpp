#include "segmentation.h"
#include "ball.h"
#include "table.h"
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

void segmentTable(const Mat &frame, const vector<Point> &tableCorners, Mat& segmented)
{
    segmented = frame.clone();
    fillConvexPoly(segmented, tableCorners, Scalar(0, 255, 0));
    //imshow("segmented", frame);
}

void segmentBalls(const Mat &frame, const vector<Ball> &balls, Mat& segmented)
{
    float radius;
    Point center;
    segmented = frame.clone();
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
        circle(frame, center, radius, c, -1);

    }
}
