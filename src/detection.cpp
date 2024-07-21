// Author: Michele Sprocatti

#include <opencv2/opencv.hpp>
#include <cmath>
#include <stdexcept>

#include "table.h"
#include "ball.h"
#include "detection.h"
#include "util.h"
#include "minimap.h"

using namespace cv;
using namespace std;

/**
 * @brief classify the ball inside the image passed as argument
 * It masks the image using a circle of the specified radius centered in the center of the image, it evaluates
 * the histogram and compute the two max values, using some conditions then it determines the class.
 * @param img image that contains only one ball centered in the center of the ball, BGR format requested.
 * @param radius radius of the circle that corresponds to the ball.
 * @return Category class of the ball.
 * @throw invalid_argument if img is empty
 * 			or if the radius is <=0 or if img has a number of channels different from 3.
 */
Category classificationBall(const Mat& img, double radius){

	if(img.empty())
		throw invalid_argument("Empty image in input");

	if(img.channels() != 3)
		throw invalid_argument("Invalid number of channels for the input image");

	if(radius <= 0)
		throw invalid_argument("Radius negative or equal to zero");

	// const to classify the ball
	const int MEAN_WHITE_CHANNEL2 = 130;
	const int MEAN_WHITE_CHANNEL3 = 160;
	const int MEAN_BLACK_CHANNEL3 = 115;
	const int NUMBER_OF_BINS_WHITE = 3;
	const int NUMBER_OF_BINS_BLACK = 2;
	const float THRESHOLD_STRIPED_MAX = 0.3;
	const float THRESHOLD_DEV_STRIPED = 55;

	// imshow("original", img);

	Mat hist, gray, mask, hsv, argmax, argmax2;
	Point2f center;

	// mask the image
	mask = Mat::zeros(img.size(), CV_8U);
	cvtColor(img, gray, COLOR_BGR2GRAY);
	center = Point(img.cols/2, img.rows/2);
	circle(mask, center, radius, 255, -1);
	// imshow("mask", mask);
	for(int i = 0; i < img.rows; i++)
		for(int j = 0; j < img.cols; j++)
			if(mask.at<uchar>(i,j) != 255)
				gray.at<uchar>(i,j) = 0;

	// imshow("gray", gray);

	// compute the histogram
	int numberOfBackgroundPixels = 4 * pow(radius, 2) - CV_PI * pow(radius, 2);
	const int channel[] = {0};
	int histSize = 10;
	float range[] = {0, 255};
	const float* histRange[] = {range};
	calcHist(&gray, 1, channel, Mat(), hist, 1, &histSize, histRange, true, false);
	hist.at<float>(0) -= numberOfBackgroundPixels;

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
	meanStdDev(hsv, mean, stddev, mask);

	// classification
	if(argmax.at<int>(0) < NUMBER_OF_BINS_BLACK
		&& mean[2] < MEAN_BLACK_CHANNEL3)
		return BLACK_BALL;

	if((argmax.at<int>(0) > NUMBER_OF_BINS_WHITE)
		 && mean[1] < MEAN_WHITE_CHANNEL2
		 && mean[2] > MEAN_WHITE_CHANNEL3)
		return WHITE_BALL;

	// skin color tested, thresholds found on the web and adjusted to the dataset
	const int LOWER_MEAN_HUE = 0;
	const int UPPER_MEAN_HUE = 50;
	const int LOWER_MEAN_SATURATION = 80;
	const int UPPER_MEAN_SATURATION = 100;
	const int LOWER_MEAN_VALUE = 140;
	const int UPPER_MEAN_VALUE = 250;
	if(mean[0] > LOWER_MEAN_HUE && mean[0] < UPPER_MEAN_HUE
		&& mean[1] > LOWER_MEAN_SATURATION && mean[1] < UPPER_MEAN_SATURATION
		&& mean[2] > LOWER_MEAN_VALUE && mean[2] < UPPER_MEAN_VALUE)
		return BACKGROUND;

	if(val2 > THRESHOLD_STRIPED_MAX * val
		&& stddev[1] > THRESHOLD_DEV_STRIPED)
			return STRIPED_BALL;

	return SOLID_BALL;
}

/**
 * @brief Change the category of the balls in order to have only one white ball
 * and only one black ball in the vector.
 * @param img image where there is the ball, HSV format requested.
 * @param balls pointer to a vector of balls where to do non-maxima suppression.
 * @throw invalid_argument if the img is empty, if the image has less than 3 channels,
 *  		if the vector pointed by balls is empty or if balls is nullptr
 */
void nonMaximaSuppressionWhiteBlack(const Mat &img, Ptr<vector<Ball>> balls)
{
	if(img.empty())
		throw invalid_argument("Empty input image");
	if(img.channels() != 3)
		throw invalid_argument("Wrong number of channels");
	if(balls == nullptr)
		throw invalid_argument("Null pointer");
	if(balls->empty())
		throw invalid_argument("Empty vector of balls");

	vector<int> whiteFound;
	vector<int> blackFound;
	for(int i = 0; i < balls->size(); i++){
		if((balls->at(i)).getCategory() == WHITE_BALL)
			whiteFound.push_back(i);

		else if((balls->at(i)).getCategory() == BLACK_BALL)
			blackFound.push_back(i);
	}

	if(whiteFound.size() > 1){
		sort(whiteFound.begin(), whiteFound.end(), [&balls, &img](int a, int b) -> bool {
			Rect BboxA = (balls->at(a)).getBbox();
			Rect BboxB = (balls->at(b)).getBbox();
			Mat subImgA = img(BboxA);
			Mat subImgB = img(BboxB);

			double radius = (BboxA.br().y - BboxA.tl().y) / 2.0;
			Point center = Point(subImgA.cols/2, subImgA.rows/2);
			Mat maskA = Mat::zeros(subImgA.size(), CV_8U);
			circle(maskA, center, radius, 255, -1);

			radius = (BboxB.br().y - BboxB.tl().y) / 2.0;
			center = Point(subImgB.cols/2, subImgB.rows/2);
			Mat maskB = Mat::zeros(subImgB.size(), CV_8U);
			circle(maskB, center, radius, 255, -1);

			vector<double> meanA, stddevA;
			vector<double> meanB, stddevB;
			meanStdDev(subImgA, meanA, stddevA, maskA);
			meanStdDev(subImgB, meanB, stddevB, maskB);
			return meanA[1] < meanB[1];
		});

		for(int i = 1; i < whiteFound.size(); i++)
			(balls->at(whiteFound[i])).setCategory(STRIPED_BALL);

	}

	if(blackFound.size() > 1){
		sort(blackFound.begin(), blackFound.end(), [&balls, &img](int a, int b) -> bool {
			Rect BboxA = (balls->at(a)).getBbox();
			Rect BboxB = (balls->at(b)).getBbox();
			Mat subImgA = img(BboxA);
			Mat subImgB = img(BboxB);

			double radius = (BboxA.br().y - BboxA.tl().y) / 2.0;
			Point center = Point(subImgA.cols/2, subImgA.rows/2);
			Mat maskA = Mat::zeros(subImgA.size(), CV_8U);
			circle(maskA, center, radius, 255, -1);

			radius = (BboxB.br().y - BboxB.tl().y) / 2.0;
			center = Point(subImgB.cols/2, subImgB.rows/2);
			Mat maskB = Mat::zeros(subImgB.size(), CV_8U);
			circle(maskB, center, radius, 255, -1);

			vector<double> meanA, stddevA;
			vector<double> meanB, stddevB;
			meanStdDev(subImgA, meanA, stddevA, maskA);
			meanStdDev(subImgB, meanB, stddevB, maskB);

			// in the provided dataset the light is on top of the table
			// so the black ball reflect it in some videos
			// so we need an higher standard deviation in the third channel
			return meanA[2] < meanB[2] && stddevA[2] > stddevB[2];
		});
		for(int i = 1; i < blackFound.size(); i++)
			(balls->at(blackFound[i])).setCategory(SOLID_BALL);

	}
}

/**
 * @brief detect the corners of the table and its color in an image.
 * Create a mask using the most common color in the image central area, then evaluates the edge with the Canny
 * algorithm and then it uses Hough lines to detect the lines. To select the intersections, it computes them
 * and then merge the closest in order to have the four different corners.
 * @param frame image where there is a table to be detected, BGR format requested.
 * @param corners output vector containing the 4 corners found.
 * @param colorRange output vector containing a range for the table colors.
 * @throw runtime_error if it does not find enough lines or if it does not find enough interceptions.
 * @throw invalid_argument if frame is empty or if frame has a number of channels different from 3.
 */
void detectTable(const Mat &frame, Vec<Point2f, 4> &corners, Vec2b &colorRange){

	if(frame.empty())
		throw invalid_argument("Empty image in input");
	if(frame.channels() != 3)
		throw invalid_argument("Invalid number of channels for the input image");


	// const used during the function
	const int DIM_STRUCTURING_ELEMENT = 4;
	const int CANNY_THRESHOLD1 = 200;
	const int CANNY_THRESHOLD2 = 250;
	const int THRESHOLD_HOUGH = 90;
	const int MAX_LINE_GAP = 35;
	const int MIN_LINE_LENGTH = 155;
	const int CLOSE_POINT_THRESHOLD = 50;

	// variables
	Mat imgGray, imgBorder, thisImg, mask, kernel;
	vector<Vec4i> lines;
	int colsover4 = frame.cols/4;
	Scalar line_color = Scalar(0, 0, 255);
	vector<Point2f> intersections;
	vector<Vec3f> coefficients;

	// get the color range for the table
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
	// Mat imgLine = frame.clone();
	HoughLinesP(imgBorder, lines, 1, CV_PI/180, THRESHOLD_HOUGH, MIN_LINE_LENGTH, MAX_LINE_GAP);

	// lines drawing
	float aLine, bLine, cLine;
	for(size_t i = 0; i < lines.size(); i++){
		Point pt1, pt2;
		pt1.x = lines[i][0];
		pt1.y = lines[i][1];
		pt2.x = lines[i][2];
		pt2.y = lines[i][3];
		// line(imgLine, pt1, pt2, line_color, 2, LINE_AA);
		equationFormula(pt1.x, pt1.y, pt2.x, pt2.y, aLine, bLine, cLine);
		coefficients.push_back(Vec3f(aLine, bLine, cLine));
	}
	if(lines.size() < 4) // at least 4 lines needed to find 4 points
		throw runtime_error("Not enough lines found");

	// find intersections
	Point2f intersection;
	for(size_t i = 0; i < coefficients.size(); i++){

		for(size_t j = i+1; j < coefficients.size(); j++){

			computeIntersection(coefficients[i], coefficients[j], intersection);
			// if valid it is maintened
			if (intersection.x >= 0 && intersection.x < frame.cols
					&& intersection.y >= 0 && intersection.y < frame.rows)
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

	// at least 4 corners of the table
	if(intersectionsGood.size() < 4)
		throw runtime_error("Not enough unique intersections found");

	if(intersectionsGood.size() > 4) // if more take the 4 nearest the center
		sort(intersectionsGood.begin(), intersectionsGood.end(), [&center](Point a, Point b) -> bool {
			return norm(a - center) < norm(b - center);
		});

	// clockwise order
	sort(intersectionsGood.begin(), intersectionsGood.begin()+4, [&center](Point a, Point b) -> bool {

		if (a.x < center.x && b.x < center.x)
			return a.y > b.y;
		else if (a.x < center.x && b.x > center.x)
			return true;
		else if (a.x > center.x && b.x > center.x)
			return a.y < b.y;
		else
			return false;
	});

	vector<Scalar> colors = {Scalar(255, 0, 0), Scalar(0, 255, 0), Scalar(0, 0, 255), Scalar(255, 255, 0)};
	// for(size_t i = 0; i < 4; i++)
	// 	circle(imgLine, intersectionsGood[i], 10, colors[i], -1);

	for(size_t i = 0; i < 4; i++)
		corners[i] = intersectionsGood[i];

	//imshow("Line", imgLine);
}

/**
 * @brief detect balls in an image given some information about the table.
 * In order to do this it exploits the information in the class table. Uses a bilateral filter to remove
 * noise but maintain the edges. Cluster the image using kmeans, another bilateral filter and then hough circles.
 * To isolate the good circles exploit the information of the table.
 * @param frame image where there are the balls to be detected, BGR format requested.
 * @param table initialized object that contains the corner and the color, the balls are added in this function.
 * @throw invalid_argument if frame is empty or if frame has a number of channels different from 3.
 */
void detectBalls(const Mat &frame, Table &table){

	if(frame.empty())
		throw invalid_argument("Empty image in input");

	if(frame.channels() != 3)
		throw invalid_argument("Invalid number of channels for the input image");

	//table properties
	const int NUMBER_CORNERS = 4;
	Vec2b colorTable = table.getColorRange();
	Vec<Point2f, NUMBER_CORNERS> tableCorners = table.getBoundaries();
	Ptr<vector<Ball>> balls = table.ballsPtr();

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
	const float RANGE_RADIUS = 0.3;
	const int RADIUS_CORNERS = 20;

	vector<Vec3b> colors = {
		Vec3b(0, 0, 255),
		Vec3b(0, 255, 0),
		Vec3b(255, 0, 0),
		Vec3b(255, 255, 0),
		Vec3b(0, 255, 255),
	}; // needed as input for the clustering (5 colors with different gray level)

	// variables
	Mat gray, HSVImg, mask, smooth, kernelMorphological, resClustering, resClusteringSmooth;
	Mat poly = Mat::zeros(frame.size(), CV_8UC1);
	vector<Vec3f> circles;

	//creation of the mask
	cvtColor(frame, HSVImg, COLOR_BGR2HSV);
	// imshow("HSV", HSVImg);
	inRange(HSVImg, Scalar(colorTable[0], S_CHANNEL_COLOR_THRESHOLD, V_CHANNEL_COLOR_THRESHOLD),
			Scalar(colorTable[1], 255, 255), mask);
	kernelMorphological = getStructuringElement(MORPH_ELLIPSE, Size(2, 2));
	morphologyEx(mask, mask, MORPH_DILATE, kernelMorphological);
	//imshow("mask dilate", mask);

	// smoothing
	bilateralFilter(frame, smooth, SIZE_BILATERAL, SIGMA_COLOR, SIGMA_SPACE);
	// imshow("smoothed", smooth);

	// poly to isolate the table
	vector<Point> tableCornersInt;
	for(int i = 0; i < NUMBER_CORNERS; i++) // needed otherwise exception
		tableCornersInt.push_back(Point(static_cast<int>(tableCorners[i].x), static_cast<int>(tableCorners[i].y)));
	fillConvexPoly(poly, tableCornersInt, 255);
	for(int i = 0; i < tableCornersInt.size(); i++)
		circle(poly, tableCornersInt[i], RADIUS_CORNERS, 0, FILLED, 8, 0);
	kernelMorphological = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
	morphologyEx(poly, poly, MORPH_ERODE, kernelMorphological, Point(-1,-1), 7);
	//imshow("Poly eroded", poly);

	// mask the smooth image
	for(int i = 0; i < poly.rows; i++)
		for(int j = 0; j < poly.cols; j++)
			if(poly.at<uchar>(i, j) != 255)
				smooth.at<Vec3b>(i,j) = Vec3b(0, 0, 0);

	// clustering
	kMeansClustering(smooth, colors, resClustering);
	cvtColor(resClustering, gray, COLOR_BGR2GRAY);
	// imshow("Kmeans gray", gray);
	// imshow("Kmeans", resClustering);

	/*
	float minRadius, maxRadius;
	radiusInterval(min_temp, max_temp, tableCorners);
	*/
	// Hough transform
	HoughCircles(gray, circles, HOUGH_GRADIENT, INVERSE_ACCUMULATOR_RESOLUTION,
					MIN_DISTANCE, HOUGH_PARAM1, HOUGH_PARAM2, MIN_RADIUS, MAX_RADIUS);

	// compute the mean of good circles
	Category category;
	Point center;
	int radius;
	Vec3i c;
	Rect rect;
	Mat subImg;
	vector<Vec3f> lines;
	double meanRadius = 0;
	int counter = 0;
	for(size_t i = 0; i < circles.size(); i++ ){

		c = circles[i];
	 	center = Point(c[0], c[1]);
	 	radius = c[2];
		// inside the table and with a color different from the table color
		if(poly.at<uchar>(center.y, center.x) == 255
	 		&& poly.at<uchar>(center.y+radius, center.x) == 255
			&& poly.at<uchar>(center.y-radius, center.x) == 255
	 		&& poly.at<uchar>(center.y, center.x+radius) == 255
	 		&& poly.at<uchar>(center.y, center.x-radius) == 255
	 		&& mask.at<uchar>(center.y, center.x) == 0){

			meanRadius+= radius;
			counter++;
		}
	}
	meanRadius /= counter;

	for(size_t i = 0; i < circles.size(); i++ ){
		c = circles[i];
	 	center = Point(c[0], c[1]);
	 	radius = c[2];
		// inside the table and with a color different from the table color, not too big and not too small
		if(radius > (1 - RANGE_RADIUS) * meanRadius && radius <  (1 + RANGE_RADIUS) * meanRadius
			&& poly.at<uchar>(center.y, center.x) == 255
	 		&& poly.at<uchar>(center.y+radius, center.x) == 255
			&& poly.at<uchar>(center.y-radius, center.x) == 255
	 		&& poly.at<uchar>(center.y, center.x+radius) == 255
	 		&& poly.at<uchar>(center.y, center.x-radius) == 255
	 		&& mask.at<uchar>(center.y, center.x) == 0){

			rect = Rect(center.x-c[2], center.y-c[2], 2*c[2], 2*c[2]);
			subImg = frame(rect);
			category = classificationBall(subImg, radius);
			if(category != BACKGROUND)
				balls->push_back(Ball(rect, category));
		}
	}

	nonMaximaSuppressionWhiteBlack(HSVImg, balls);
}
