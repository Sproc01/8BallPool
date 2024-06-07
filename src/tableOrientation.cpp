//Author: Michela Schibuola

#include <opencv2/opencv.hpp>
#include "tableOrientation.h"

using namespace cv;
using namespace std;

struct Edge{
    Point corners[2];
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
double computePercentile(Mat &frame, Rect rect, Vec3b color_range) {
    int count = 0;
    for(int x = rect.x; x < rect.x + rect.width; x++) {
        for(int y = rect.y; y < rect.y + rect.height; y++) {
            //TODO: fix backgroud color range and how to compute it
            /*
            if(frame.at<Vec3d>(x, y) >= Scalar(color_range[0], 50, 90)
                && frame.at<Vec3d>(x, y) <= Scalar(color_range[1], 255, 255)) {
                count++;
                }
            */
        }
    }
    return count/rect.area();
}


void orderTableCornersByOrientation(Mat &frame, Vec<Point, 4> &corners, Vec3d background_color){

    //compute the centers of each table edge
    vector<Edge> edges;

    int next_corner;
    for(int i = 0; i < 4; i++) {
        next_corner = i+1;
        if(i == 3)
            next_corner = 0;
        edges[i].corners[0] = corners[i];
        edges[i].corners[1] = corners[next_corner];
    }

    for(Edge edge : edges) {
        edge.center = getCenter(edge.corners[0], edge.corners[1]);
    }

    //compute the rects around the centers shifted more to the center of the table
    const int rect_width = 10;
    const int rect_height = 10;

    for(Edge edge : edges) {
        edge.center_rect = Rect(edge.center.x, edge.center.y - rect_height/2, rect_width, rect_height);
    }

    //compute the rects with and without the pools
    //compute the percentile of rectangle with color close to the table background
    //TODO: fix backgroud_color to use directly its range
    for(Edge edge : edges) {
        edge.backgroud_percentile = computePercentile(frame, edge.center_rect, background_color);
    }

    //order the edges by the percentile of background in the rectangle around the center of the edge
    vector<Edge> ordered_edges = edges;
    sort(edges.begin(), edges.end(), compareByPercentile);

    //edges[0] and edges[1] are the longest edges
    //edges[2] and edges[3] are the shortes edges

    //pick the corner more on the left of the image
    if(edges[0].corners[0].x < edges[1].corners[0].x) {
        //edges[0].corners[0] is the corner to be put first
        corners[0] = edges[0].corners[0];
        corners[1] = edges[0].corners[1];
        for(int i = 0; i < edges.size(); i++) {
            if(edges[i].corners[0] == corners[1]) {
                corners[2] = edges[i].corners[1];
                break;
            }
        }
        for(int i = 0; i < edges.size(); i++) {
            if(edges[i].corners[0] == corners[2]) {
                corners[3] = edges[i].corners[1];
                break;
            }
        }

    }
    else {
        //edges[1].corners[0] is the corner to be put first
        corners[0] = edges[1].corners[0];
        corners[1] = edges[1].corners[1];
        for(int i = 0; i < edges.size(); i++) {
            if(edges[i].corners[0] == corners[1]) {
                corners[2] = edges[i].corners[1];
                break;
            }
        }
        for(int i = 0; i < edges.size(); i++) {
            if(edges[i].corners[0] == corners[2]) {
                corners[3] = edges[i].corners[1];
                break;
            }
        }
    }

    //TODO: check if the results are correct (not sure on the tecnique used)
}