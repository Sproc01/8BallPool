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
			//TODO: throw error if no correct category is found
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

// Gives the equation of the line passing through two points in the form  ax + by + c = 0
void equationFormula(float x1, float y1, float x2, float y2, float &a, float &b, float &c)
{
	if(x2==x1)
	{
		b = 0;
		a = -1;
		c = x1;
	}
	else
	{
		b = -1;
		a = (y2 - y1) / (x2 - x1);
		c = -a * x1 - b * y1;
	}
}

// compute intersection of two lines if there is one
void computeIntersection(const Vec3f &line1, const Vec3f &line2, Point2f &intersection)
{
	float a1 = line1[0], b1 = line1[1], c1 = line1[2];
	float a2 = line2[0], b2 = line2[1], c2 = line2[2];
	float det = a1*b2 - a2*b1;
	if(det != 0)
	{
		intersection.x = (b1*c2 - b2*c1) / det;
		intersection.y = (a2*c1 - a1*c2) / det;
	}
	else
	{
		intersection.x = -1;
		intersection.y = -1;
	}
}

// calculate the most frequent color in the image
Vec2b mostFrequentHueColor(const Mat &img)
{
	Mat thisImg, hist;
	cvtColor(img, thisImg, COLOR_BGR2HSV);
	const int histSize = 8; // number of bins
	const float range[] = {0, 179+1}; // range (upper bound is exclusive)
	const float* histRange[] = {range};

	// Evaluate only H channel
	const int c[] = {0};
	calcHist(&thisImg, 1, c, Mat(), hist, 1, &histSize, histRange);

	// find the argmax
	Mat argmax;
	reduceArgMax(hist, argmax, 0);
	int start = range[1] / histSize * argmax.at<int>(0);
	int diameter = (range[1] / histSize);
	return Vec2b(start, start + diameter);
}

// create the output image with frame and minimap
void createOutputImage(const Mat& frame, const Mat& minimap_with_balls, Mat& res)
{
	const int offset = 408;
	const float scaling_factor = 0.3;

	Mat resized;
	res = frame.clone();
	resize(minimap_with_balls, resized, Size(), scaling_factor, scaling_factor, INTER_LINEAR);
	for(int i = 0; i < resized.rows; i++)
		for(int j = 0; j < resized.cols; j++)
			res.at<Vec3b>(i+offset,j) = resized.at<Vec3b>(i,j);
}

void kMeansClustering(const Mat inputImage, int clusterCount, Mat& clusteredImage)
{
    Mat samples, labels;
	int attempts = 10;
    vector<Vec3b> colors;
	for(int i = 0; i < clusterCount; i++)
        colors.push_back(Vec3b(rand()%255, rand()%255, rand()%255));
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

    TermCriteria criteria = TermCriteria(TermCriteria::EPS, 0, 1.0);
    kmeans(samples, clusterCount, labels, criteria, attempts, KMEANS_PP_CENTERS);
    clusteredImage = Mat(inputImage.size(), CV_8UC3);

	int cluster_idx = -1;
    for(int i = 0; i < inputImage.rows; i++)
    {
        for(int j = 0; j < inputImage.cols; j++)
        {
            cluster_idx = labels.at<int>(i * inputImage.cols + j);
            clusteredImage.at<Vec3b>(i, j) = colors[cluster_idx];
        }
    }
}
