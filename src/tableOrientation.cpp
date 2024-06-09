//Author: Michela Schibuola

#include <opencv2/opencv.hpp>
#include "tableOrientation.h"

using namespace cv;
using namespace std;

struct Edge{
    Point corner1;
    Point corner2;
    Point center;
    Rect center_rect;
    double backgroud_percentile;
};

//compare two edges by their percentile of background color
bool compareByPercentile(const Edge &e1, const Edge &e2)
{
    return e1.backgroud_percentile < e2.backgroud_percentile;
}

//TODO: it is duplicated, also in transformation
// Return the center point between two points
Point getCenter(Point p1, Point p2) {
    Point center;
    int dx_half = abs(p1.x - p2.x)/2;
    int dy_half = abs(p1.y - p2.y)/2;
    if(p1.x > p2.x)
        dx_half *= -1;
    if(p1.y > p2.y)
        dy_half *= -1;
    center = Point(p1.x + dx_half, p1.y + dy_half);
    return center;
}

//Return the percentile of pixels in the color_range within the rectangle in the image
double computeTablePercentile(Mat &mask_img, Rect rect) {
    double count = 0;
    for(int x = rect.x; x < rect.x + rect.width; x++) {
        for(int y = rect.y; y < rect.y + rect.height; y++) {
            if(mask_img.at<uchar>(y, x) == 255) {
                count++;
                }
        }
    }
    return count/rect.area();
}

//TODO: must be called after transformation otherwise the centers are not correct
void orderTableCornersByOrientation(Mat frame, Vec<Point, 4> &corners, Vec2b background_color){

    //compute the centers of each table edge
    vector<Edge> edges(4);

    int next_corner;
    for(int i = 0; i < 4; i++) {
        next_corner = i+1;
        if(i == 3)
            next_corner = 0;
        edges[i].corner1 = corners[i];
        edges[i].corner2 = corners[next_corner];
    }

    for(int i = 0; i < 4; i++) {
        edges[i].center = getCenter(edges[i].corner1, edges[i].corner2);
    }

    //compute the rects around the centers shifted more to the center of the table
    //TODO: rect width and height fixed?
    const int rect_width = 40;
    const int rect_height = 40;

    for(int i = 0; i < 4; i++) {
        edges[i].center_rect = Rect(edges[i].center.x - rect_width/2, edges[i].center.y - rect_height/2, rect_width, rect_height);
    }

    //print the rectangles on the pools
    Mat img_pools_rectangles = frame.clone();
    for(int i = 0; i < 4; i++) {
        rectangle(img_pools_rectangles, edges[i].center_rect, Scalar(0, 0, 255), 1, LINE_AA);
    }
    //imshow("Rectangles on pools", img_pools_rectangles);

    //TODO: apply perspective transformation to apply centers

    // mask the image
    Mat mask_img;
    Mat frameHSV;
    cvtColor(frame, frameHSV, COLOR_BGR2HSV);
    inRange(frameHSV, Scalar(background_color[0], 50, 90),
                Scalar(background_color[1], 255, 255), mask_img);
    //imshow("Mask img", mask_img);

    //compute the rects with and without the pools
    //compute the percentile of rectangle with color close to the table background
    for(int i = 0; i < 4; i++) {
        edges[i].backgroud_percentile = computeTablePercentile(mask_img, edges[i].center_rect);
    }

    //order the edges by the percentile of background in the rectangle around the center of the edge
    vector<Edge> ordered_edges = edges;
    sort(ordered_edges.begin(), ordered_edges.end(), compareByPercentile);

    //edges[0] and edges[1] are the longest edges
    //edges[2] and edges[3] are the shortest edges

    //pick the corner more on the left of the image
    if(edges[0].corner1.x < edges[1].corner1.x) {
        //edges[0].corners[0] is the corner to be put first
        corners[0] = edges[0].corner1;
        corners[1] = edges[0].corner2;
        for(int i = 0; i < edges.size(); i++) {
            if(edges[i].corner1 == corners[1]) {
                corners[2] = edges[i].corner2;
                break;
            }
        }
        for(int i = 0; i < edges.size(); i++) {
            if(edges[i].corner1 == corners[2]) {
                corners[3] = edges[i].corner2;
                break;
            }
        }

    }
    else {
        //edges[1].corners[0] is the corner to be put first
        corners[0] = edges[1].corner1;
        corners[1] = edges[1].corner2;
        for(int i = 0; i < edges.size(); i++) {
            if(edges[i].corner1 == corners[1]) {
                corners[2] = edges[i].corner2;
                break;
            }
        }
        for(int i = 0; i < edges.size(); i++) {
            if(edges[i].corner1 == corners[2]) {
                corners[3] = edges[i].corner2;
                break;
            }
        }
    }

    Mat img_ordered_corners = frame.clone();
    //blue, green, red, other
    vector<Scalar> colors = {Scalar(255, 0, 0), Scalar(0, 255, 0), Scalar(0, 0, 255), Scalar(255, 255, 0)};
    for(size_t i = 0; i < 4; i++)
    {
        circle(img_ordered_corners, corners[i], 10, colors[i], -1);
    }
    imshow("Ordered corners", img_ordered_corners);
    //waitKey(0);
}