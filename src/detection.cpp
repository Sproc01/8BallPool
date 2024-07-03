// Author: Michele Sprocatti

#include <opencv2/opencv.hpp>
#include <iostream>
#include <opencv2/features2d.hpp>

#include "table.h"
#include "ball.h"
#include "detection.h"
#include "util.h"
#include "minimapConstants.h"
#include "segmentation.h"

using namespace cv;
using namespace std;

Mat otsuClustering(cv::Mat inputImage)
{
    Mat gray, blurred, result;
    GaussianBlur(inputImage, blurred, Size(5,5), 0);
    cvtColor(blurred, gray, COLOR_BGR2GRAY);
    threshold(gray, result, 0, 255, THRESH_OTSU);
    return result;
}

Mat watershedClustering(const Mat inputImage)
{
    Mat blur, gray, dist, dist_8u, markers, drawing, result;
    GaussianBlur(inputImage, blur, Size(5,5), 0);
    cvtColor(blur, gray, COLOR_BGR2GRAY);
    threshold(gray, gray, 40, 255, THRESH_BINARY | THRESH_OTSU);
    distanceTransform(gray, dist, DIST_L2, 3);
    normalize(dist, dist, 0, 1.0, NORM_MINMAX);
    //imshow("Distance Transform Image", dist);
    threshold(dist, dist, 0.5, 1.0, THRESH_BINARY);
    Mat kernel1 = Mat::ones(3, 3, CV_8U);
    dilate(dist, dist, kernel1);
    //imshow("Peaks", dist);
    dist.convertTo(dist_8u, CV_8U);
    vector<vector<Point>> contours;
    findContours(dist_8u, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    markers = Mat::zeros(dist.size(), CV_32SC1);
	//Mat res = Mat::zeros(dist.size(), CV_8UC3);
    for(int i = 0; i < contours.size(); i++)
    {
		drawContours(markers, contours, i, Scalar::all(i+1), -1);
		//drawContours(res, contours, i, Vec3b(255,255,0), -1);
	}
    circle(markers, Point(5,5), 3, Scalar(255,255,255), -1);
    //imshow("Contours", drawing);
    watershed(inputImage, markers);
    result = Mat::zeros(inputImage.size(), CV_8UC3);
    vector<Vec3b> colors;
    for(int i = 0; i<contours.size(); i++)
    {
        colors.push_back(Vec3b(rand()%255, rand()%255, rand()%255));
    }
    for (int i = 0; i < markers.rows; i++)
    {
        for (int j = 0; j < markers.cols; j++)
        {
            int index = markers.at<int>(i, j);
            if (index > 0 && index <= contours.size())
                result.at<Vec3b>(i, j) = colors[index-1];
            else
                result.at<Vec3b>(i, j) = Vec3b(0, 0, 0);
        }
    }
    return result;
}

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


void detectBalls(const Mat &frame, vector<Ball> &balls, const Vec<Point2f, 4> &tableCorners, const Scalar &colorTable)
{
	// const used during the function
	const int MIN_RADIUS = 5;
	const int MAX_RADIUS = 15;
	const int HOUGH_PARAM1 = 100;
	const int HOUGH_PARAM2 = 11;
	const float ACCUMULATOR_RESOLUTION = 1;
	const int MIN_DISTANCE = 20;

	// const filters
	const int DIM_STRUCTURING_ELEMENT = 15;
	// const int DIM_BILATERAL_FILTER = 5;
	// const int SIGMA_COLOR = 10;
	// const int SIGMA_SPACE = 50;

	// const for the ball
	const int MEAN_WHITE_CHANNEL2 = 110;
	const int MEAN_WHITE_CHANNEL3 = 210;
	const int MEAN_BLACK_CHANNEL3 = 90;
	const int STD_DEV_BLACK = 60;
	const int STD_DEV_SOLID = 20;
	const int STD_DEV_STRIPED = 20;

	// variables
	Mat gray, imgBorder, HSVImg, mask;
	cvtColor(frame, HSVImg, COLOR_BGR2HSV);
	inRange(HSVImg, Scalar(colorTable[0], S_CHANNEL_COLOR_THRESHOLD, V_CHANNEL_COLOR_THRESHOLD),
			Scalar(colorTable[1], 255, 255), mask);
	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(DIM_STRUCTURING_ELEMENT, DIM_STRUCTURING_ELEMENT));
	morphologyEx(mask, mask, MORPH_ERODE, kernel);

	//Mat dst;
	//bilateralFilter(frame, dst, DIM_BILATERAL_FILTER, SIGMA_COLOR, SIGMA_SPACE);
	cvtColor(frame, gray, COLOR_BGR2GRAY);

	vector<Vec3f> circles;
	vector<Rect> boundRect;
	Mat frameRect = frame.clone();
	Mat frameCircle = frame.clone();

	Mat cropped = Mat::zeros(gray.size(), CV_8UC1);

	// needed otherwise exception
	vector<Point> tableCornersInt;
	for(int i = 0; i < 4; i++)
	{
		tableCornersInt.push_back(Point(static_cast<int>(tableCorners[i].x), static_cast<int>(tableCorners[i].y)));
	}
	fillConvexPoly(cropped, tableCornersInt, 255);
	Mat water = frame.clone();

	// mask the image
	for(int i = 0; i < cropped.rows; i++)
	{
		for(int j = 0; j < cropped.cols; j++)
		{
			if(cropped.at<uchar>(i, j) != 255)
			{
				gray.at<uchar>(i, j) = 0;
				water.at<Vec3b>(i,j) = Vec3b(0,0,0);
			}

			if(mask.at<uchar>(i,j) == 255)
			{
				gray.at<uchar>(i,j) = 0;
				//water.at<Vec3b>(i,j) = Vec3b(0,0,0);
			}
		}
	}
	// Mat res; // = watershedClustering(frame);
	// // imshow("watershed", res);
	// imshow("input",water);
	// //Mat res = watershedClustering(water);
	// // imshow("watershed", res);
	// kMeansClustering(water, res, 3);
	// imshow("kmeans", res);
	// // res = otsuClustering(water);
	// // imshow("otsu", res);
	// // vector<vector<Point>> contours;
	// cvtColor(res, gray, COLOR_BGR2GRAY);

	// Hough transform
	HoughCircles(gray, circles, HOUGH_GRADIENT,
					ACCUMULATOR_RESOLUTION, MIN_DISTANCE, HOUGH_PARAM1, HOUGH_PARAM2, MIN_RADIUS, MAX_RADIUS);
	Mat subImg;
	vector<double> mean, stddev;
	Category category;
	double meanRadius = 0; 	// mean of radius of the balls
	Point center;
	int radius;
	Vec3i c;
	for(size_t i = 0; i < circles.size(); i++)
	{
		c = circles[i];
		radius = c[2];
		center = Point(c[0], c[1]);
		if(cropped.at<uchar>(center.y, center.x) == 255
			&& cropped.at<uchar>(center.y+radius, center.x) == 255
			&& cropped.at<uchar>(center.y-radius, center.x) == 255
			&& cropped.at<uchar>(center.y, center.x+radius) == 255
			&& cropped.at<uchar>(center.y, center.x-radius) == 255
			&& mask.at<uchar>(center.y, center.x) == 0)
			meanRadius += circles[i][2];
	}
	meanRadius /= circles.size();
	//cout << "Mean radius: " << meanRadius << endl;
	Rect rect;
	//bool ballFound;
	for(size_t i = 0; i < circles.size(); i++ )
	{
		//ballFound = true;
		c = circles[i];
		center = Point(c[0], c[1]);
		radius = c[2];
		// Inside the cropped image, not too small, not too big, not selected by the table mask
		if(radius > 0.5 * meanRadius && radius < 1.8 * meanRadius
		 	&& cropped.at<uchar>(center.y, center.x) == 255
			&& cropped.at<uchar>(center.y+radius, center.x) == 255
			&& cropped.at<uchar>(center.y-radius, center.x) == 255
			&& cropped.at<uchar>(center.y, center.x+radius) == 255
			&& cropped.at<uchar>(center.y, center.x-radius) == 255
			&& mask.at<uchar>(center.y, center.x) == 0)
		{
			// rect and circle
			rect = Rect(center.x-c[2], center.y-c[2], 2*c[2], 2*c[2]);
			boundRect.push_back(rect);
			int halfRad = static_cast<int>(radius/2);
			subImg = HSVImg.colRange(c[0]-halfRad, c[0]+halfRad).rowRange(c[1]-halfRad, c[1]+halfRad);
			// Vec2b color = mostFrequentColor(frame.colRange(c[0]-halfRad, c[0]+halfRad).rowRange(c[1]-halfRad, c[1]+halfRad));
			// cout << (int)color[0] << "....." << (int)color[1] << endl;
			// only for debug
//			for(int j = 0; j < subImg.rows; j++)
//			{
//				for(int k = 0; k < subImg.cols; k++)
//				{
//					cout << subImg.at<Vec3b>(j,k) << endl;
//				}
//			}

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
			{ // striped blue
				category = Category::STRIPED_BALL;
				circle(frameCircle, center, radius, STRIPED_BGR_COLOR, 1, LINE_AA);
				rectangle(frameRect, rect, STRIPED_BGR_COLOR, 1, LINE_AA);
			}
			// else
			// {
			// 	circle(frameCircle, center, radius, Scalar(255, 255, 255), 1, LINE_AA);
			// 	ballFound = false;
			// }
			// if(ballFound)
			// {
				//Ball ball(rect, category);
				balls.push_back(Ball(rect, category));
			// }
		}
	}
	imshow("detected circles", frameCircle);
	imshow("detected rectangles", frameRect);

	//waitKey(0);
}
