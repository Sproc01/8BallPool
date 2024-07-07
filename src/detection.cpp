// Author: Michele Sprocatti

#define _USE_MATH_DEFINES

#include <opencv2/opencv.hpp>
#include <iostream>
#include <opencv2/features2d.hpp>

#include "table.h"
#include "ball.h"
#include "detection.h"
#include "util.h"
#include "minimapConstants.h"
#include "math.h"

using namespace cv;
using namespace std;

void detectTable(const Mat &frame, Vec<Point2f, 4> &corners, Vec2b &colorRange)
{
	// const used during the function
	const int DIM_STRUCTURING_ELEMENT = 12;
	const int CANNY_THRESHOLD1 = 200;
	const int CANNY_THRESHOLD2 = 250;
	const int THRESHOLD_HOUGH = 90;
	const int MAX_LINE_GAP = 35;
	const int MIN_LINE_LENGTH = 150;
	const int CLOSE_POINT_THRESHOLD = 50;
	// variables
	Mat imgGray, imgLine, imgBorder, thisImg, mask, kernel;
	vector<Vec4i> lines;
	//int rowsover4 = frame.rows/4;
	int colsover4 = frame.cols/4;
	Scalar line_color = Scalar(0, 0, 255);
	vector<Point2f> intersections;
	vector<Vec3f> coefficients;

	// get the color range
	colorRange = mostFrequentColor(frame.colRange(colsover4, 3*colsover4)); //.rowRange(rowsover4, 3*rowsover4)

	// // mask the image
	cvtColor(frame, thisImg, COLOR_BGR2HSV);
	inRange(thisImg, Scalar(colorRange[0], S_CHANNEL_COLOR_THRESHOLD, V_CHANNEL_COLOR_THRESHOLD),
				Scalar(colorRange[1], 255, 255), mask);
	//imshow("Mask", mask);

	// morphological operations
	kernel = getStructuringElement(MORPH_CROSS, Size(DIM_STRUCTURING_ELEMENT, DIM_STRUCTURING_ELEMENT));
	morphologyEx(mask, mask, MORPH_CLOSE, kernel);
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
	int maxI = 6;
	if(maxI > lines.size()) maxI = lines.size();
	for(size_t i = 0; i < maxI; i++)
	{
		pt1.x = lines[i][0];
		pt1.y = lines[i][1];
		pt2.x = lines[i][2];
		pt2.y = lines[i][3];
		line(imgLine, pt1, pt2, line_color, 2, LINE_AA);
		equationFormula(pt1.x, pt1.y, pt2.x, pt2.y, aLine, bLine, cLine);
		coefficients.push_back(Vec3f(aLine, bLine, cLine));
	}
	if(lines.size() < 4)
		throw runtime_error("Not enough lines found");

	// find intersections
	Point2f intersection;
	for(size_t i = 0; i < coefficients.size(); i++)
	{
		for(size_t j = i+1; j < coefficients.size(); j++)
		{
			computeIntersection(coefficients[i], coefficients[j], intersection);
			if (intersection.x >= 0 && intersection.x < frame.cols && intersection.y >= 0 && intersection.y < frame.rows)
				intersections.push_back(intersection);
		}
	}
	Point center = Point(frame.cols/2, frame.rows/2);
	// remove intersections that are too close
	vector<Point2f> intersectionsGood;
	sort(intersections.begin(), intersections.end(), [](Point a, Point b) -> bool
	{
		return norm(a) < norm(b);
	});
	// auto or this one TODO decide
	vector<Point2f>::iterator end2 = unique(intersections.begin(), intersections.end(), [&CLOSE_POINT_THRESHOLD](Point a, Point b) -> bool
	{
		return abs(a.x - b.x) < CLOSE_POINT_THRESHOLD && abs(a.y - b.y) < CLOSE_POINT_THRESHOLD;
	});
	// auto or this one TODO decide
	for(vector<Point2f>::iterator it = intersections.begin(); it != end2; it++)
	{
		intersectionsGood.push_back(*it);
	}

	// clockwise order
	sort(intersectionsGood.begin(), intersectionsGood.end(), [&center](Point a, Point b) -> bool
	{
		if (a.x < center.x && b.x < center.x)
			return a.y > b.y;
		else if (a.x < center.x && b.x > center.x)
			return true;
		else if (a.x > center.x && b.x > center.x)
			return a.y < b.y;
		else
			return false;
	});

	if(intersectionsGood.size() < 4)
		throw runtime_error("Not enough unique intersections found");

	vector<Scalar> colors = {Scalar(255, 0, 0), Scalar(0, 255, 0), Scalar(0, 0, 255), Scalar(255, 255, 0)};
	for(size_t i = 0; i < 4; i++)
	{
		circle(imgLine, intersectionsGood[i], 10, colors[i], -1);
	}

	for(size_t i = 0; i < 4; i++)
	{
		corners[i] = intersectionsGood[i];
	}
	imshow("Line", imgLine);
}


Category classifyBall(const Mat& img, double radius)
{
	// const to classify the ball
	const int MEAN_WHITE_CHANNEL2 = 130;
	const int MEAN_WHITE_CHANNEL3 = 150;
	const int MEAN_BLACK_CHANNEL3 = 115;

	Mat hist, gray, mask;
	mask = Mat::zeros(img.size(), CV_8U);
	//imshow("original", img);
	cvtColor(img, gray, COLOR_BGR2GRAY);
	Point2f center = Point(img.cols/2, img.rows/2);
	circle(mask, center, radius, 255, FILLED, 8, 0);
	//imshow("mask", mask);
	for(int i = 0; i < img.rows; i++)
		for(int j = 0; j < img.cols; j++)
			if(mask.at<uchar>(i,j) != 255)
				gray.at<uchar>(i,j) = 0;
	//imshow("gray", gray);
	//cout << img.rows << "," << img.cols << endl;
	int numberOfBackgroundPixels = 4 * pow(radius, 2) - M_PI * pow(radius, 2);
	//cout << numberOfBackgroundPixels << endl;
	const int channel[] = {0};
	int histSize = 20;
	float range[] = {0, 255};
	const float* histRange[] = {range};
	calcHist(&gray, 1, channel, Mat(), hist, 1, &histSize, histRange, true, false);

	// draw histogram
	// int hist_w = 512;
    // int hist_h = 512;
	// Mat hist2;
    // Mat histImage(hist_h, hist_w, CV_8U, Scalar(0));
    // int bin_w = cvRound((double) hist_w/histSize);
    // normalize(hist, hist2, 0, histImage.rows, NORM_MINMAX, -1, Mat());
	// cout << hist << endl;
	hist.at<float>(0) -= numberOfBackgroundPixels;
	// cout << hist << endl;

    // for(int i = 1; i < histSize; i++)
    // {
    //     line(histImage, Point(bin_w*(i-1), hist_h - cvRound(hist2.at<float>(i-1))) ,
    //                     Point(bin_w*(i), hist_h - cvRound(hist2.at<float>(i))),
    //                     Scalar(255), 2, 8, 0);
    // }
	// imshow("histogram", histImage);

	// first peak
	Mat argmax;
	reduceArgMax(hist, argmax, 0);
	//cout << "first " << argmax << endl;
	float val = hist.at<float>(argmax.at<int>(0));
	hist.at<float>(argmax.at<int>(0)) = 0;

	// second peak
	Mat argmax2;
	reduceArgMax(hist, argmax2, 0);
	//cout << "second " << argmax2 << endl;
	float val2 = hist.at<float>(argmax2.at<int>(0));
	hist.at<float>(argmax2.at<int>(0)) = 0;

	vector<double> mean, stddev;
	Mat hsv;
	cvtColor(img, hsv, COLOR_BGR2HSV);
	meanStdDev(hsv, mean, stddev);
	// cout << val << endl;
	// cout << argmax << endl;
	// cout << val2 << endl;
	// cout << argmax2 << endl;
	// waitKey(0);
	if(argmax.at<int>(0) > 5)
	{
		if(mean[1] < MEAN_WHITE_CHANNEL2 && mean[2] > MEAN_WHITE_CHANNEL3)
			return WHITE_BALL;
	}
	else if(argmax.at<int>(0) < 5)
	{
		if(mean[2] < MEAN_BLACK_CHANNEL3)
			return BLACK_BALL;
	}
	if(val2 > 0.75 * val)
		if(argmax2.at<int>(0) > 5.5 || argmax.at<int>(0) > 5.5)
			return STRIPED_BALL;
	if (val2 < 0.95 * val)
		return SOLID_BALL;
	return PLAYING_FIELD;
}

void detectBalls(const Mat &frame, vector<Ball> &balls, const Vec<Point2f, 4> &tableCorners, const Scalar &colorTable)
{
	// const used during the function
	const int MIN_RADIUS = 6;
	const int MAX_RADIUS = 14;
	const int HOUGH_PARAM1 = 100;
	const int HOUGH_PARAM2 = 9;
	const float INVERSE_ACCUMULATOR_RESOLUTION = 0.1;
	const int MIN_DISTANCE = 25;
	const int NUMBER_CLUSTER_KMEANS = 6;

	// variables
	Mat gray, HSVImg, mask, smooth, kernel;
	Mat frameRect = frame.clone();
	Mat frameCircle = frame.clone();
	Mat cropped = Mat::zeros(frame.size(), CV_8UC1);
	vector<Vec3f> circles;


	cvtColor(frame, HSVImg, COLOR_BGR2HSV);
	inRange(HSVImg, Scalar(colorTable[0], S_CHANNEL_COLOR_THRESHOLD, V_CHANNEL_COLOR_THRESHOLD),
			Scalar(colorTable[1], 255, 255), mask);
	kernel = getStructuringElement(MORPH_ELLIPSE, Size(2, 2));
	morphologyEx(mask, mask, MORPH_DILATE, kernel);
	// imshow("mask dilate", mask);

	// imshow("HSV", HSVImg);
	bilateralFilter(HSVImg, smooth, 3, 75, 75);
	//imshow("smoothed", smooth);

	// needed otherwise exception
	vector<Point> tableCornersInt;
	for(int i = 0; i < 4; i++)
		tableCornersInt.push_back(Point(static_cast<int>(tableCorners[i].x), static_cast<int>(tableCorners[i].y)));

	fillConvexPoly(cropped, tableCornersInt, 255);
	//imshow("Poly", cropped);
	for(int i = 0; i < tableCornersInt.size(); i++)
		circle(cropped, tableCornersInt[i], 15, 0, FILLED, 8, 0);
	kernel = getStructuringElement(MORPH_RECT, Size(2, 2));
	morphologyEx(cropped, cropped, MORPH_ERODE, kernel, Point(-1,-1), 4);
	//imshow("Poly eroded", cropped);

	// mask the smooth image
	for(int i = 0; i < cropped.rows; i++)
		for(int j = 0; j < cropped.cols; j++)
			if(cropped.at<uchar>(i, j) != 255)
				smooth.at<Vec3b>(i,j) = Vec3b(0, 0, 0);

	//clustering
	Mat resClustering;
	kMeansClustering(smooth, resClustering, NUMBER_CLUSTER_KMEANS);
	cvtColor(resClustering, gray, COLOR_BGR2GRAY);
	// imshow("Kmeans", resClustering);
	// imshow("res kmeans gray", gray);

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
	for(size_t i = 0; i < circles.size(); i++ )
	{
		ballFound = true;
		c = circles[i];
	 	center = Point(c[0], c[1]);
	 	radius = c[2];
		if(cropped.at<uchar>(center.y, center.x) == 255
	 		&& cropped.at<uchar>(center.y+radius, center.x) == 255
			&& cropped.at<uchar>(center.y-radius, center.x) == 255
	 		&& cropped.at<uchar>(center.y, center.x+radius) == 255
	 		&& cropped.at<uchar>(center.y, center.x-radius) == 255
	 		&& mask.at<uchar>(center.y, center.x) == 0)
		{
			subImg = frame.colRange(c[0]-radius, c[0]+radius).rowRange(c[1]-radius, c[1]+radius);
			rect = Rect(center.x-c[2], center.y-c[2], 2*c[2], 2*c[2]);
			category = classifyBall(subImg, radius);
			switch(category)
			{
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
						circle(frameCircle, center, radius, Scalar(120,0,120), 1, LINE_AA);
						ballFound = false;
					break;
			}
			if(ballFound)
				balls.push_back(Ball(rect, category));
		}

	}
	imshow("detected circles", frameCircle);
	//imshow("detected rectangles", frameRect);
}
