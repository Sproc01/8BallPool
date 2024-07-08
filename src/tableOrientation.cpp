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
    return e1.background_percentile < e2.background_percentile;
}

//Return the percentile of pixels in the color_range within the rectangle in the image
double computeTablePercentile(Mat &mask_img, Rect rect) {
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

bool oppositeEdges(Edge e1, Edge e2) {
    if((e1.corner1 == e2.corner2)||
        e1.corner2 == e2.corner1 ||
        e1.corner1 == e2.corner1 ||
        e1.corner2 == e2.corner2)
        return false;
    return true;
}

//TODO: must be called after transformation otherwise the centers are not correct
//TODO: see if it is needed for table oriented horizontal in the frame
bool checkHorizontalTable(Mat table_img){
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
    const int rect_width = (15.0/250.0)*table_img.cols;
    const int rect_height = (15.0/250.0)*table_img.cols;

    for(int i = 0; i < 4; i++) {
        edges[i].center_rect = Rect(edges[i].center.x - rect_width/2, edges[i].center.y - rect_height/2, rect_width, rect_height);
    }

    //print the rectangles on the pools
    Mat img_pools_rectangles = table_img.clone();
    for(int i = 0; i < 4; i++) {
        rectangle(img_pools_rectangles, edges[i].center_rect, Scalar(0, 0, 255), 1, LINE_AA);
    }
    //imshow("Rectangles on pools", img_pools_rectangles);
    //waitKey(0);

    //TODO: apply perspective transformation to apply centers

    // mask the image
    Mat mask_img;
    Mat frameHSV;
    Vec2b background_color = mostFrequentColor(table_img);
    cvtColor(table_img, frameHSV, COLOR_BGR2HSV);
    inRange(frameHSV, Scalar(background_color[0], 50, 90),
                Scalar(background_color[1], 255, 255), mask_img);
    //imshow("Mask img", mask_img);

	//print the rectangles on the pool of the masked img (just for testing)
    //TODO: remove this
    Mat mask_img_rectangles = mask_img.clone();
    for(int i = 0; i < 4; i++) {
        rectangle(mask_img_rectangles, edges[i].center_rect, Scalar(0, 0, 255), 1, LINE_AA);
    }
    //imshow("Rectangles on pools (mask)", mask_img_rectangles);
    //waitKey(0);

    //compute the rects with and without the pools
    //compute the percentile of rectangle with color close to the table background
    for(int i = 0; i < 4; i++) {
        edges[i].background_percentile = computeTablePercentile(mask_img, edges[i].center_rect);
    }

    //order the edges by the percentile of background in the rectangle around the center of the edge
    vector<Edge> ordered_edges;
    copy(edges.begin(), edges.end(), back_inserter(ordered_edges));
    sort(ordered_edges.begin(), ordered_edges.end(), compareByPercentile);

    //waitKey(0);
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

    //
    /* old method
    if(edges[0].background_percentile + edges[2].background_percentile <  edges[1].background_percentile + edges[3].background_percentile) {
        //edges[0] and edges[2] contain pools
        return true;
    }
    else {
        return false;
    }
    */
}
