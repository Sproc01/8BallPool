// Author: Michele Sprocatti

#include <opencv2/opencv.hpp>
#include <iostream>
#include <opencv2/features2d.hpp>
#include <math.h>

#include "table.h"
#include "ball.h"
#include "detection.h"
#include "util.h"
#include "minimapConstants.h"

using namespace cv;
using namespace std;

/**
 * @brief detect the corners of the table and its color in an image.
 * Create a mask using the most common color in the image central area, then evaluates the edge with the Canny
 * algorithm and then it uses Hough lines to detect the lines. To select the intersections, it computes them
 * and then merge the closest in order to have the four different corners.
 * @param frame image where there is a table to be detected.
 * @param corners output vector containing the 4 corners found.
 * @param colorRange output vector containing a range for the table colors.
 * @throw runtime_error if it does not find enough lines.
 * @throw runtime_error if it does not find enough interceptions.
 * @throw runtime_error if too many interceptions.
 * @throw invalid_argument if frame is empty.
 */
void detectTable(const Mat &frame, Vec<Point2f, 4> &corners, Vec2b &colorRange){

	if(frame.empty())
		throw invalid_argument("Empty image in input");

	// const used during the function
	const int DIM_STRUCTURING_ELEMENT = 4;
	const int CANNY_THRESHOLD1 = 200;
	const int CANNY_THRESHOLD2 = 250;
	const int THRESHOLD_HOUGH = 90;
	const int MAX_LINE_GAP = 35;
	const int MIN_LINE_LENGTH = 155;
	const int CLOSE_POINT_THRESHOLD = 50;

	// variables
	Mat imgGray, imgLine, imgBorder, thisImg, mask, kernel;
	vector<Vec4i> lines;
	int colsover4 = frame.cols/4;
	Scalar line_color = Scalar(0, 0, 255);
	vector<Point2f> intersections;
	vector<Vec3f> coefficients;

	// get the color range
	colorRange = mostFrequentHueColor(frame.colRange(colsover4, 3*colsover4));

	// mask the image
	cvtColor(frame, thisImg, COLOR_BGR2HSV);
	inRange(thisImg, Scalar(colorRange[0], S_CHANNEL_COLOR_THRESHOLD, V_CHANNEL_COLOR_THRESHOLD),
				Scalar(colorRange[1], 255, 255), mask);
	//imshow("Mask", mask);

	// morphological operations
	kernel = getStructuringElement(MORPH_RECT, Size(DIM_STRUCTURING_ELEMENT, DIM_STRUCTURING_ELEMENT));
	morphologyEx(mask, mask, MORPH_CLOSE, kernel, Point(-1,-1), 3);
	//imshow("Morphology", mask);

	// edge detection
	Canny(mask, imgBorder, CANNY_THRESHOLD1, CANNY_THRESHOLD2);
	//imshow("Canny Result", imgBorder);

	// Hough transform
	imgLine = frame.clone();
	HoughLinesP(imgBorder, lines, 1, CV_PI/180, THRESHOLD_HOUGH, MIN_LINE_LENGTH, MAX_LINE_GAP);

	// lines drawing
	Point pt1, pt2, pt3, pt4;
	float aLine, bLine, cLine;
	for(size_t i = 0; i < lines.size(); i++){

		pt1.x = lines[i][0];
		pt1.y = lines[i][1];
		pt2.x = lines[i][2];
		pt2.y = lines[i][3];
		line(imgLine, pt1, pt2, line_color, 2, LINE_AA);
		equationFormula(pt1.x, pt1.y, pt2.x, pt2.y, aLine, bLine, cLine);
		coefficients.push_back(Vec3f(aLine, bLine, cLine));
	}
	if(lines.size() < 4) // 4 lines needed to find 4 points
		throw runtime_error("Not enough lines found");

	// find intersections
	Point2f intersection;
	for(size_t i = 0; i < coefficients.size(); i++){

		for(size_t j = i+1; j < coefficients.size(); j++){

			computeIntersection(coefficients[i], coefficients[j], intersection);
			// if valid it is maintened
			if (intersection.x >= 0 && intersection.x < frame.cols && intersection.y >= 0 && intersection.y < frame.rows)
				intersections.push_back(intersection);
		}
	}
	Point center = Point(frame.cols/2, frame.rows/2);

	// remove intersections that are too close
	vector<Point2f> intersectionsGood;
	sort(intersections.begin(), intersections.end(), [](Point a, Point b) -> bool {
		return norm(a) < norm(b);
	});
	vector<Point2f>::iterator end2 = unique(intersections.begin(), intersections.end(),
		[&CLOSE_POINT_THRESHOLD](Point a, Point b) -> bool {
			return abs(a.x - b.x) < CLOSE_POINT_THRESHOLD && abs(a.y - b.y) < CLOSE_POINT_THRESHOLD;
	});
	for(vector<Point2f>::iterator it = intersections.begin(); it != end2; it++)
		intersectionsGood.push_back(*it);

	//cout << intersectionsGood.size() << endl;

	// clockwise order
	sort(intersectionsGood.begin(), intersectionsGood.end(), [&center](Point a, Point b) -> bool {

		if (a.x < center.x && b.x < center.x)
			return a.y > b.y;
		else if (a.x < center.x && b.x > center.x)
			return true;
		else if (a.x > center.x && b.x > center.x)
			return a.y < b.y;
		else
			return false;
	});

	if(intersectionsGood.size() < 4) // 4 corners of the table
		throw runtime_error("Not enough unique intersections found");
	else if(intersectionsGood.size() > 4)
		throw runtime_error("Too many unique intersections found"); // TODO decide

	vector<Scalar> colors = {Scalar(255, 0, 0), Scalar(0, 255, 0), Scalar(0, 0, 255), Scalar(255, 255, 0)};
	for(size_t i = 0; i < 4; i++)
		circle(imgLine, intersectionsGood[i], 10, colors[i], -1);

	for(size_t i = 0; i < 4; i++)
		corners[i] = intersectionsGood[i];

	imshow("Line", imgLine);
}

/**
 * @brief classify the ball inside the image passed as argument
 * It mask the image using a circle of the specified radius centered in the center of the image, it evaluates
 * the histogram and compute the two max values, using some conditions then it determines the class.
 * @param img image that contains only one ball centered in the center of the ball.
 * @param radius radius of the circle that correspond to the ball.
 * @return Category class of the ball.
 * @throw invalid_argument if img is empty or if the radius is <=0.
 */
Category classificationBall(const Mat& img, double radius){

	if(img.empty())
		throw invalid_argument("Empty image in input");

	if(radius <= 0)
		throw invalid_argument("Radius negative or equal to zero");

	// const to classify the ball
	const int MEAN_WHITE_CHANNEL2 = 130;
	const int MEAN_WHITE_CHANNEL3 = 150;
	const int MEAN_BLACK_CHANNEL3 = 110;
	const int NUMBER_OF_BINS_WHITE = 3;
	const int NUMBER_OF_BINS_BLACK = 5;
	const float THRESHOLD_STRIPED_MAX = 0.6;
	const float THRESHOLD_STRIPED_NPIXELS = 0.33;

	Mat hist, gray, mask, grayT, hsv, argmax, argmax2;


	mask = Mat::zeros(img.size(), CV_8U);
	// imshow("original", img);
	cvtColor(img, gray, COLOR_BGR2GRAY);
	Point2f center = Point(img.cols/2, img.rows/2);
	circle(mask, center, radius, 255, -1);
	// imshow("mask", mask);

	for(int i = 0; i < img.rows; i++)
		for(int j = 0; j < img.cols; j++)
			if(mask.at<uchar>(i,j) != 255)
				gray.at<uchar>(i,j) = 0;


	adaptiveThreshold(gray, grayT, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 3, 2);
	//imshow("grayT", grayT);
	//imshow("gray", gray);

	int numberOfBackgroundPixels = 4 * pow(radius, 2) - CV_PI * pow(radius, 2);
	//cout << numberOfBackgroundPixels << endl;
	const int channel[] = {0};
	int histSize = 10;
	float range[] = {0, 255};
	const float* histRange[] = {range};
	calcHist(&gray, 1, channel, Mat(), hist, 1, &histSize, histRange, true, false);
	hist.at<float>(0) -= numberOfBackgroundPixels;
	// cout << hist << endl;

	// first peak
	reduceArgMax(hist, argmax, 0);
	float val = hist.at<float>(argmax.at<int>(0));
	hist.at<float>(argmax.at<int>(0)) = 0;

	// second peak
	reduceArgMax(hist, argmax2, 0);
	float val2 = hist.at<float>(argmax2.at<int>(0));
	hist.at<float>(argmax2.at<int>(0)) = 0;

	vector<double> mean, stddev;
	cvtColor(img, hsv, COLOR_BGR2HSV);
	meanStdDev(hsv, mean, stddev);
	// cout << val << endl;
	// cout << argmax << endl;
	// cout << val2 << endl;
	// cout << argmax2 << endl;
	//waitKey(0);
	int count = 0;
	int numberOfPixels = grayT.rows * grayT.cols;
	for(int i = 0; i < grayT.rows; i++)
		for(int j = 0; j < grayT.rows; j++)
			if(grayT.at<uchar>(i,j) == 0)
				count++;

	if(val2 > THRESHOLD_STRIPED_MAX * val
		&& (argmax2.at<int>(0) > NUMBER_OF_BINS_WHITE || argmax.at<int>(0) > NUMBER_OF_BINS_WHITE))
			if (count > THRESHOLD_STRIPED_NPIXELS * numberOfPixels)
				return STRIPED_BALL;

	if((argmax.at<int>(0) > NUMBER_OF_BINS_WHITE || argmax2.at<int>(0) > NUMBER_OF_BINS_WHITE)
		 && mean[1] < MEAN_WHITE_CHANNEL2 && mean[2] > MEAN_WHITE_CHANNEL3)
		return WHITE_BALL;

	if(argmax.at<int>(0) < NUMBER_OF_BINS_BLACK && mean[2] < MEAN_BLACK_CHANNEL3)
		return BLACK_BALL;

	return SOLID_BALL;

}

/**
 * @brief detect balls in an image given some information about the table.
 * In order to do this it exploits the information in the class table. Uses a bilateral filter to remove
 * noise but maintain the edges. Cluster the image using kmeans, another bilateral filter and then hough circles.
 * To isolate the good circles exploit the information of the table.
 * @param frame image where there are the balls to be detected.
 * @param table initialized object that contains the corner and the color.
 * @param balls output vector of the balls detected.
 * @throw invalid_argument if frame is empty.
 */
void detectBalls(const Mat &frame, const Table &table, vector<Ball> &balls){

	if(frame.empty())
		throw invalid_argument("Empty image in input");

	//table properties
	const int NUMBER_CORNERS = 4;
	Vec2b colorTable = table.getColor();
	Vec<Point2f, NUMBER_CORNERS> tableCorners = table.getBoundaries();

	// const used during the function
	const int MIN_RADIUS = 6;
	const int MAX_RADIUS = 14;
	const int HOUGH_PARAM1 = 200;
	const int HOUGH_PARAM2 = 8;
	const float INVERSE_ACCUMULATOR_RESOLUTION = 0.1;
	const int MIN_DISTANCE = 19;
	const int SIZE_BILATERAL = 3;
	const int SIGMA_COLOR = 15;
	const int SIGMA_SPACE = 70;
	Scalar border_color = Scalar(0, 255, 255);

	// variables
	Mat gray, HSVImg, mask, smooth, kernel;
	Mat frameRect = frame.clone();
	Mat frameCircle = frame.clone();
	Mat cropped = Mat::zeros(frame.size(), CV_8UC1);
	vector<Vec3f> circles;

	//creation of the mask
	cvtColor(frame, HSVImg, COLOR_BGR2HSV);
	// imshow("HSV", HSVImg);

	inRange(HSVImg, Scalar(colorTable[0], S_CHANNEL_COLOR_THRESHOLD, V_CHANNEL_COLOR_THRESHOLD),
			Scalar(colorTable[1], 255, 255), mask);
	kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
	morphologyEx(mask, mask, MORPH_DILATE, kernel, Point(-1,-1), 2);
	imshow("mask dilate", mask);

	// smoothing
	bilateralFilter(frame, smooth, SIZE_BILATERAL, SIGMA_COLOR, SIGMA_SPACE);
	// imshow("smoothed", smooth);

	//poly to isolate the table
	vector<Point> tableCornersInt;
	for(int i = 0; i < NUMBER_CORNERS; i++) // needed otherwise exception
		tableCornersInt.push_back(Point(static_cast<int>(tableCorners[i].x), static_cast<int>(tableCorners[i].y)));
	fillConvexPoly(cropped, tableCornersInt, 255);
	//imshow("Poly", cropped);
	for(int i = 0; i < tableCornersInt.size(); i++)
		circle(cropped, tableCornersInt[i], 20, 0, FILLED, 8, 0);
	//imshow("Cropped", cropped);
	kernel = getStructuringElement(MORPH_ELLIPSE, Size(2, 2));
	morphologyEx(cropped, cropped, MORPH_ERODE, kernel, Point(-1,-1), 6);
	//imshow("Poly eroded", cropped);

	// mask the smooth image
	for(int i = 0; i < cropped.rows; i++)
		for(int j = 0; j < cropped.cols; j++)
			if(cropped.at<uchar>(i, j) != 255)
				smooth.at<Vec3b>(i,j) = Vec3b(0, 0, 0);

	// clustering
	Mat resClustering;
	Mat resClusteringSmooth;
	vector<Vec3b> colors = {
		Vec3b(0, 0, 255),
		Vec3b(0, 255, 0),
		Vec3b(255, 0, 0),
		Vec3b(255, 255, 255),
		Vec3b(0, 0, 0),
	};
	kMeansClustering(smooth, colors, resClustering);
	bilateralFilter(resClustering, resClusteringSmooth, SIZE_BILATERAL, SIGMA_COLOR, SIGMA_SPACE);
	cvtColor(resClusteringSmooth, gray, COLOR_BGR2GRAY);
	imshow("Kmeans", resClusteringSmooth);
	// adaptiveThreshold(gray, gray, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 3, 2);
	// kernel = getStructuringElement(MORPH_ELLIPSE, Size(2, 2));
	// morphologyEx(mask, mask, MORPH_CLOSE, kernel, Point(-1,1), 2);
	imshow("Kmeans gray", gray);

	// Hough transform
	HoughCircles(gray, circles, HOUGH_GRADIENT, INVERSE_ACCUMULATOR_RESOLUTION,
					MIN_DISTANCE, HOUGH_PARAM1, HOUGH_PARAM2, MIN_RADIUS, MAX_RADIUS);

	// circles classification
	Category category;
	Point center;
	int radius;
	Vec3i c;
	Rect rect;
	bool ballFound;
	Mat subImg;
	vector<Vec3f> lines;
	double meanRadius = 0;
	int counter = 0;
	for(size_t i = 0; i < circles.size(); i++ ){

		c = circles[i];
	 	center = Point(c[0], c[1]);
	 	radius = c[2];
		// inside the table and with a color different from the table color
		if(cropped.at<uchar>(center.y, center.x) == 255
	 		&& cropped.at<uchar>(center.y+radius, center.x) == 255
			&& cropped.at<uchar>(center.y-radius, center.x) == 255
	 		&& cropped.at<uchar>(center.y, center.x+radius) == 255
	 		&& cropped.at<uchar>(center.y, center.x-radius) == 255
	 		&& mask.at<uchar>(center.y, center.x) == 0){

			meanRadius+= radius;
			counter++;
		}
	}
	meanRadius /= counter;
	for(size_t i = 0; i < circles.size(); i++ ){

		ballFound = true;
		c = circles[i];
	 	center = Point(c[0], c[1]);
	 	radius = c[2];
		// inside the table and with a color different from the table color, not too big and not too small
		if(radius > 0.7 * meanRadius && radius < 1.3 * meanRadius
			&& cropped.at<uchar>(center.y, center.x) == 255
	 		&& cropped.at<uchar>(center.y+radius, center.x) == 255
			&& cropped.at<uchar>(center.y-radius, center.x) == 255
	 		&& cropped.at<uchar>(center.y, center.x+radius) == 255
	 		&& cropped.at<uchar>(center.y, center.x-radius) == 255
	 		&& mask.at<uchar>(center.y, center.x) == 0){

			subImg = frame.colRange(c[0]-radius, c[0]+radius).rowRange(c[1]-radius, c[1]+radius);
			rect = Rect(center.x-c[2], center.y-c[2], 2*c[2], 2*c[2]);
			category = classificationBall(subImg, radius);

			switch(category){

				case WHITE_BALL:
						circle(frameCircle, center, radius, WHITE_BGR_COLOR, 1, LINE_AA);
						rectangle(frameRect, rect, WHITE_BGR_COLOR, 1, LINE_AA);
					break;
				case BLACK_BALL:
						circle(frameCircle, center, radius, BLACK_BGR_COLOR, 1, LINE_AA);
						rectangle(frameRect, rect, BLACK_BGR_COLOR, 1, LINE_AA);
					break;
				case SOLID_BALL:
						circle(frameCircle, center, radius, SOLID_BGR_COLOR, 1, LINE_AA);
						rectangle(frameRect, rect, SOLID_BGR_COLOR, 1, LINE_AA);
					break;
				case STRIPED_BALL:
						circle(frameCircle, center, radius, STRIPED_BGR_COLOR, 1, LINE_AA);
						rectangle(frameRect, rect, STRIPED_BGR_COLOR, 1, LINE_AA);
					break;
				default:
						ballFound = false;
					break;
			}
			if(ballFound)
				balls.push_back(Ball(rect, category));
		}
		for(int i = 0; i < tableCornersInt.size()-1; i++)
			line(frameRect, tableCornersInt[i], tableCornersInt[i+1], border_color, 2, LINE_AA);

		line(frameRect, tableCornersInt[0], tableCornersInt[3], border_color, 2, LINE_AA);

	}
	// imshow("detected circles", frameCircle);
	imshow("detected rectangles", frameRect);
}
