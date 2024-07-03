#include <opencv2/opencv.hpp>
#include <iostream>

#include "segmentation.h"
#include "ball.h"
#include "table.h"
#include "detection.h"
#include "minimapConstants.h"

using namespace cv;
using namespace std;

void kMeansClustering(const Mat inputImage, Mat& clusteredImage, int clusterCount)
{
    Mat blurred, samples, labels;
    GaussianBlur(inputImage, blurred, Size(15,15), 0, 0);
    int attempts = 5;
    vector<Vec3b> colors;
    for(int i = 0; i < clusterCount; i++)
    {
        colors.push_back(Vec3b(rand()%256, rand()%256, rand()%256));
    }
    samples = Mat(inputImage.total(), 3, CV_32F);
    int index = 0;
    for(int i = 0; i < inputImage.rows; i++)
    {
        for(int j = 0; j < inputImage.cols; j++)
        {
            samples.at<float>(index, 0) = inputImage.at<Vec3b>(i, j)[0];
            samples.at<float>(index, 1) = inputImage.at<Vec3b>(i, j)[1];
            samples.at<float>(index, 2) = inputImage.at<Vec3b>(i, j)[2];
            index++;
        }
    }
    TermCriteria criteria = TermCriteria(TermCriteria::MAX_ITER|TermCriteria::EPS, 10, 0.01);
    kmeans(samples, clusterCount, labels, criteria, attempts, KMEANS_PP_CENTERS);
    clusteredImage = Mat(inputImage.size(), CV_8UC3);
    for(int i = 0; i < inputImage.rows; i++)
    {
        for(int j = 0; j < inputImage.cols; j++)
        {
            int cluster_idx = labels.at<int>(i * inputImage.cols + j);
            clusteredImage.at<Vec3b>(i, j) = colors[cluster_idx];
        }
    }
}

void segmentTable(const Mat &frame, const Vec<Point2f, 4> &tableCorners, const Scalar &colorTable, Mat& segmented)
{
	Mat clustered;
	kMeansClustering(frame, clustered);
	segmented = frame.clone();
	// Mat HSVimage;
	// Mat mask;
	// at the center color of table in the dataset
	uchar color = clustered.at<uchar>(clustered.rows/2, clustered.cols/2);
	Mat polyImage = Mat::zeros(frame.size(), CV_8UC1);
	//cvtColor(frame, HSVimage, COLOR_BGR2HSV);
	// inRange(HSVimage,  Scalar(colorTable[0], S_CHANNEL_COLOR_THRESHOLD, V_CHANNEL_COLOR_THRESHOLD),
	// 		Scalar(colorTable[1], 255, 255), mask);
	vector<Point> tableCornersInt;
	for(int i = 0; i < 4; i++)
	{
		tableCornersInt.push_back(Point(static_cast<int>(tableCorners[i].x), static_cast<int>(tableCorners[i].y)));
	}
	fillConvexPoly(polyImage, tableCornersInt, 255);
	for(int i = 0; i < segmented.rows; i++)
	{
		for(int j = 0; j < segmented.cols; j++)
		{
			if(polyImage.at<uchar>(i, j) == 255 && clustered.at<uchar>(i, j) == color)
			{
				segmented.at<Vec3b>(i, j) = PLAYING_FIELD_BGR_COLOR;
			}
			else
			{
				segmented.at<Vec3b>(i, j) = BACKGROUND_BGR_COLOR;
			}
		}
	}
	//imshow("segmented", segmented);
}

void segmentBalls(const Mat &frame, const vector<Ball> &balls, Mat& segmented)
{
	float radius;
	Point center;
	//segmented = frame.clone();
	Scalar c = Scalar(0, 0, 0);
	for (const Ball &ball : balls)
	{
		if(ball.getCategory() == Category::BLACK_BALL)
			c = BLACK_BGR_COLOR;
		else if(ball.getCategory() == Category::WHITE_BALL)
			c = WHITE_BGR_COLOR;
		else if(ball.getCategory() == Category::SOLID_BALL)
			c = SOLID_BGR_COLOR;
		else if(ball.getCategory() == Category::STRIPED_BALL)
			c = STRIPED_BGR_COLOR;
		Rect b = ball.getBbox();
		radius = b.width / 2;
		center = Point(b.tl().x + radius, b.tl().y + radius);
		circle(segmented, center, radius, c, -1);
	}
}
