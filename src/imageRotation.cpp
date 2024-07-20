// Author: Alberto Pasqualetto

#include "imageRotation.h"

#include "table.h"
#include "util.h"

using namespace std;
using namespace cv;

void calculateInscriptionParameters(const Mat &img, int targetWidth, int targetHeight, bool &toRotate, bool &toResize, short &leftBorderLength, short &rightBorderLength) {
	// TODO see if it is possible to avoid cloning the image so many times
	Mat test = img.clone();

	// skip if already has the target aspect
	toRotate = false;
	toResize = false;
	leftBorderLength = 0;
	rightBorderLength = 0;

	if (!isHorizontal(test)) {	// rotate if vertical
		toRotate = true;

		test = img.clone();
		doInscript(test, targetHeight, targetWidth, toRotate, toResize, leftBorderLength, rightBorderLength);
	}

	if (test.cols != targetWidth && test.rows != targetHeight) {	// resize if not inscribed
		toResize = true;

		test = img.clone();
		doInscript(test, targetWidth, targetHeight, toRotate, toResize, leftBorderLength, rightBorderLength);
	}

	// parameters to center the image
	test = img.clone();
	doInscript(test, targetWidth, targetHeight, toRotate, toResize, leftBorderLength, rightBorderLength);
	leftBorderLength = (targetWidth - test.cols) / 2;
	rightBorderLength = targetWidth - test.cols - leftBorderLength;	// if the difference is odd, the right border will be one pixel longer than the left one
}

void doInscript(Mat &img, int targetWidth, int targetHeight, const bool &toRotate, const bool &toResize, const short &leftBorderLength, const short &rightBorderLength) {
	if (toRotate) {
		rotate(img, img, ROTATE_90_CLOCKWISE);
	}

	if (toResize) {
		resize(img, img, Size(round(targetHeight/targetWidth * img.cols), targetHeight));
	}

	if (leftBorderLength > 0 || rightBorderLength > 0) {
		copyMakeBorder(img, img, 0, 0, leftBorderLength, rightBorderLength, BORDER_CONSTANT, Scalar(0, 0, 0));	// center the image
	}
}

void undoInscript(Mat &img, int originalWidth, int originalHeight, const bool &toRotate, const bool &toResize, const short &leftBorderLength, const short &rightBorderLength) {
	if (leftBorderLength > 0 || rightBorderLength > 0) {
		img = img(Rect(leftBorderLength, 0, originalWidth, originalHeight));
	}

	if (toResize) {
		resize(img, img, Size(originalWidth, originalHeight));
	}

	if (toRotate) {
		rotate(img, img, ROTATE_90_COUNTERCLOCKWISE);
	}
}

/**
 * @brief Find the point in the 90° clockwise rotated image corresponding to the input point.
 * @param point point with still to be rotated image coordinates.
 * @param destImg destination image (used to get its size).	TODO
 * @return rotated Point2f.
 */
Point2f rotatePoint(Point2f point, int targetWidth, int targetHeight) {
	float old_row = point.y;
	point.y = point.x;
	point.x = targetWidth - old_row;

	return point;
}

/**
 * @brief Find the point in the 90° counter-clockwise rotated image corresponding to the input point.
 * @param point point with still to be "unrotated" image coordinates.
 * @param destImg destination image (used to get its size).	TODO
 * @return "unrotated" Point2f.
 */
Point2f unrotatePoint(Point2f point, int targetWidth, int targetHeight) {
	float old_row = point.y;
	point.y = targetHeight - point.x;
	point.x = old_row;

	return point;
}

/**
 * @brief Find the same rect in the 90° clockwise rotated image corresponding to the input point.
 * @param rect Rect with still to be rotated image coordinates.
 * @param destImg destination image (used to get its size).	TODO
 * @return rotated Rect.
 */
Rect rotateRect(Rect rect, int targetWidth, int targetHeight) {
	Point2f oldTR(rect.tl().x + rect.width, rect.tl().y);
	Point2f oldBL(rect.tl().x, rect.tl().y + rect.height);

	return Rect(rotatePoint(oldTR, targetWidth, targetHeight), rotatePoint(oldBL, targetWidth, targetHeight));
}

/**
 * @brief Find the same rect in the 90° counter-clockwise rotated image corresponding to the input point.
 * @param rect Rect with still to be unrotated image coordinates.
 * @param destImg destination image (used to get its size).	TODO
 * @return "unrotated" Rect.
 */
Rect unrotateRect(Rect rect, int targetWidth, int targetHeight) {
	Point2f oldTR(rect.tl().x + rect.width, rect.tl().y);
	Point2f oldBL(rect.tl().x, rect.tl().y + rect.height);

	return Rect(unrotatePoint(oldTR, targetWidth, targetHeight), unrotatePoint(oldBL, targetWidth, targetHeight));
}

void rotateTable(Table &table, int targetWidth, int targetHeight, bool changeBboxPrec /* = false */) {
	Vec<Point2f, 4> boundaries = table.getBoundaries();
	for (size_t i = 0; i < 4; ++i) {
		boundaries[i] = rotatePoint(boundaries[i], targetWidth, targetHeight);
	}
	table.setBoundaries(boundaries);

	// transform matrix is not being rotated

	Ptr<vector<Ball>> balls = table.ballsPtr();
	for (Ball &ball : *balls) {
		ball.setBbox(rotateRect(ball.getBbox(), targetWidth, targetHeight));
		if (changeBboxPrec) {
			ball.setBbox_prec(rotateRect(ball.getBbox_prec(), targetWidth, targetHeight));
		}
	}
}

void unrotateTable(Table &table, int targetWidth, int targetHeight, bool changeBboxPrec /* = false */) {
	Vec<Point2f, 4> boundaries = table.getBoundaries();
	for (size_t i = 0; i < 4; ++i) {
		boundaries[i] = unrotatePoint(boundaries[i], targetWidth, targetHeight);
	}
	table.setBoundaries(boundaries);

	// transform matrix is not being (un)rotated

	Ptr<vector<Ball>> balls = table.ballsPtr();
	for (Ball &ball : *balls) {
		ball.setBbox(unrotateRect(ball.getBbox(), targetWidth, targetHeight));
		if (changeBboxPrec) {
			ball.setBbox_prec(unrotateRect(ball.getBbox_prec(), targetWidth, targetHeight));
		}
	}
}
