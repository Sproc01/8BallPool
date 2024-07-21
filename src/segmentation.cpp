// Author: Michele Sprocatti

#include <opencv2/opencv.hpp>
#include <stdexcept>

#include "segmentation.h"
#include "ball.h"
#include "table.h"
#include "minimap.h"
#include "util.h"

using namespace cv;
using namespace std;

/**
 * @brief segment the table in the input image: To do this firstly the image is clustered using kmeans
 * with 2 clusters,Then two masks are created: one using the corners of the table to isolate it
 * and one using the color of it. Then all the information is put together to create the output image.
 * @param frame input image.
 * @param table initialized object containing information about the table in the input image.
 * @param segmented output image where the table is green.
 * @throw invalid_argument if frame is empty or if frame has less than 3 channels.
 */
void segmentTable(const Mat &frame, const Table& table, Mat& segmented){

	if(frame.empty())
		throw invalid_argument("Empty image in input");

	if(frame.channels() != 3)
		throw invalid_argument("Invalid number of channels for the input image");

	Mat polyImage = Mat::zeros(frame.size(), CV_8UC1);
	vector<Point> tableCornersInt;

	// table properties
	Vec2b colorTable = table.getColorRange();
	Vec<Point2f, 4> tableCorners = table.getBoundaries();

	// needed otherwise error
	for(int i = 0; i < 4; i++)
		tableCornersInt.push_back(Point(static_cast<int>(tableCorners[i].x), static_cast<int>(tableCorners[i].y)));

	fillConvexPoly(polyImage, tableCornersInt, 255);
	//imshow("poly", polyImage);

	Mat clustered, HSVimg, mask;
	cvtColor(frame, HSVimg, COLOR_BGR2HSV);

	inRange(HSVimg, Scalar(colorTable[0], S_CHANNEL_COLOR_THRESHOLD, V_CHANNEL_COLOR_THRESHOLD),
				Scalar(colorTable[1], 255, 255), mask);
	//imshow("mask", mask);
	const vector<Vec3b> COLORS = {
		Vec3b(0, 0, 0),
		Vec3b(255, 255, 255)
	};
	kMeansClustering(frame, COLORS, clustered);
	//imshow("cluster", clustered);
	Vec3b color;// = clustered.at<Vec3b>(frame.rows/2, frame.cols/2);
	for(int i = frame.rows/4; i < 3*frame.rows/4; i++)
		for(int j = frame.cols/4; j < 3*frame.cols/4; j++)
			if(polyImage.at<uchar>(i,j) == 255 && mask.at<uchar>(i,j) == 255){
				color = clustered.at<Vec3b>(i, j);
				break;
			}

	segmented = Mat::zeros(frame.size(), CV_8UC3);
	for(int i = 0; i < segmented.rows; i++){
		for(int j = 0; j < segmented.cols; j++){
			if(polyImage.at<uchar>(i, j) == 255 && (clustered.at<Vec3b>(i, j) == color || mask.at<uchar>(i,j) == 255))
				segmented.at<Vec3b>(i, j) = PLAYING_FIELD_BGR_COLOR;
			else
				segmented.at<Vec3b>(i, j) = BACKGROUND_BGR_COLOR;
		}
	}
	//imshow("segmented", segmented);
}

/**
 * @brief segment the input image by highlight the balls. Using the information from each specific ball colors
 * the corresponding pixels with the correct color.
 * @param frame input image.
 * @param balls vector of the balls in the image.
 * @param segmented output image where each category of the ball corresponds to a different color.
 * @throw invalid_argument if frame is empty, if frame has less than 3 channels, if balls is nullptr, if balls point to an empty vector.
 */
void segmentBalls(const Mat &frame, Ptr<vector<Ball>> balls, Mat& segmented){

	if(balls == nullptr)
		throw invalid_argument("Null pointer");

	if(frame.empty())
		throw invalid_argument("Empty image in input");

	if(frame.channels() != 3)
		throw invalid_argument("Invalid number of channels for the input image");

	if(balls->empty())
		throw invalid_argument("Empty vector of balls");

	Scalar c = Scalar(0, 0, 0);
	for (const Ball &ball : *balls){

		if(ball.getVisibility())
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
			float radius = b.width / 2.0;
			Point center = Point(b.tl().x + radius, b.tl().y + radius);
			circle(segmented, center, radius, c, -1);
		}

	}
}
