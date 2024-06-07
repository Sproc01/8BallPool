// Author: Michele Sprocatti

#include <opencv2/opencv.hpp>
#include "detection.h"
#include <iostream>
#include <opencv2/features2d.hpp>
#include "table.h"
#include "ball.h"

using namespace cv;
using namespace std;

// S>50, V>90 to be a color and not black or white
const int S_CHANNEL_COLOR_THRESHOLD = 50;
const int V_CHANNEL_COLOR_THRESHOLD = 90;

// Gives the equation of the line passing through two points in the form  ax + by + c = 0
void equationFormula(float x1, float y1, float x2, float y2, float &a, float &b, float &c)
{
    //cout << "x1: " << x1 << " y1: " << y1 << " x2: " << x2 << " y2: " << y2 << endl;
    if(x2==x1)
    {
        b = 0;
        a = -1;
        c = x1;
        // cout << "a: " << a << " b: " << b << " c:" << c << endl;
        // cout << endl;
    }
    else
    {
        b = -1;
        a = (y2-y1)/(x2-x1);
        c = -a*x1 - b*y1;
        // cout << "a: " << a << " b: " << b << " c:" << c << endl;
        // cout << endl;
    }
}


void computeIntersection(const Vec3f &line1, const Vec3f &line2, Point &intersection)
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
    // cout << "Method" << endl;
    // cout << a1 << " " << b1 << " " << c1 << endl;
    // cout << a2 << " " << b2 << " " << c2 << endl;
    // cout << "Intersection: " << intersection << endl;

}


Vec2b histogram(const Mat &img)
{
    std::vector<Scalar> rgbColors = {Scalar(255, 0, 0), Scalar(0, 255, 0), Scalar(0, 0, 255)};
	//imshow("Cropped image (hist)", img);
	Mat thisImg;
	cvtColor(img, thisImg, COLOR_BGR2HSV);
	Mat hist;
	int histSize = 8; // number of bins
	float range[] = {0, 179+1}; // range (upper bound is exclusive)
	const float* histRange[] = { range };

	// Evaluate only H channel
	int hCh_idx = 0;
    //const int channels[] = {0, 1, 2};
	const int c[] = {hCh_idx};
	calcHist(&thisImg, 1, c, Mat(), hist, 1, &histSize, histRange);

    // draw histogram
    int hist_w = 512, hist_h = 400;
	Mat histImg(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));
	int bin_w = cvRound((double) hist_w/histSize);
	normalize(hist, hist, 0, histImg.rows, NORM_MINMAX, -1, Mat());
	for(int i = 1; i < histSize; i++){
		line(histImg, Point(bin_w*(i-1), hist_h - cvRound(hist.at<float>(i-1))),
			 Point(bin_w*(i), hist_h - cvRound(hist.at<float>(i))),
			 rgbColors[hCh_idx], 2, 8, 0);
	}

    // find the argmax
	Mat argmax;
	reduceArgMax(hist, argmax, 0);
	//cout<<hist<<endl;
	//cout << argmax << endl;
	int start = range[1] / histSize * argmax.at<int>(0);
	int diameter = (range[1] / histSize);
	//imshow("Histogram", histImg);
	return Vec2b(start, start + diameter);
}


void detectTable(const Mat &frame, vector<Point> &corners)
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
    vector<Point> intersections;
    vector<Vec3f> coefficients;

    // get the color range
	Vec2b colorRange = histogram(frame.rowRange(rowsover4, 3*rowsover4).colRange(colsover4, 3*colsover4));

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
    for(size_t i = 0; i < coefficients.size(); i++)
    {
        for(size_t j = i+1; j < coefficients.size(); j++)
        {
            Point intersection;
            computeIntersection(coefficients[i], coefficients[j], intersection);
            if (intersection.x >= 0 && intersection.x < frame.cols && intersection.y >= 0 && intersection.y < frame.rows)
                intersections.push_back(intersection);
        }
    }

    // remove intersections that are too close
    vector<Point> intersectionsGood;
    sort(intersections.begin(), intersections.end(), [frame](Point a, Point b) -> bool
    {
        Point center = Point(frame.cols/2, frame.rows/2);
        return norm(a) < norm(b);
    });
    auto end2 = unique(intersections.begin(), intersections.end(), [&CLOSE_POINT_THRESHOLD](Point a, Point b) -> bool
    {
        return abs(a.x - b.x) < CLOSE_POINT_THRESHOLD && abs(a.y - b.y) < CLOSE_POINT_THRESHOLD;
    });
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
        corners.push_back(intersectionsGood[i]);
    }
    imshow("Line", imgLine);
    //waitKey(0);
}


void detectBalls(const Mat &frame, vector<Ball> &balls, const vector<Point> &tableCorners)
{
    // const used during the function
    const int MIN_RADIUS = 5;
    const int MAX_RADIUS = 15;
    const int HOUGH_PARAM1 = 110;
    const int HOUGH_PARAM2 = 5;
    const int ACCUMULATOR_RESOLUTION = 1;
    const int MIN_DISTANCE = 35;

    // variables
    Mat gray, gradX, gradY, abs_grad_x, abs_grad_y, grad, imgBorder, HSVImg;
    cvtColor(frame, gray, COLOR_BGR2GRAY);
    vector<Vec3f> circles;
    vector<Rect> boundRect;
    Mat frameRect = frame.clone();
    Mat frameCircle = frame.clone();
    cvtColor(frame, HSVImg, COLOR_BGR2HSV);
    int maxY = max(tableCorners[0].y, tableCorners[3].y);
    int minY = min(tableCorners[1].y, tableCorners[2].y);
    int maxX = max(tableCorners[2].x, tableCorners[3].x);
    int minX = min(tableCorners[0].x, tableCorners[1].x);
    gray = gray.rowRange(minY, maxY).colRange(minX, maxX);
    imshow("Cropped image", gray);
    HoughCircles(gray, circles, HOUGH_GRADIENT,
                    ACCUMULATOR_RESOLUTION, MIN_DISTANCE, HOUGH_PARAM1, HOUGH_PARAM2, MIN_RADIUS, MAX_RADIUS);
    vector<Vec3f> circlesFiltered;
    Mat subImg;
    vector<double> mean, stddev;
    Category category;
    for( size_t i = 0; i < circles.size(); i++ )
    {
        Vec3i c = circles[i];
        Point center = Point(c[0] + minX, c[1] + minY);
        int radius = c[2];
        boundRect.push_back(Rect(center.x-c[2], center.y-c[2], 2*c[2], 2*c[2]));
        if(c[0]-c[2] > 0 && c[1]-c[2] > 0 && c[0]+c[2] < frame.cols && c[1]+c[2] < frame.rows)
        {
            int halfRad = static_cast<int>(5*c[2]/8);
            subImg = HSVImg.colRange(c[0]-halfRad, c[0]+halfRad).rowRange(c[1]-halfRad, c[1]+halfRad);
            meanStdDev(subImg, mean, stddev);
            if(mean[1] < 90 && mean[2] > 240)
            { // white ball
                category = Category::WHITE_BALL;
                circle(frameCircle, center, radius, Scalar(255, 255, 255), 1, LINE_AA);
                rectangle(frameRect, boundRect[i], Scalar(255, 255, 255), 1, LINE_AA);
            }
            else if(mean[2] < 80)
            { // black ball
                category = Category::BLACK_BALL;
                circle(frameCircle, center, radius, Scalar(0, 0, 0), 1, LINE_AA);
                rectangle(frameRect, boundRect[i], Scalar(0, 0, 0), 1, LINE_AA);
            }
            else if(stddev[0] < 10)
            { // solid blue
                category = Category::SOLID_BALL;
                circle(frameCircle, center, radius, Scalar(255, 0, 0), 1, LINE_AA);
                rectangle(frameRect, boundRect[i], Scalar(255, 0, 0), 1, LINE_AA);
            }
            else if(stddev[0] > 40)
            { // striped red
                category = Category::STRIPED_BALL;
                circle(frameCircle, center, radius, Scalar(0, 0, 255), 1, LINE_AA);
                rectangle(frameRect, boundRect[i], Scalar(0, 0, 255), 1, LINE_AA);
            }
            Ball ball(boundRect[i], category);
            balls.push_back(ball);
        }
    }
    imshow("detected circles", frameCircle);
    imshow("detected rectangles", frameRect);

    waitKey(0);
}
