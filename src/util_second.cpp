// Author: Michela Schibuola

#include <opencv2/opencv.hpp>
#include "util.h"
#include "constants.h"

using namespace std;
using namespace cv;

/**
 * @brief Compute the center between two points.
 * @param p1 first point.
 * @param p2 second point.
 * @return the point which is the center of p1 and p2.
 */
Point2f getCenter(const Point2f &p1, const Point2f &p2) {
	int dxHalf = abs(p1.x - p2.x)/2;
	int dyHalf = abs(p1.y - p2.y)/2;
	if(p1.x > p2.x)
		dxHalf *= -1;
	if(p1.y > p2.y)
		dyHalf *= -1;
	return Point2f(p1.x + dxHalf, p1.y + dyHalf);
}

/**
 * @brief Compute the color of a specific category.
 * @param category element of type Category.
 * @return the color associated to the category in input.
 */
Vec3b getColorFromCategory(const Category &category) {
	switch (category) {
		case BLACK_BALL: return BLACK_BGR_COLOR; break;
		case WHITE_BALL: return WHITE_BGR_COLOR; break;
		case STRIPED_BALL: return STRIPED_BGR_COLOR; break;
		case SOLID_BALL: return SOLID_BGR_COLOR; break;
		case BACKGROUND: return BACKGROUND_BGR_COLOR; break;
		case PLAYING_FIELD: return PLAYING_FIELD_BGR_COLOR; break;
		default:
			throw invalid_argument("Not correct category");
		break;
	}
}

/**
 * @brief Rotate the corners of the table clockwise.
 * In each position i, the corner in position i+1 is stored. And in the last position the first corner is stored.
 * In this way all the corners are rotated clockwise.
 * @param corners vector of four corners.
 */
void rotateCornersClockwise(Vec<Point2f, 4> &corners) {
	Vec<Point2f, 4> imgVerticesTemp = corners;
	for(int i = 0; i < 4; i++) {
		if(i+1 < 4) {
			corners[i].x = imgVerticesTemp[i+1].x;
			corners[i].y = imgVerticesTemp[i+1].y;
		}
		else {
			corners[i].x = imgVerticesTemp[0].x;
			corners[i].y = imgVerticesTemp[0].y;
		}
	}
}

/**
 * @brief Compute an interval min/max for the radius with respect to the distance and the perspective of the table
 * To compute the interval, first a mean value is computed by using a proportion between the diagonal of the table in
 * pixel and the dimensions of the diagonal of the table and the balls in centimeters. Then, a percentage of slope
 * between the camera direction and the table is computed, by using one of the angle that the detected table creates;
 * this angle is compared with the PI/2 angle, and a value between 0 and 1 is computed. If the value is 1, then the
 * camera is parallel to the table, if it is 0, then the camera is perpendicular to it. For computing the final interval, the
 * minimum and the maximum value are computed by subtracting and incrementing a value, which increases with the percentage
 * of slope (more the slope, more the variance), and a precision value is added due to some other variables in the images.
 * @param minRadius parameter to store the minimum radius.
 * @param maxRadius parameter to store the maximum radius.
 * @param imgCorners corners of the table in the frame
 */
void radiusInterval(float &minRadius, float &maxRadius, const Vec<Point2f, 4>  &imgCorners) {
	//compute longer diagonal
	float diag1Px = norm(imgCorners[0] - imgCorners[2]);
	float diag2Px = norm(imgCorners[1] - imgCorners[3]);
	float longDiagPx;
	if (diag1Px > diag2Px)
		longDiagPx = diag1Px;
	else
		longDiagPx = diag2Px;

	//compute mean radius
	float meanRadius = (BALL_RADIUS_CM/TABLE_DIAMETER_CM) * longDiagPx;

	//compute smaller angle (two angles will be <= 90°, the other two will be 180°-minAngle=
	float angle1 = atan2(imgCorners[0].y - imgCorners[1].y, imgCorners[0].x - imgCorners[1].x);
	angle1 = angle1 >= 0 ? angle1 : abs(angle1) - CV_PI;
	float angle2 = atan2(imgCorners[1].y - imgCorners[2].y, imgCorners[1].x - imgCorners[2].x);
	angle2 = angle2 >= 0 ? angle2 : abs(abs(angle2) - CV_PI);
	float minAngle = abs(angle1 - angle2);
	if(minAngle > (CV_PI/2))
		minAngle = CV_PI - minAngle;

	//compute the percentage of slope
	float percentageSlope = 1 - (minAngle/(CV_PI/2));
	//percentageSlope = 1 -> (angle of 180°) the camera is parallel to the table (ideally radius from 0 to infinite)
	//percentageSlope = 0.5 -> the camera is 45° to the table (radius can change)
	//percentageSlope = 0 -> (angles of 90°) the camera is perpendicular to the table (ideally radius always meanRadius)

	//compute the minimum and maximum radius
	const float PRECISION = 1;
	minRadius = meanRadius - (meanRadius*percentageSlope) - PRECISION;
	maxRadius = meanRadius + (meanRadius*percentageSlope) + PRECISION;

	if(abs(minRadius - maxRadius) < 3) {
		minRadius -= 2;
		maxRadius += 2;
	}

	//cout << "RADIUS: min: " << minRadius << ", medium: " << meanRadius << ", max: " << maxRadius << endl;
}
