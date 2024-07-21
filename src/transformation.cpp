// Author: Michela Schibuola

#include "transformation.h"
#include "minimap.h"
#include "tableOrientation.h"
#include "util.h"

using namespace cv;
using namespace std;

/**
 * @brief Compute the original image transformed and cropped to the table.
 * Apply the transformation (warpPerspective) to the image in input and crop it in the minimap
 * table coordinates.
 * @param img original image to transform.
 * @param transform transformation to apply to the image.
 * @return image with the dimension of the minimap image, transformed and cropped.
 * @throw invalid_argument if the image in input is empty
 * @throw invalid_argument if the transformation matrix in input is empty
 */
Mat imgTransformedCropped(const Mat &img, const Mat &transform) {
	if(img.empty())
		throw invalid_argument("Empty image in input");

	if(transform.empty())
		throw invalid_argument("Empty transformation matrix in input");

	//img transformed with perspective
	Mat imgTransformed;
	warpPerspective(img, imgTransformed, transform, Size(MINIMAP_IMG_WIDTH, MINIMAP_IMG_HEIGHT));

	//img transformed cropped
	Mat imgTransformedCrop = imgTransformed.rowRange(MAP_CORNERS[0].y, MAP_CORNERS[3].y)
											.colRange(MAP_CORNERS[0].x, MAP_CORNERS[1].x);

	return imgTransformedCrop;
}

/**
 * @brief Compute the transformation matrix.
 * Compute the transformation matrix using the corners of the table in the original image and the corners of the table
 * in the minimap image, and with that compute the corrisponding perspective transform. After the first computation,
 * the correctness of the table orientation is checked, if it is not correct, the transformation is recomputed with a
 * new corners order.
 * @param img original image used to check the orientation of the table.
 * @param imgCorners corners of the table in the original image.
 * @return transformation matrix.
 * @throw invalid_argument if the image in input is empty
 */
Mat computeTransformation(const Mat& img, Vec<Point2f, 4>  &imgCorners) {
	if(img.empty())
		throw invalid_argument("Empty image in input");

	//compute perspective transform
	Mat transform = getPerspectiveTransform(imgCorners, MAP_CORNERS);

	//apply transformation considering corners such as top-left is the first one, followed by a long table side
	Mat tableSegmentedTransformed = imgTransformedCropped(img, transform);
	//imshow("Img transformed cropped", tableSegmentedTransformed);
	//waitKey(0);

	Vec<Point2f, 4> tableSegmentedTransformedCorners =  {Point2f(0, 0),
								Point2f(tableSegmentedTransformed.cols, 0),
								Point2f(tableSegmentedTransformed.cols, tableSegmentedTransformed.rows),
								Point2f(0, tableSegmentedTransformed.rows)};

	//check if the transformation produces the table oriented correctly (in horizontal direction)
	if(!checkHorizontalTable(tableSegmentedTransformed, tableSegmentedTransformedCorners)) {
		//the table is not correctly rotated

		//rotate the corners correctly
		rotateCornersClockwise(imgCorners);

		//compute perspective transform with corners correctly ordered
		transform = getPerspectiveTransform(imgCorners, MAP_CORNERS);
		//imgTransformed = imgTransformedCropped(img, transform, MAP_CORNERS);
		//imshow("Img transformed cropped", imgTransformed);
	}
	return transform;
}

/**
 * @brief Draw the balls and their tracking on the minimap.
 * First compute the current and previous positions of the balls using the transformation matrix.
 * Draw the tracking lines in the image that will be reused in the next frames. Use a copy of the
 * previous image to draw the balls with their correct colors.
 * @param minimapWithTrack minimap image in which the tracking lines are kept.
 * @param transform transformation matrix.
 * @param balls vector of balls containing their positions in the original image.
 * @return minimap image with tracking lines and balls.
 * @throw invalid_argument if the image in input is empty
 * @throw invalid_argument if the transformation matrix in input is empty
 * @throw invalid_argument if the balls pointer is a null pointer
 */
Mat drawMinimap(Mat &minimapWithTrack, const Mat &transform, Ptr<vector<Ball>> balls) {
	if(minimapWithTrack.empty())
		throw invalid_argument("Empty image in input");

	if(transform.empty())
		throw invalid_argument("Empty transformation matrix in input");

	if(balls == nullptr)
		throw invalid_argument("Null pointer");

	if(balls->empty())
		return minimapWithTrack;

	//compute balls and prec balls positions in the image
	vector<Point2f> imgBallsPos (balls->size());
	vector<Point2f> imgPrecBallsPos (balls->size());
	vector<Vec3b> ballColors (balls->size());
	for(int i = 0; i < balls->size(); i++) {
		imgBallsPos[i] = (balls->at(i)).getBBoxCenter();
		ballColors[i] = getColorFromCategory((balls->at(i)).getCategory());
		imgPrecBallsPos[i] = (balls->at(i)).getBboxCenter_prec();
	}

	//compute balls and prec balls positions in the map
	vector<Point2f> mapBallsPos;
	perspectiveTransform(imgBallsPos, mapBallsPos, transform);
	vector<Point2f> mapPrecBallsPos;
	perspectiveTransform(imgPrecBallsPos, mapPrecBallsPos, transform);

	//draw tracking lines
	for(int i = 0; i < balls->size(); i++) {
		//check if a previous ball exists, otherwise do not draw a line
		if(imgPrecBallsPos[i].x != -1 && imgPrecBallsPos[i].y != -1 && (balls->at(i)).getVisibility()) {
			if(pointPolygonTest	(MAP_CORNERS, mapBallsPos[i], false) >= 0
				&& pointPolygonTest	(MAP_CORNERS, mapPrecBallsPos[i], false) >= 0) {
				line(minimapWithTrack, mapPrecBallsPos[i], mapBallsPos[i], Vec3d(0, 0, 0), 2);
			}
			else {
				(balls->at(i)).setVisibility(false);
			}
		}
	}

	//draw balls in the returned minimap
	Mat minimapWithBalls = minimapWithTrack.clone();
	for(int i = 0; i < balls->size(); i++) {
		if((balls->at(i)).getVisibility()) {
			if(pointPolygonTest	(MAP_CORNERS, mapBallsPos[i], false) >= 0) {
				circle(minimapWithBalls, mapBallsPos[i], MAP_BALL_RADIUS, ballColors[i], -1);
				circle(minimapWithBalls, mapBallsPos[i], MAP_BALL_RADIUS, Vec3d(0, 0, 0), 2);
			}
			else
				(balls->at(i)).setVisibility(false);
		}
	}
	return minimapWithBalls;
}
