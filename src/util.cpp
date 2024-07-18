//Author: TODO
#include <opencv2/opencv.hpp>
#include "util.h"
#include "minimapConstants.h"

using namespace std;
using namespace cv;

//return center between two points
Point2f getCenter(Point2f p1, Point2f p2) {
    Point2f center;
    int dx_half = abs(p1.x - p2.x)/2;
    int dy_half = abs(p1.y - p2.y)/2;
    if(p1.x > p2.x)
        dx_half *= -1;
    if(p1.y > p2.y)
        dy_half *= -1;
    center = Point2f(p1.x + dx_half, p1.y + dy_half);
    return center;
}

//return the color of a specific category
Vec3b getColorFromCategory(Category category) {
	switch (category) {
		case BLACK_BALL: return BLACK_BGR_COLOR; break;
		case WHITE_BALL: return WHITE_BGR_COLOR; break;
		case STRIPED_BALL: return STRIPED_BGR_COLOR; break;
		case SOLID_BALL: return SOLID_BGR_COLOR; break;
		case BACKGROUND: return BACKGROUND_BGR_COLOR; break;
		case PLAYING_FIELD: return BACKGROUND_BGR_COLOR; break;
		default:
			throw invalid_argument("Not correct category");
		break;
	}
}

//rotate the corners of the table clockwise
void rotateCornersClockwise(Vec<Point2f, 4> &corners) {
	Vec<Point2f, 4> img_vertices_temp = corners;
	for(int i = 0; i < 4; i++) {
		if(i+1 < 4) {
			corners[i].x = img_vertices_temp[i+1].x;
			corners[i].y = img_vertices_temp[i+1].y;
		}
		else {
			corners[i].x = img_vertices_temp[0].x;
			corners[i].y = img_vertices_temp[0].y;
		}
	}
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
 * @throw invalid_argument if img is empty.
 */
Vec2b mostFrequentHueColor(const Mat &img){

	if(img.empty())
		throw invalid_argument("Empty input image");

	Mat thisImg, hist;
	Mat argmax;

	cvtColor(img, thisImg, COLOR_BGR2HSV);
	const int numberOfBins = 8;
	const float range[] = {0, 179+1}; // TODO write 180 instead of 179+1
	const float* histRange[] = {range};

	// Evaluate only H channel
	const int c[] = {0};
	calcHist(&thisImg, 1, c, Mat(), hist, 1, &numberOfBins, histRange);

	// find the argmax
	reduceArgMax(hist, argmax, 0);
	int start = range[1] / numberOfBins * argmax.at<int>(0);
	int diameter = (range[1] / numberOfBins);
	return Vec2b(start, start + diameter);
}

/**
 * @brief Create a Output Image object.
 * @param frame input image.
 * @param minimap_with_balls minimap that must be superimposed onto the input image.
 * @param res output image containing the input image with superimposition of the minimap.
 * @throw invalid_argument if frame or minimap_with_balls are empty.
 */
void createOutputImage(const Mat& frame, const Mat& minimap_with_balls, Mat& res){

	if(frame.empty() || minimap_with_balls.empty())
		throw invalid_argument("Empty frame or minimap in input");

	float scaling_factor = 0.3 * frame.cols / minimap_with_balls.cols;
	float percentage = (frame.rows - scaling_factor * minimap_with_balls.rows) / frame.rows;
	int offset = static_cast<int>(percentage * frame.rows);

	Mat resized;
	res = frame.clone();
	resize(minimap_with_balls, resized, Size(), scaling_factor, scaling_factor, INTER_LINEAR);
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
void kMeansClustering(const Mat inputImage, const vector<Vec3b> &colors, Mat& clusteredImage){

	if(colors.size() == 0)
		throw invalid_argument("Empty color vector");
	if(inputImage.empty())
		throw invalid_argument("Empty input image");
	if(inputImage.channels() != 3)
		throw invalid_argument("Invalid number of channels for the input image");

	int clusterCount = colors.size();
    Mat samples, labels;
	int attempts = 10;
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

    TermCriteria criteria = TermCriteria(TermCriteria::EPS, 0, 1.0);
    kmeans(samples, clusterCount, labels, criteria, attempts, KMEANS_PP_CENTERS);
    clusteredImage = Mat(inputImage.size(), CV_8UC3);

	int cluster_idx = -1;
    for(int i = 0; i < inputImage.rows; i++){

        for(int j = 0; j < inputImage.cols; j++){

            cluster_idx = labels.at<int>(i * inputImage.cols + j);
            clusteredImage.at<Vec3b>(i, j) = colors[cluster_idx];
        }
    }
}
