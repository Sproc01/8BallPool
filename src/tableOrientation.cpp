//Author: Michela Schibuola

#include <opencv2/opencv.hpp>
#include "tableOrientation.h"
#include "minimapConstants.h"
#include "util.h"

#include "detection.h"

using namespace cv;
using namespace std;

/* edge that contains the corners, its center, a rect centered in its center and the
 * percentile of background in the rectangle*/
struct Edge{
    Point2f corner1;
    Point2f corner2;
    Point2f center;
    Rect center_rect;
    double background_percentile;
};

/**
 * @brief compare two edges by their percentile of background color.
 * Compare two edges by their percentile of background in the center pool.
 * @param e1 first edge to compare.
 * @param e2 second edge to compare.
 * @return true if e1 has less background color than e2, false otherwise.
 */
bool compareByPercentile(const Edge &e1, const Edge &e2)
{
    //TODO: can be empty?
    return e1.background_percentile < e2.background_percentile;
}

/**
 * @brief compute the percentile of white pixels in the rectangle
 * Count the number of pixels of the rect inside the image and the number of pixels which color is
 * white in the mask (which correspond to the table)
 * @param mask_img image with the table masked
 * @param rect rectangle in which compute the percentile
 * @return percentile of white pixels in the rect
 * @throw invalid_argument if the image in input is empty
 * @throw invalid_argument if the rect in input is empty
 */
double computeTablePercentile(const Mat &mask_img, const Rect &rect) {
    //TODO: check mask_img and colors
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

/**
 * @brief check if two edges are opposite to each other in a quadrangle.
 * Check if two edges are opposite to each other by using the value of their corners
 * @param e1 first edge.
 * @param e2 second edge.
 * @return true if e1 and e2 are opposite to each other, false otherwise
 */
bool oppositeEdges(const Edge &e1, const Edge &e2) {
    //TODO: can be empty?
    if((e1.corner1 == e2.corner2)||
        e1.corner2 == e2.corner1 ||
        e1.corner1 == e2.corner1 ||
        e1.corner2 == e2.corner2)
        return false;
    return true;
}

/**
 * @brief check if the table image (transformed and cropped) is horizontal
 * Compute the edges of the image and for each of them compute: the center, the rect around the center,
 * the percentile of background in the rect. Verify which of the four centers are the pools of the longest
 * table edges, using the background percentile.
 * @param table_img image of the table transformed and cropped to the minimap dimension
 * @return true if the image is horizontal, false otherwise
 * @throw invalid_argument if the image in input is empty
 */
bool checkHorizontalTable(const Mat &table_img){
	if(table_img.empty())
        throw invalid_argument("Empty image in input");

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
    //TODO: set table dimensions in const file to use it somewhere else
    const int RECT_WIDTH = (POOL_DIAMETER_CM/TABLE_LONGEST_EDGE_CM)*table_img.cols;
    const int RECT_HEIGHT = (POOL_DIAMETER_CM/TABLE_LONGEST_EDGE_CM)*table_img.cols;

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
