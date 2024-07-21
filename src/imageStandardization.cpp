// Author: Alberto Pasqualetto

#include "imageStandardization.h"

#include "table.h"
#include "util.h"

using namespace std;
using namespace cv;

// TODO FIX to accept images with aspect ratio > 16/9 (borders > 0 -> black borders; borders < 0 -> additional image length in sides)
void calculateStandardizationParameters(const Mat &img, int targetWidth, int targetHeight, bool &toRotate, bool &toResize, short &leftBorderLength, short &rightBorderLength) {
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
		doInscriptImage(test, targetHeight, targetWidth, toRotate, toResize, leftBorderLength, rightBorderLength);
	}

	if (test.cols != targetWidth && test.rows != targetHeight) {	// resize if not inscribed
		toResize = true;

		test = img.clone();
		doInscriptImage(test, targetWidth, targetHeight, toRotate, toResize, leftBorderLength, rightBorderLength);
	}

	// parameters to center the image
	test = img.clone();
	doInscriptImage(test, targetWidth, targetHeight, toRotate, toResize, leftBorderLength, rightBorderLength);
	leftBorderLength = (targetWidth - test.cols) / 2;
	rightBorderLength = targetWidth - test.cols - leftBorderLength;	// if the difference is odd, the right border will be one pixel longer than the left one
}

void doInscriptImage(Mat &img, int targetWidth, int targetHeight, const bool &toRotate, const bool &toResize, const short &leftBorderLength, const short &rightBorderLength) {
	if (toRotate) {
		rotate(img, img, ROTATE_90_CLOCKWISE);
	}

	if (toResize) {
		resize(img, img, Size(round(static_cast<double>(targetHeight)/targetWidth * img.cols), targetHeight), 0, 0);
	}

	if (leftBorderLength > 0 || rightBorderLength > 0) {
		copyMakeBorder(img, img, 0, 0, leftBorderLength, rightBorderLength, BORDER_CONSTANT, Scalar(0, 0, 0));	// center the image
	}
}

void undoInscriptImage(Mat &img, int originalWidth, int originalHeight, const bool &toRotate, const bool &toResize, const short &leftBorderLength, const short &rightBorderLength) {
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
Point2f rotatePoint(Point2f point, int destWidth, int destHeight) {
	float old_row = point.y;
	point.y = point.x;
	point.x = destWidth - old_row;

	return point;
}

/**
 * @brief Find the point in the 90° counter-clockwise rotated image corresponding to the input point.
 * @param point point with still to be "unrotated" image coordinates.
 * @param destImg destination image (used to get its size).	TODO
 * @return "unrotated" Point2f.
 */
Point2f unrotatePoint(Point2f point, int destWidth, int destHeight) {
	float old_row = point.y;
	point.y = destHeight - point.x;
	point.x = old_row;

	return point;
}

/**
 * @brief Find the same rect in the 90° clockwise rotated image corresponding to the input point.
 * @param rect Rect with still to be rotated image coordinates.
 * @param destImg destination image (used to get its size).	TODO
 * @return rotated Rect.
 */
Rect rotateRect(Rect rect, int destWidth, int destHeight) {
	Point2f oldTR(rect.tl().x + rect.width, rect.tl().y);
	Point2f oldBL(rect.tl().x, rect.tl().y + rect.height);

	return Rect(rotatePoint(oldTR, destWidth, destHeight), rotatePoint(oldBL, destWidth, destHeight));
}

/**
 * @brief Find the same rect in the 90° counter-clockwise rotated image corresponding to the input point.
 * @param rect Rect with still to be unrotated image coordinates.
 * @param destImg destination image (used to get its size).	TODO
 * @return "unrotated" Rect.
 */
Rect unrotateRect(Rect rect, int destWidth, int destHeight) {
	Point2f oldTR(rect.tl().x + rect.width, rect.tl().y);
	Point2f oldBL(rect.tl().x, rect.tl().y + rect.height);

	return Rect(unrotatePoint(oldTR, destWidth, destHeight), unrotatePoint(oldBL, destWidth, destHeight));
}

void doInscriptTableObject(Table &table, int targetWidth, int targetHeight, int originalWidth, int originalHeight, const bool &toRotate, const bool &toResize, const short &leftBorderLength, const short &rightBorderLength, bool changeBboxPrec /* = false */) {
	Vec<Point2f, 4> boundaries = table.getBoundaries();
		// transform matrix is not being inscripted
	Ptr<vector<Ball>> balls = table.ballsPtr();

	if (toRotate) {
		for (size_t i = 0; i < 4; ++i) {
			boundaries[i] = rotatePoint(boundaries[i], targetWidth, targetHeight);
		}
		table.setBoundaries(boundaries);

		for (Ball &ball : *balls) {
			ball.setBbox(rotateRect(ball.getBbox(), targetWidth, targetHeight));
			if (changeBboxPrec) {
				ball.setBbox_prec(rotateRect(ball.getBbox_prec(), targetWidth, targetHeight));
			}
		}
	}

	if (toResize) {
		for (size_t i = 0; i < 4; ++i) {
			boundaries[i].x = boundaries[i].x/originalHeight*targetHeight;
			boundaries[i].y = boundaries[i].y/originalHeight*targetHeight;
		}
		table.setBoundaries(boundaries);

		for (Ball &ball : *balls) {
			ball.setBbox(Rect(ball.getBbox().tl().x/originalHeight*targetHeight, ball.getBbox().tl().y/originalHeight*targetHeight, ball.getBbox().width/originalHeight*targetHeight, ball.getBbox().height/originalHeight*targetHeight));
			if (changeBboxPrec) {
				ball.setBbox_prec(Rect(ball.getBbox_prec().tl().x/originalHeight*targetHeight, ball.getBbox_prec().tl().y/originalHeight*targetHeight, ball.getBbox_prec().width/originalHeight*targetHeight, ball.getBbox_prec().height/originalHeight*targetHeight));
			}
		}
	}

	if (leftBorderLength > 0) {
		for (size_t i = 0; i < 4; ++i) {
			boundaries[i].x += leftBorderLength;
		}
		table.setBoundaries(boundaries);

		for (Ball &ball : *balls) {
			ball.setBbox(Rect(ball.getBbox().tl().x + leftBorderLength, ball.getBbox().tl().y, ball.getBbox().width, ball.getBbox().height));
			if (changeBboxPrec) {
				ball.setBbox_prec(Rect(ball.getBbox_prec().tl().x + leftBorderLength, ball.getBbox_prec().tl().y, ball.getBbox_prec().width, ball.getBbox_prec().height));
			}
		}
	}
}

void undoInscriptTableObject(Table &table, int targetWidth, int targetHeight, int originalWidth, int originalHeight, const bool &toRotate, const bool &toResize, const short &leftBorderLength, const short &rightBorderLength, bool changeBboxPrec /* = false */) {
	Vec<Point2f, 4> boundaries = table.getBoundaries();
	// transform matrix is not being (un)inscripted
	Ptr<vector<Ball>> balls = table.ballsPtr();

	if (leftBorderLength) {
		for (size_t i = 0; i < 4; ++i) {
			boundaries[i].x -= leftBorderLength;
		}
		table.setBoundaries(boundaries);

		for (Ball &ball : *balls) {
			ball.setBbox(Rect(ball.getBbox().tl().x - leftBorderLength, ball.getBbox().tl().y, ball.getBbox().width, ball.getBbox().height));
			if (changeBboxPrec) {
				ball.setBbox_prec(Rect(ball.getBbox_prec().tl().x - leftBorderLength, ball.getBbox_prec().tl().y, ball.getBbox_prec().width, ball.getBbox_prec().height));
			}
		}
	}

	if (toResize) {
		for (size_t i = 0; i < 4; ++i) {
			boundaries[i].x = boundaries[i].x/targetHeight*originalHeight;
			boundaries[i].y = boundaries[i].y/targetHeight*originalHeight;
		}
		table.setBoundaries(boundaries);

		for (Ball &ball : *balls) {
			ball.setBbox(Rect(ball.getBbox().tl().x/targetHeight*originalHeight, ball.getBbox().tl().y/targetHeight*originalHeight, ball.getBbox().width/targetHeight*originalHeight, ball.getBbox().height/targetHeight*originalHeight));
			if (changeBboxPrec) {
				ball.setBbox_prec(Rect(ball.getBbox_prec().tl().x/targetHeight*originalHeight, ball.getBbox_prec().tl().y/targetHeight*originalHeight, ball.getBbox_prec().width/targetHeight*originalHeight, ball.getBbox_prec().height/targetHeight*originalHeight));
			}
		}
	}

	if (toRotate) {
		for (size_t i = 0; i < 4; ++i) {
			boundaries[i] = unrotatePoint(boundaries[i], originalWidth, originalHeight);
		}
		table.setBoundaries(boundaries);

		for (Ball &ball : *balls) {
			ball.setBbox(unrotateRect(ball.getBbox(), originalWidth, originalHeight));
			if (changeBboxPrec) {
				ball.setBbox_prec(unrotateRect(ball.getBbox_prec(), originalWidth, originalHeight));
			}
		}
	}

}
