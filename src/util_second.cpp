// Author: Michela Schibuola

#include <opencv2/opencv.hpp>
#include "util.h"
#include "minimapConstants.h"

using namespace std;
using namespace cv;

/**
 * @brief compute the center between two points.
 * @param p1 first point.
 * @param p2 second point.
 * @return the point which is the center of p1 and p2.
 */
Point2f getCenter(const Point2f &p1, const Point2f &p2) {
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

/**
 * @brief compute the color of a specific category.
 * @param category element of type Category.
 * @return the color associated to the category in input.
 */
Vec3b getColorFromCategory(Category category) {
	switch (category) {
		case BLACK_BALL: return BLACK_BGR_COLOR; break;
		case WHITE_BALL: return WHITE_BGR_COLOR; break;
		case STRIPED_BALL: return STRIPED_BGR_COLOR; break;
		case SOLID_BALL: return SOLID_BGR_COLOR; break;
		case BACKGROUND: return BACKGROUND_BGR_COLOR; break;
		case PLAYING_FIELD: return BACKGROUND_BGR_COLOR; break;
		default:
			throw invalid_argument("Not correct category");
		break;
	}
}

/**
 * @brief rotate the corners of the table clockwise.
 * In each position i, the corner in position i+1 is stored. And in the last position the first corner is stored.
 * In this way all the corners are rotated clockwise.
 * @param corners vector of four corners.
 */
void rotateCornersClockwise(Vec<Point2f, 4> &corners) {
	Vec<Point2f, 4> img_vertices_temp = corners;
	for(int i = 0; i < 4; i++) {
		if(i+1 < 4) {
			corners[i].x = img_vertices_temp[i+1].x;
			corners[i].y = img_vertices_temp[i+1].y;
		}
		else {
			corners[i].x = img_vertices_temp[0].x;
			corners[i].y = img_vertices_temp[0].y;
		}
	}
}

/**
 * @brief compute an interval min/max for the radius with respect to the distance and the perspecdtive of the table
 * To compute the interval, first a mean value is computed by using a proportion between the diagonal of the table in
 * pixel and the dimensions of the diagonal of the table and the balls in centimeters. Then, a percentage of slope
 * between the camera direction and the table is computed, by using one of the angle that the detected table creates;
 * this angle is compared with the PI/2 angle, and a value between 0 and 1 is computed. If the value is 1, then the
 * camera is parallel to the table, if it is 0, then the camera is perpendicular to it. To compute the final interval, the
 * minimum and the maximum value are computed by subtracting and incrementing a value, which increases with the percentage
 * of slope (more the slope, more the variance), and a precision value is added due to some other variables in the images.
 * @param min_radius parameter that will store the minimum radius.
 * @param max_radius parameter that will store the maximum radius.
 * @param img_corners corners of the table in the frame
 */
void radiusInterval(float &min_radius, float &max_radius, const Vec<Point2f, 4>  &img_corners) {
	//compute longer diagonal
	float diag1_px = norm(img_corners[0] - img_corners[2]);
	float diag2_px = norm(img_corners[1] - img_corners[3]);
	float long_diag_px;
	if (diag1_px > diag2_px)
		long_diag_px = diag1_px;
	else
		long_diag_px = diag2_px;

	//compute mean radius
	float mean_radius = (BALL_RADIUS_CM/TABLE_DIAMETER_CM) * long_diag_px;

	//compute smaller angle (two angles will be <= 90°, the other two will be 180°-min_angle=
	float angle1 = atan2(img_corners[0].y - img_corners[1].y, img_corners[0].x - img_corners[1].x);
	angle1 = angle1 >= 0 ? angle1 : abs(angle1) - CV_PI;
	float angle2 = atan2(img_corners[1].y - img_corners[2].y, img_corners[1].x - img_corners[2].x);
	angle2 = angle2 >= 0 ? angle2 : abs(abs(angle2) - CV_PI);
	float min_angle = abs(angle1 - angle2);
	if(min_angle > (CV_PI/2))
		min_angle = CV_PI - min_angle;

	//compute the percentage of slope
	float percentage_slope = 1 - (min_angle/(CV_PI/2));
	//percentage_slope = 1 -> (angle of 180°) the camera is parallel to the table (ideally radius from 0 to infinite)
	//percentage_slope = 0.5 -> the camera is 45° to the table (radius can change)
	//percentage_slope = 0 -> (angles of 90°) the camera is perpendicular to the table (ideally radius always mean_radius)

	//compute the minimum and maximum radius
	const float PRECISION = 1;
	min_radius = mean_radius - (mean_radius*percentage_slope) - PRECISION;
	max_radius = mean_radius + (mean_radius*percentage_slope) + PRECISION;

	if(abs(min_radius - max_radius) < 3) {
		min_radius -= 2;
		max_radius += 2;
	}

	//cout << "RADIUS: min: " << min_radius << ", medium: " << mean_radius << ", max: " << max_radius << endl;
}
