//Author: Michela Schibuola

#include <opencv2/opencv.hpp>
#include "tableOrientation.h"
#include "minimap.h"
#include "util.h"

#include "detection.h"

using namespace cv;
using namespace std;

/*
 Edge that contains the corners, its center, a rectangle centered in its center and the percentile of background in the rectangle.
 */
struct Edge{
	Point2f corner1;
	Point2f corner2;
	Point2f center;
	Rect centerRect;
	double backgroundPercentile;
};

/**
 * @brief Compare two edges by their percentile of background color.
 * Compare two edges by their percentile of background in the center pool.
 * @param e1 first edge to compare.
 * @param e2 second edge to compare.
 * @return true if e1 has less background color than e2, false otherwise.
 */
bool compareByPercentile(const Edge &e1, const Edge &e2)
{
	return e1.backgroundPercentile < e2.backgroundPercentile;
}

/**
 * @brief Compute the percentile of white pixels in the rectangle.
 * Count the number of pixels of the rectangle inside the image and the number of pixels which color is
 * white in the mask (which corresponds to the table).
 * @param maskImg image with the table masked.
 * @param rect rectangle in which compute the percentile.
 * @return percentile of white pixels in the rect.
 * @throw invalid_argument if the image in input is empty.
 * @throw invalid_argument if the rect in input is empty.
 */
double computeTablePercentile(const Mat &maskImg, const Rect &rect) {

	if(maskImg.empty())
		throw invalid_argument("Empty image in input");

	if(rect.empty())
		throw invalid_argument("Empty rect in input");
	double count = 0;
	double countTot = 0;
	for(int x = rect.x; x < rect.x + rect.width; x++) {
		for(int y = rect.y; y < rect.y + rect.height; y++) {
			if(x >= 0 && x < maskImg.cols && y >= 0 && y < maskImg.rows) {
				if(maskImg.at<uchar>(y, x) == 255) {
					count++;
				}
				countTot++;
			}
		}
	}
	return count/countTot;
}

/**
 * @brief Check if two edges are opposite to each other in a quadrangle.
 * Check if two edges are opposite to each other by using the value of their corners.
 * @param e1 first edge.
 * @param e2 second edge.
 * @return true if e1 and e2 are opposite to each other, false otherwise
 */
bool oppositeEdges(const Edge &e1, const Edge &e2) {
	if((e1.corner1 == e2.corner2)||
		e1.corner2 == e2.corner1 ||
		e1.corner1 == e2.corner1 ||
		e1.corner2 == e2.corner2)
		return false;
	return true;
}

/**
 * @brief Check if the table image (transformed and cropped) is horizontal.
 * Compute the edges of the image and for each of them compute: the center, the rect around the center,
 * the percentile of background in the rect. Verify which of the four centers are the pools of the longest
 * table edges, using the background percentile.
 * @param tableImg image of the table transformed and cropped to the minimap dimension
 * @param corners corners of the table in the tableImg
 * @return true if the image is horizontal, false otherwise
 * @throw invalid_argument if the image in input is empty
 */
bool checkHorizontalTable(const Mat &tableImg, Vec<Point2f, 4> corners){
	if(tableImg.empty())
		throw invalid_argument("Empty image in input");

	//compute the centers of each table edge
	vector<Edge> edges(4);

	for(int i = 0; i < 4; i++) {
		int next_corner = i+1;
		if(i == 3)
			next_corner = 0;
		edges[i].corner1 = corners[i];
		edges[i].corner2 = corners[next_corner];
	}

	for(int i = 0; i < 4; i++) {
		edges[i].center = getCenter(edges[i].corner1, edges[i].corner2);
	}

	//compute the rects around the centers
	const int RECT_WIDTH = (POOL_DIAMETER_CM/TABLE_LONGEST_EDGE_CM)*tableImg.cols;
	const int RECT_HEIGHT = (POOL_DIAMETER_CM/TABLE_LONGEST_EDGE_CM)*tableImg.cols;

	for(int i = 0; i < 4; i++) {
		edges[i].centerRect = Rect(edges[i].center.x - RECT_WIDTH/2.0, edges[i].center.y - RECT_HEIGHT/2.0, RECT_WIDTH, RECT_HEIGHT);
	}

	//print the rectangles on the pools
	Mat imgPoolsRectangles = tableImg.clone();
	for(int i = 0; i < 4; i++) {
		rectangle(imgPoolsRectangles, edges[i].centerRect, Scalar(0, 0, 255), 1, LINE_AA);
	}
	//imshow("Rectangles on pools", imgPoolsRectangles);

	// mask the image
	Mat maskImg;
	Mat frameHSV;
	Vec2b backgroundColor = mostFrequentHueColor(tableImg);
	cvtColor(tableImg, frameHSV, COLOR_BGR2HSV);
	inRange(frameHSV, Scalar(backgroundColor[0], 50, 90),
				Scalar(backgroundColor[1], 255, 255), maskImg);
	//imshow("Mask img", maskImg);
	//waitKey(0);

	//print the rectangles on the pool of the masked img (just for testing)
	/*
	Mat maskImgRectangles = maskImg.clone();
	for(int i = 0; i < 4; i++) {
		rectangle(maskImgRectangles, edges[i].centerRect, Scalar(0, 0, 255), 1, LINE_AA);
	}
	//imshow("Rectangles on pools (mask)", maskImgRectangles);
	*/

	//compute the rects with and without the pools
	//compute the percentile of rectangle with color close to the table background
	for(int i = 0; i < 4; i++) {
		edges[i].backgroundPercentile = computeTablePercentile(maskImg, edges[i].centerRect);
	}

	//order the edges by the percentile of background in the rectangle around the center of the edge
	vector<Edge> orderedEdges;
	copy(edges.begin(), edges.end(), back_inserter(orderedEdges));
	sort(orderedEdges.begin(), orderedEdges.end(), compareByPercentile);

	if(oppositeEdges(orderedEdges[0], orderedEdges[1])) {
		//the ones with "more pool" are opposite edges -> they are the longest edges
		if(orderedEdges[0].center == edges[0].center || orderedEdges[1].center == edges[0].center)
			return true;
		else
			return false;
	}
	else if (oppositeEdges(orderedEdges[0], orderedEdges[3])) {
		//the one with "more pool" is opposite to the one with "less pool" --> they are not the longest edges
		if(orderedEdges[0].center == edges[0].center || orderedEdges[3].center == edges[0].center)
			return false;
		else
			return true;
	}
	else {
		//there is uncertainty, probably the one with "more pool" is the longest edge
		if(orderedEdges[0].center == edges[0].center)
			return true;
		else
			return false;
	}
}
