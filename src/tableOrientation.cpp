//Author: Michela Schibuola

#include <opencv2/opencv.hpp>
#include "tableOrientation.h"
#include "util.h"

#include "detection.h"

using namespace cv;
using namespace std;

struct Edge{
    Point2f corner1;
    Point2f corner2;
    Point2f center;
    Rect center_rect;
    double background_percentile;
};

//compare two edges by their percentile of background color
bool compareByPercentile(const Edge &e1, const Edge &e2)
{
    //TODO: can be empty?
    return e1.background_percentile < e2.background_percentile;
}

//Return the percentile of pixels in the color_range within the rectangle in the image
double computeTablePercentile(const Mat &mask_img, const Rect &rect) {
    if(mask_img.empty())
        throw invalid_argument("Empty image in input");

    if(rect.empty())
        throw invalid_argument("Empty rect in input");

    double count = 0;
    double count_tot = 0;
    for(int x = rect.x; x < rect.x + rect.width; x++) {
        for(int y = rect.y; y < rect.y + rect.height; y++) {
            if(x >= 0 && x < mask_img.cols && y >= 0 && y < mask_img.rows) {
                if(mask_img.at<uchar>(y, x) == 255) {
                    count++;
                }
                count_tot++;
            }
        }
    }
    return count/count_tot;
}

bool oppositeEdges(const Edge &e1, const Edge &e2) {
    //TODO: can be empty?
    if((e1.corner1 == e2.corner2)||
        e1.corner2 == e2.corner1 ||
        e1.corner1 == e2.corner1 ||
        e1.corner2 == e2.corner2)
        return false;
    return true;
}

bool checkHorizontalTable(const Mat &table_img){
    if(table_img.empty())
        throw invalid_argument("Empty image in input");

    //TODO: corners as argument?
    Vec<Point2f, 4> corners =  {Point2f(0, 0),
                                Point2f(table_img.cols, 0),
                                Point2f(table_img.cols, table_img.rows),
                                Point2f(0, table_img.rows)};

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

    //compute the rects around the centers
    //loghest edge table = 250 cm (about)
    //pool diameter = 15 cm (about)
    //pool/edge = 15/250 (about)
    //TODO: set table dimensions in const file to use it somewhere else
    const int RECT_WIDTH = (15.0/250.0)*table_img.cols;
    const int RECT_HEIGHT = (15.0/250.0)*table_img.cols;

    for(int i = 0; i < 4; i++) {
        edges[i].center_rect = Rect(edges[i].center.x - RECT_WIDTH/2, edges[i].center.y - RECT_HEIGHT/2, RECT_WIDTH, RECT_HEIGHT);
    }

    //print the rectangles on the pools
    Mat img_pools_rectangles = table_img.clone();
    for(int i = 0; i < 4; i++) {
        rectangle(img_pools_rectangles, edges[i].center_rect, Scalar(0, 0, 255), 1, LINE_AA);
    }
    //imshow("Rectangles on pools", img_pools_rectangles);

    // mask the image
    Mat mask_img;
    Mat frameHSV;
    Vec2b background_color = mostFrequentHueColor(table_img);
    cvtColor(table_img, frameHSV, COLOR_BGR2HSV);
    inRange(frameHSV, Scalar(background_color[0], 50, 90),
                Scalar(background_color[1], 255, 255), mask_img);
    //imshow("Mask img", mask_img);

	//print the rectangles on the pool of the masked img (just for testing)
    /*
    Mat mask_img_rectangles = mask_img.clone();
    for(int i = 0; i < 4; i++) {
        rectangle(mask_img_rectangles, edges[i].center_rect, Scalar(0, 0, 255), 1, LINE_AA);
    }
    //imshow("Rectangles on pools (mask)", mask_img_rectangles);
    */

    //compute the rects with and without the pools
    //compute the percentile of rectangle with color close to the table background
    for(int i = 0; i < 4; i++) {
        edges[i].background_percentile = computeTablePercentile(mask_img, edges[i].center_rect);
    }

    //order the edges by the percentile of background in the rectangle around the center of the edge
    vector<Edge> ordered_edges;
    copy(edges.begin(), edges.end(), back_inserter(ordered_edges));
    sort(ordered_edges.begin(), ordered_edges.end(), compareByPercentile);

    if(oppositeEdges(ordered_edges[0], ordered_edges[1])) {
        //the one with "more pool" are opposite edges -> they are the longest edges
        if(ordered_edges[0].center == edges[0].center || ordered_edges[1].center == edges[0].center)
            return true;
        else
            return false;
    }
    else if (oppositeEdges(ordered_edges[0], ordered_edges[3])) {
        //the one with "more pool" is opposite to the one with "less pool" --> they are not the longest edges
        if(ordered_edges[0].center == edges[0].center || ordered_edges[3].center == edges[0].center)
            return false;
        else
            return true;
    }
    else {
        //there is uncertanty, probably the one with "more pool" is one longest edge
        if(ordered_edges[0].center == edges[0].center)
            return true;
        else
            return false;
    }
}
