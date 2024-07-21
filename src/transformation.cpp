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
	Mat img_transformed;
	warpPerspective(img, img_transformed, transform,
		Size(MINIMAP_IMG_WIDTH, MINIMAP_IMG_HEIGHT));

	//img transformed cropped
	Mat img_transformed_cropped = img_transformed.rowRange(map_corners[0].y, map_corners[3].y)
											.colRange(map_corners[0].x, map_corners[1].x);

	return img_transformed_cropped;
}

/**
 * @brief Compute the transformation matrix.
 * Compute the transformation matrix using the corners of the table in the original image and the corners of the table
 * in the minimap image and with that compute the corrispondent perspective transform. After the first computation,
 * the correctness of the table orientation is checked, if it is not correct, the transformation is recomputed.
 * @param img original image used to check the orientation of the table.
 * @param img_corners corners of the table in the original image.
 * @return transformation matrix.
 * @throw invalid_argument if the image in input is empty
 */
Mat computeTransformation(const Mat& img, Vec<Point2f, 4>  &img_corners) {
	if(img.empty())
		throw invalid_argument("Empty image in input");

	//compute perspective transform
	Mat transform = getPerspectiveTransform(img_corners, map_corners);

	//apply transformation considering corners such as top-left is the first one, followed by a long table side
	Mat table_segmented_transformed = imgTransformedCropped(img, transform);
	//imshow("Img transformed cropped", table_segmented_transformed);
	//waitKey(0);

	Vec<Point2f, 4> table_segmented_transformed_corners =  {Point2f(0, 0),
								Point2f(table_segmented_transformed.cols, 0),
								Point2f(table_segmented_transformed.cols, table_segmented_transformed.rows),
								Point2f(0, table_segmented_transformed.rows)};

	//check if the transformation produces the table oriented correctly (in horizontal direction)
	if(!checkHorizontalTable(table_segmented_transformed, table_segmented_transformed_corners)) {
		//the table is not correctly rotated

		//rotate the corners correctly
		rotateCornersClockwise(img_corners);

		//compute perspective transform with corners correctly ordered
		transform = getPerspectiveTransform(img_corners, map_corners);
		//imgTransformed = imgTransformedCropped(img, transform, map_corners);
		//imshow("Img transformed cropped", imgTransformed);
	}
	return transform;
}

/**
 * @brief Draw the balls and their tracking on the minimap.
 * First compute the current and previous positions of the balls using the transformation matrix.
 * Draw the tracking lines in the image that will be reused in the next frames. Use a copy of the
 * previous image to draw the balls with their correct colors.
 * @param minimap_with_track minimap image in which the tracking lines are kept.
 * @param transform transformation matrix.
 * @param balls vector of balls containing their positions in the original image.
 * @return minimap image with tracking lines and balls.
 * @throw invalid_argument if the image in input is empty
 * @throw invalid_argument if the transformation matrix in input is empty
 * @throw invalid_argument if the balls pointer is a null pointer
 */
Mat drawMinimap(Mat &minimap_with_track, const Mat &transform, Ptr<vector<Ball>> balls) {
	if(minimap_with_track.empty())
		throw invalid_argument("Empty image in input");

	if(transform.empty())
		throw invalid_argument("Empty transformation matrix in input");

	if(balls == nullptr)
		throw invalid_argument("Null pointer");

	if(balls->empty())
		return minimap_with_track;

	//compute balls and prec balls positions in the image
	vector<Point2f> img_balls_pos (balls->size());
	vector<Point2f> img_prec_balls_pos (balls->size());
	vector<Vec3b> ball_colors (balls->size());
	for(int i = 0; i < balls->size(); i++) {
		img_balls_pos[i] = (balls->at(i)).getBBoxCenter();
		ball_colors[i] = getColorFromCategory((balls->at(i)).getCategory());
		img_prec_balls_pos[i] = (balls->at(i)).getBboxCenter_prec();
	}

	//compute balls and prec balls positions in the map
	vector<Point2f> map_balls_pos;
	perspectiveTransform(img_balls_pos, map_balls_pos, transform);
	vector<Point2f> map_prec_balls_pos;
	perspectiveTransform(img_prec_balls_pos, map_prec_balls_pos, transform);

	//draw tracking lines
	for(int i = 0; i < balls->size(); i++) {
		//check if a precedent ball exists, otherwise do not draw a line
		if(img_prec_balls_pos[i].x != -1 && img_prec_balls_pos[i].y != -1 && (balls->at(i)).getVisibility()) {
			if(pointPolygonTest	(map_corners, map_balls_pos[i], false) >= 0
				&& pointPolygonTest	(map_corners, map_prec_balls_pos[i], false) >= 0) {
				line(minimap_with_track, map_prec_balls_pos[i], map_balls_pos[i], Vec3d(0, 0, 0), 2);
			}
			else {
				(balls->at(i)).setVisibility(false);
			}
		}
	}

	//draw balls in the returned minimap
	Mat minimap_with_balls = minimap_with_track.clone();
	for(int i = 0; i < balls->size(); i++) {
		if((balls->at(i)).getVisibility()) {
			if(pointPolygonTest	(map_corners, map_balls_pos[i], false) >= 0) {
				circle(minimap_with_balls, map_balls_pos[i], MAP_BALL_RADIUS, ball_colors[i], -1);
				circle(minimap_with_balls, map_balls_pos[i], MAP_BALL_RADIUS, Vec3d(0, 0, 0), 2);
			}
			else
				(balls->at(i)).setVisibility(false);
		}
	}
	return minimap_with_balls;
}
