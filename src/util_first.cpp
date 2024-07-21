//Author: Michele Sprocatti

#include <stdexcept>
#include <opencv2/opencv.hpp>
#include "util.h"
#include "constants.h"
#include "ball.h"
#include "table.h"

using namespace std;
using namespace cv;


/**
 * @brief Helper function which enlarges a rectangle by a given amount of pixels on all sides.
 * @param rect rectangle to enlarge.
 * @param px number of pixels to add to each side.
 */
void enlargeRect(Rect &rect, int px) {
	if (rect.x - px < 0 || rect.y - px < 0) {
		px = std::min(rect.x, rect.y);
	}	// cannot protect from exiting the image on the right and bottom
	rect.x -= px;
	rect.y -= px;
	rect.width += 2 * px;
	rect.height += 2 * px;
}

/**
 * @brief Helper function which shrinks a rectangle by a given amount of pixels on all sides.
 * @param rect rectangle to shrink.
 * @param px number of pixels to remove to each side.
 */
void shrinkRect(Rect &rect, int px) {
	rect.x += px;
	rect.y += px;
	rect.width -= 2 * px;
	rect.height -= 2 * px;
}

/**
 * @brief Gives the equation of the line passing through two points in the form  ax + by + c = 0.
 * @param x1 x-coordinate of the first point
 * @param y1 y-coordinate of the first point
 * @param x2 x-coordinate of the second point
 * @param y2 y-coordinate of the second point
 * @param a output parameter: coefficient a
 * @param b output parameter: coefficient b
 * @param c output parameter: coefficient c
 */
void equationFormula(float x1, float y1, float x2, float y2, float &a, float &b, float &c)
{
	if(x2==x1){
		b = 0;
		a = -1;
		c = x1;
	}
	else{
		b = -1;
		a = (y2 - y1) / (x2 - x1);
		c = -a * x1 - b * y1;
	}
}

/**
 * @brief compute intersection of two lines if there is one.
 * @param line1 first line.
 * @param line2 second line.
 * @param intersection output point that corresponds to the intersection if there is one otherwise
 * 						it contains Point(-1,-1).
 */
void computeIntersection(const Vec3f &line1, const Vec3f &line2, Point2f &intersection)
{
	float a1 = line1[0], b1 = line1[1], c1 = line1[2];
	float a2 = line2[0], b2 = line2[1], c2 = line2[2];
	float det = a1*b2 - a2*b1;
	if(det != 0){
		intersection.x = (b1*c2 - b2*c1) / det;
		intersection.y = (a2*c1 - a1*c2) / det;
	}
	else{
		intersection.x = -1;
		intersection.y = -1;
	}
}

/**
 * @brief calculate the most frequent value of Hue in the input image.
 * Convert the image to HSv representation and then evaluate the histogram for the first channel.
 * @param img input image in BGR format.
 * @return Vec2b the color interval corresponding to the most frequent Hue.
 * @throw invalid_argument if img is empty or if img has less than 3 channels.
 */
Vec2b mostFrequentHueColor(const Mat &img){

	if(img.empty())
		throw invalid_argument("Empty input image");
	if(img.channels() != 3)
		throw invalid_argument("Invalid number of channels for the input image");

	Mat thisImg, hist;
	Mat argmax;

	cvtColor(img, thisImg, COLOR_BGR2HSV);
	const int numberOfBins = 8;
	const float range[] = {0, 179+1};
	const float* histRange[] = {range};

	// Evaluate only H channel
	const int c[] = {0};
	calcHist(&thisImg, 1, c, Mat(), hist, 1, &numberOfBins, histRange);

	// find the argmax
	reduceArgMax(hist, argmax, 0);
	int start = range[1] / numberOfBins * argmax.at<int>(0);
	int diameter = range[1] / numberOfBins;
	return Vec2b(start, start + diameter);
}

/**
 * @brief Create a Output Image object.
 * @param frame input image.
 * @param minimapWithBalls minimap that must be superimposed onto the input image.
 * @param res output image containing the input image with superimposition of the minimap.
 * @throw invalid_argument if frame or minimapWithBalls are empty or if the two images have different number of channels.
 */
void createOutputImage(const Mat &frame, const Mat &minimapWithBalls, Mat &res){

	if(frame.empty() || minimapWithBalls.empty())
		throw invalid_argument("Empty frame or minimap in input");

	if(frame.channels() != minimapWithBalls.channels())
		throw invalid_argument("Different number of channels between the images");

	float scaling_factor = 0.3 * frame.cols / minimapWithBalls.cols;
	float percentage = (frame.rows - scaling_factor * minimapWithBalls.rows) / frame.rows;
	int offset = static_cast<int>(percentage * frame.rows);

	Mat resized;
	res = frame.clone();
	resize(minimapWithBalls, resized, Size(), scaling_factor, scaling_factor, INTER_LINEAR);
	for(int i = 0; i < resized.rows; i++)
		for(int j = 0; j < resized.cols; j++)
			res.at<Vec3b>(i+offset,j) = resized.at<Vec3b>(i,j);
}

/**
 * @brief do the clustering by using only color information on the input image.
 * It maps each pixel in the color space and then do clustering till the termination criteria is reached.
 * To initialize the centers it uses Kmeans++.
 * @param inputImage image to be clustered.
 * @param colors vector containing the different colors for the different clusters,
 * the size of the vector is the number of output clusters.
 * @param clusteredImage output image: original image clustered
 * @throw invalid_argument if the input image is empty or if colors is empty
 * 						   	or if inputImage has a number of channels different from 3.
 */
void kMeansClustering(const Mat &inputImage, const vector<Vec3b> &colors, Mat &clusteredImage){

	if(colors.empty())
		throw invalid_argument("Empty color vector");
	if(inputImage.empty())
		throw invalid_argument("Empty input image");
	if(inputImage.channels() != 3)
		throw invalid_argument("Invalid number of channels for the input image");

	int clusterCount = colors.size();
	Mat samples, labels;
	const int ATTEMPTS = 10;
	samples = Mat(inputImage.total(), 3, CV_32F);
	theRNG().state = 123456789; //fixed random state to have centers used to tune the rest of the program

	int index = 0;
	for(int i = 0; i < inputImage.rows; i++){

		for(int j = 0; j < inputImage.cols; j++){
			samples.at<float>(index, 0) = inputImage.at<Vec3b>(i, j)[0];
			samples.at<float>(index, 1) = inputImage.at<Vec3b>(i, j)[1];
			samples.at<float>(index, 2) = inputImage.at<Vec3b>(i, j)[2];
			index++;
		}
	}

	const TermCriteria CRITERIA = TermCriteria(TermCriteria::EPS, 0, 1.0);
	kmeans(samples, clusterCount, labels, CRITERIA, ATTEMPTS, KMEANS_PP_CENTERS);
	clusteredImage = Mat(inputImage.size(), CV_8UC3);

	int cluster_idx = -1;
	for(int i = 0; i < inputImage.rows; i++){

		for(int j = 0; j < inputImage.cols; j++){

			cluster_idx = labels.at<int>(i * inputImage.cols + j);
			clusteredImage.at<Vec3b>(i, j) = colors[cluster_idx];
		}
	}
}

/**
 * @brief push the elements of the first vector in the right vector according to the category.
 * @param balls input pointer to a vector a balls that needs to be separated.
 * @param white output vector containing the white balls.
 * @param black output vector containing the black balls.
 * @param solid output vector containing the solid balls.
 * @param striped output vector containing the striped balls.
 * @throw invalid_argument if balls is nullptr or if balls point to an empty vector.
 */
void separateResultBalls(Ptr<vector<Ball>> balls, vector<Ball> &white, vector<Ball> &black,
							vector<Ball> &solid, vector<Ball> &striped) {
	if(balls == nullptr)
		throw invalid_argument("Null balls vector");

	if(balls->empty())
		throw invalid_argument("Empty balls vector");

	for(const Ball &ball : *balls) {
		switch(ball.getCategory()) {
			case WHITE_BALL:
				white.push_back(ball);
				break;
			case BLACK_BALL:
				black.push_back(ball);
				break;
			case SOLID_BALL:
				solid.push_back(ball);
				break;
			case STRIPED_BALL:
				striped.push_back(ball);
				break;
			default:
				break;
		}
	}
}

/**
 * @brief push the elements of the first vector in the right vector according to the category.
 * @param gt input vector containing the elements to be separated.
 * @param white output vector containing the white elements.
 * @param black output vector containing the black elements.
 * @param solid output vector containing the solid elements.
 * @param striped output vector containing the striped elements.
 * @throw invalid_argument if gt is empty.
 */
void separateResultGT(const vector<pair<Rect, Category>> &gt,
                      vector<pair<Rect, Category>> &white,
                      vector<pair<Rect, Category>> &black,
                      vector<pair<Rect, Category>> &solid,
                      vector<pair<Rect, Category>> &striped) {
	if (gt.empty())
		throw invalid_argument("Empty ground truth vector");

	for(const pair<Rect, Category> &pair : gt) {
		switch(pair.second) {
			case WHITE_BALL:
				white.push_back(pair);
				break;
			case BLACK_BALL:
				black.push_back(pair);
				break;
			case SOLID_BALL:
				solid.push_back(pair);
				break;
			case STRIPED_BALL:
				striped.push_back(pair);
				break;
			default:
				break;
		}
	}
}

/**
 * @brief Draw the bounding boxes of the balls and the table boundaries on the output image.
 * @param img input image.
 * @param table table containing the boundaries and the balls.
 * @param output output image containing the input image with the bounding boxes.
 */
void drawBoundingBoxes(const Mat &img, Table &table, Mat &output) {
	output = img.clone();
	Scalar border_color = Scalar(0, 255, 255); // color of the borders of the table
	for(const Ball &ball : *table.ballsPtr()) {
		Rect bbox = ball.getBbox();
		if(ball.getVisibility())
			switch (ball.getCategory()){
				case WHITE_BALL:
						rectangle(output, bbox, getColorFromCategory(ball.getCategory()), 1, LINE_AA);
					break;
				case BLACK_BALL:
						rectangle(output, bbox, getColorFromCategory(ball.getCategory()), 1, LINE_AA);
					break;
				case SOLID_BALL:
						rectangle(output, bbox, getColorFromCategory(ball.getCategory()), 1, LINE_AA);
					break;
				case STRIPED_BALL:
						rectangle(output, bbox, getColorFromCategory(ball.getCategory()), 1, LINE_AA);
					break;
				default:
					break;
			}
	}

	Vec<Point2f, 4> corners = table.getBoundaries();
	for(int i = 0; i < 3; i++)
		line(output, corners[i], corners[i+1], border_color, 2, LINE_AA);

	line(output, corners[3], corners[0], border_color, 2, LINE_AA);
}
