// Author: Michele Sprocatti

#include <opencv2/opencv.hpp>
#include "ObjectDetection.h"

using namespace cv;
using namespace std;

void detectTable(cv::Mat &frame)
{
    Mat imgGray, imgLine, imgBorder;
    cvtColor(frame, imgGray, COLOR_BGR2GRAY);
    Canny(imgGray, imgBorder, 100, 200);
    //imshow("Canny Result", imgBorder);
    vector<Vec2f> lines; 
    imgLine = frame.clone();
    HoughLines(imgBorder, lines, 1, CV_PI/180, 150, 0, 0);
    for(size_t i = 0; i < lines.size(); i++)
    {
        float rho = lines[i][0], theta = lines[i][1];
        // if (((theta > 0.70 && theta < 0.90) || (theta > 2.30 && theta < 2.31)) && (rho <550)) 
        // {
            Point pt1, pt2;
            double a = cos(theta), b = sin(theta);
            double x0 = a*rho, y0 = b*rho;
            pt1.x = cvRound(x0 + 1300*(-b));
            pt1.y = cvRound(y0 + 1300*(a));
            pt2.x = cvRound(x0 - 1300*(-b));
            pt2.y = cvRound(y0 - 1300*(a));
            line(imgLine, pt1, pt2, Scalar(0, 255, 0), 1, LINE_8);
        // }
    }
    //imshow("Line", imgLine);
    //waitKey(0);
}

void detectBalls(cv::Mat &frame)
{
    Mat gray, imgBorder;
    cvtColor(frame, gray, COLOR_BGR2GRAY);
    GaussianBlur(gray, gray, Size(3, 3), 1, 1);
    imshow("Gaussian Blur", gray);
    Canny(gray, imgBorder, 50, 100);
    imshow("Canny Result2", imgBorder);
    vector<Vec3f> circles;
    HoughCircles(imgBorder, circles, HOUGH_GRADIENT, 1, 30, 200, 0.75, 1, 15);
    for( size_t i = 0; i < circles.size(); i++ )
    {
        Vec3i c = circles[i];
        Point center = Point(c[0], c[1]);
        int radius = c[2];
        circle(frame, center, radius, Scalar(0, 0, 255), 1, LINE_AA);
    }
    imshow("detected circles", frame);
    waitKey(0);
}