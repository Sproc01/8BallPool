//Author: TODO
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

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


Vec2b mostFrequentColor(const Mat &img)
{
	//std::vector<Scalar> rgbColors = {Scalar(255, 0, 0), Scalar(0, 255, 0), Scalar(0, 0, 255)};
	//imshow("Cropped image (hist)", img);
	Mat thisImg;
	cvtColor(img, thisImg, COLOR_BGR2HSV);
	Mat hist;
	int histSize = 8; // number of bins
	float range[] = {0, 179+1}; // range (upper bound is exclusive)
	const float* histRange[] = {range};

	// Evaluate only H channel
	int hCh_idx = 0;
	//const int channels[] = {0, 1, 2};
	const int c[] = {hCh_idx};
	calcHist(&thisImg, 1, c, Mat(), hist, 1, &histSize, histRange);

	// draw histogram
	// int hist_w = 512, hist_h = 400;
	// Mat histImg(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));
	// int bin_w = cvRound((double) hist_w/histSize);
	// normalize(hist, hist, 0, histImg.rows, NORM_MINMAX, -1, Mat());
	// for(int i = 1; i < histSize; i++){
	// 	line(histImg, Point(bin_w*(i-1), hist_h - cvRound(hist.at<float>(i-1))),
	// 		 Point(bin_w*(i), hist_h - cvRound(hist.at<float>(i))),
	// 		 rgbColors[hCh_idx], 2, 8, 0);
	// }

	// find the argmax
	Mat argmax;
	reduceArgMax(hist, argmax, 0);
	int start = range[1] / histSize * argmax.at<int>(0);
	int diameter = (range[1] / histSize);
	//imshow("Histogram", histImg);
	return Vec2b(start, start + diameter);
}


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


void kMeansClustering(const Mat inputImage, Mat& clusteredImage, int clusterCount)
{
    Mat blurred, samples, labels;
    GaussianBlur(inputImage, blurred, Size(15,15), 0, 0);
    int attempts = 5;
    vector<Vec3b> colors;
    for(int i = 0; i < clusterCount; i++)
    {
        colors.push_back(Vec3b(rand()%255, rand()%255, rand()%255));
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
