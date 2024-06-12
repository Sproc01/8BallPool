//Author:
#include <opencv2/opencv.hpp>
using namespace cv;

Point2f getCenter(Point2f p1, Point2f p2) {
    Point2f center;
    int dx_half = abs(p1.x - p2.x)/2;
    int dy_half = abs(p1.y - p2.y)/2;
    if(p1.x > p2.x)
        dx_half *= -1;
    if(p1.y > p2.y)
        dy_half *= -1;
    center = Point2f(p1.x + dx_half, p1.y + dy_half);
    return center;
}
