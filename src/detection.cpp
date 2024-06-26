// Author: Michele Sprocatti

#include <opencv2/opencv.hpp>
#include <iostream>
#include <opencv2/features2d.hpp>

#include "table.h"
#include "ball.h"
#include "detection.h"
#include "util.h"
#include "minimapConstants.h"

using namespace cv;
using namespace std;

// S>50, V>90 to be a color and not black or white
const int S_CHANNEL_COLOR_THRESHOLD = 50;
const int V_CHANNEL_COLOR_THRESHOLD = 90;


void detectTable(const Mat &frame, Vec<Point2f, 4> &corners, Vec2b &colorRange)
{
	// const used during the function
	const int DIM_STRUCTURING_ELEMENT = 27;
	const int DIM_GAUSSIAN_KERNEL = 17;
	const int CANNY_THRESHOLD1 = 95;
	const int CANNY_THRESHOLD2 = 110;
	const int THRESHOLD_HOUGH = 130;
	const int MAX_LINE_GAP = 15;
	const int MIN_LINE_LENGTH = 120;
	const int CLOSE_POINT_THRESHOLD = 30;

	// variables
	Mat imgGray, imgLine, imgBorder, thisImg, mask, kernel;
	vector<Vec4i> lines;
	int rowsover4 = frame.rows/4, colsover4 = frame.cols/4;
	Scalar line_color = Scalar(0, 0, 255);
	vector<Point2f> intersections;
	vector<Vec3f> coefficients;

	// get the color range
	colorRange = mostFrequentColor(frame.rowRange(rowsover4, 3*rowsover4).colRange(colsover4, 3*colsover4));

	// mask the image
	cvtColor(frame, thisImg, COLOR_BGR2HSV);
	inRange(thisImg, Scalar(colorRange[0], S_CHANNEL_COLOR_THRESHOLD, V_CHANNEL_COLOR_THRESHOLD),
				Scalar(colorRange[1], 255, 255), mask);
	//imshow("Mask", mask);

	// morphological operations
	kernel = getStructuringElement(MORPH_ELLIPSE, Size(DIM_STRUCTURING_ELEMENT, DIM_STRUCTURING_ELEMENT));
	morphologyEx(mask, mask, MORPH_CLOSE, kernel);
	//imshow("Morphology", mask);

	// edge detection
	GaussianBlur(mask, mask, Size(DIM_GAUSSIAN_KERNEL, DIM_GAUSSIAN_KERNEL), 0);
	//imshow("Gaussian Blur", mask);
	Canny(mask, imgBorder, CANNY_THRESHOLD1, CANNY_THRESHOLD2);
	//imshow("Canny Result", imgBorder);

	// Hough transform
	imgLine = frame.clone();
	HoughLinesP(imgBorder, lines, 1, CV_PI/180, THRESHOLD_HOUGH, MIN_LINE_LENGTH, MAX_LINE_GAP);

	// lines drawing
	Point pt1, pt2;
	float aLine, bLine, cLine;
	for(size_t i = 0; i < lines.size(); i++)
	{
		pt1.x = lines[i][0];
		pt1.y = lines[i][1];
		pt2.x = lines[i][2];
		pt2.y = lines[i][3];
		line(imgLine, pt1, pt2, line_color, 2, LINE_AA);
		equationFormula(pt1.x, pt1.y, pt2.x, pt2.y, aLine, bLine, cLine);
		coefficients.push_back(Vec3f(aLine, bLine, cLine));
	}

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

	// remove intersections that are too close
	vector<Point2f> intersectionsGood;
	sort(intersections.begin(), intersections.end(), [frame](Point a, Point b) -> bool
	{
		Point center = Point(frame.cols/2, frame.rows/2);
		return norm(a) < norm(b);
	});
	// TODO remove auto
	auto end2 = unique(intersections.begin(), intersections.end(), [&CLOSE_POINT_THRESHOLD](Point a, Point b) -> bool
	{
		return abs(a.x - b.x) < CLOSE_POINT_THRESHOLD && abs(a.y - b.y) < CLOSE_POINT_THRESHOLD;
	});
	// TODO remove auto
	for(auto it = intersections.begin(); it != end2; it++)
	{
		intersectionsGood.push_back(*it);
	}

	sort(intersectionsGood.begin(), intersectionsGood.end(), [frame](Point a, Point b) -> bool
	{
		Point center = Point(frame.cols/2, frame.rows/2);
		return norm(a - center) < norm(b - center);
	});
	// clockwise order
	sort(intersectionsGood.begin(), intersectionsGood.end(), [frame](Point a, Point b) -> bool
	{
		Point center = Point(frame.cols/2, frame.rows/2);
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
	for(size_t i = 0; i < 4; i++)
	{
		circle(imgLine, intersectionsGood[i], 10, colors[i], -1);
	}

	//cout << intersectionsGood << endl;
	for(size_t i = 0; i < 4; i++)
	{
		corners[i] = intersectionsGood[i];
	}
	imshow("Line", imgLine);
	//waitKey(0);
}


void detectBalls(const Mat &frame, vector<Ball> &balls, const Vec<Point2f, 4> &tableCorners, const Scalar &colorTable)
{
	// const used during the function
	const int MIN_RADIUS = 5;
	const int MAX_RADIUS = 15;
	const int HOUGH_PARAM1 = 120;
	const int HOUGH_PARAM2 = 5;
	const int ACCUMULATOR_RESOLUTION = 1;
	const int MIN_DISTANCE = 30;

	// const for the ball
	const int MEAN_WHITE_CHANNEL2 = 90;
	const int MEAN_WHITE_CHANNEL3 = 220;
	const int MEAN_BLACK_CHANNEL3 = 100;
	const int STD_DEV_BLACK = 70;
	const int STD_DEV_SOLID = 20;
	const int STD_DEV_STRIPED = 20;


	// variables
	Mat gray, gradX, gradY, abs_grad_x, abs_grad_y, grad, imgBorder, HSVImg;
	Mat mask;
	cvtColor(frame, HSVImg, COLOR_BGR2HSV);
	inRange(HSVImg, Scalar(colorTable[0], S_CHANNEL_COLOR_THRESHOLD, V_CHANNEL_COLOR_THRESHOLD),
			Scalar(colorTable[1], 255, 255), gray);
	vector<Vec3f> circles;
	vector<Rect> boundRect;
	Mat frameRect = frame.clone();
	Mat frameCircle = frame.clone();

	int maxY = max(tableCorners[0].y, tableCorners[3].y);
	int minY = min(tableCorners[1].y, tableCorners[2].y);
	int maxX = max(tableCorners[2].x, tableCorners[3].x);
	int minX = min(tableCorners[0].x, tableCorners[1].x);
	Mat cropped;
	cropped = Mat::zeros(gray.size(), CV_8UC1);

	// needed otherwise exception
	vector<Point> tableCornersInt;
	for(int i = 0; i < 4; i++)
	{
		tableCornersInt.push_back(Point(static_cast<int>(tableCorners[i].x), static_cast<int>(tableCorners[i].y)));
	}
	fillConvexPoly(cropped, tableCornersInt, 255);

	// mask the image
	for(int i = 0; i < cropped.rows; i++)
	{
		for(int j = 0; j < cropped.cols; j++)
		{
			if(cropped.at<uchar>(i, j) != 255)
			{
				gray.at<uchar>(i, j) = 0;
			}
		}
	}
	imshow("Cropped image", gray);

	// Hough transform
	HoughCircles(gray, circles, HOUGH_GRADIENT,
					ACCUMULATOR_RESOLUTION, MIN_DISTANCE, HOUGH_PARAM1, HOUGH_PARAM2, MIN_RADIUS, MAX_RADIUS);
	Mat subImg;
	vector<double> mean, stddev;
	Category category;
	double meanRadius = 0; 	// mean of radius of the balls
	for(size_t i = 0; i < circles.size(); i++)
	{
		//cout << circles[i][2] << endl;
		meanRadius += circles[i][2];
	}
	meanRadius /= circles.size();
	//cout << "Mean radius: " << meanRadius << endl;
	Rect rect;
	for(size_t i = 0; i < circles.size(); i++ )
	{
		Vec3i c = circles[i];
		Point center = Point(c[0], c[1]);
		int radius = c[2];
		// Inside the cropped image, not too small, not too big, not on the border
		if(radius < 1.5 * meanRadius && radius > 0.5 * meanRadius
			&& cropped.at<uchar>(center.y, center.x) == 255
			&& cropped.at<uchar>(center.y+radius, center.x) == 255
			&& cropped.at<uchar>(center.y-radius, center.x) == 255
			&& cropped.at<uchar>(center.y, center.x+radius) == 255
			&& cropped.at<uchar>(center.y, center.x-radius) == 255
			&& gray.at<uchar>(center.y, center.x) == 0)
		{
			// rect and circle
			rect = Rect(center.x-c[2], center.y-c[2], 2*c[2], 2*c[2]);
			boundRect.push_back(rect);
			int halfRad = static_cast<int>(c[2]/2);
			subImg = HSVImg.colRange(c[0]-halfRad, c[0]+halfRad).rowRange(c[1]-halfRad, c[1]+halfRad);

			// only for debug
			for(int j = 0; j < subImg.rows; j++)
			{
				for(int k = 0; k < subImg.cols; k++)
				{
					cout << subImg.at<Vec3b>(j,k) << endl;
				}
			}

			// check the color of the ball
			meanStdDev(subImg, mean, stddev);
			if(mean[1] < MEAN_WHITE_CHANNEL2 && mean[2] > MEAN_WHITE_CHANNEL3)
			{ // white ball
				category = Category::WHITE_BALL;
				circle(frameCircle, center, radius, WHITE_BGR_COLOR, 1, LINE_AA);
				rectangle(frameRect, rect, WHITE_BGR_COLOR, 1, LINE_AA);
			}
			else if(mean[2] < MEAN_BLACK_CHANNEL3 && stddev[2] < STD_DEV_BLACK)
			{ // black ball
				category = Category::BLACK_BALL;
				circle(frameCircle, center, radius, BLACK_BGR_COLOR, 1, LINE_AA);
				rectangle(frameRect, rect, BLACK_BGR_COLOR, 1, LINE_AA);
			}
			else if(stddev[0] < STD_DEV_SOLID)
			{ // solid red
				category = Category::SOLID_BALL;
				circle(frameCircle, center, radius, SOLID_BGR_COLOR, 1, LINE_AA);
				rectangle(frameRect, rect, SOLID_BGR_COLOR, 1, LINE_AA);
			}
			else if(stddev[0] > STD_DEV_STRIPED)
			{ // striped green
				category = Category::STRIPED_BALL;
				circle(frameCircle, center, radius, STRIPED_BGR_COLOR, 1, LINE_AA);
				rectangle(frameRect, rect, STRIPED_BGR_COLOR, 1, LINE_AA);
			}
			Ball ball(rect, category);
			balls.push_back(ball);
		}
	}
	imshow("detected circles", frameCircle);
	imshow("detected rectangles", frameRect);

	//waitKey(0);
}
