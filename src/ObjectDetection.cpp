// Author: Michele Sprocatti

#include <opencv2/opencv.hpp>
#include "ObjectDetection.h"
#include <iostream>

using namespace cv;
using namespace std;

void detectTable(cv::Mat &frame)
{
    Mat imgGray, imgLine, imgBorder;
    cvtColor(frame, imgGray, COLOR_BGR2GRAY);
    Canny(imgGray, imgBorder, 100, 200);
    //imshow("Canny Result", imgBorder);
    vector<Vec2f> lines;
    vector<Vec2f> linesFiltered;
    imgLine = frame.clone();
    HoughLines(imgBorder, lines, 1, CV_PI/180, 150, 0, 0);
    // for(size_t i = 0; i < 8; i++)
    // {
    //     linesFiltered.push_back(lines[i]);
    // }
    for(size_t i = 0; i < lines.size(); i++)
    {
        float rho = lines[i][0], theta = lines[i][1];
        Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;
        pt1.x = cvRound(x0 + 1300*(-b));
        pt1.y = cvRound(y0 + 1300*(a));
        pt2.x = cvRound(x0 - 1300*(-b));
        pt2.y = cvRound(y0 - 1300*(a));
        line(imgLine, pt1, pt2, Scalar(0, 0, 255), 1, LINE_AA);
    }
    // imshow("Line", imgLine);
    // waitKey(0);
}

void detectBalls(cv::Mat &frame)
{
    Mat gray, gradX, gradY, abs_grad_x, abs_grad_y, grad, imgBorder;
    cvtColor(frame, gray, COLOR_BGR2GRAY);
    Canny(gray, imgBorder, 40, 130);
    // Sobel(gray, gradX, CV_8U, 2, 0);
    // Sobel(gray, gradY, CV_8U, 0, 2);
    // convertScaleAbs(gradX, abs_grad_x);
    // convertScaleAbs(gradY, abs_grad_y);
    // addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);
    // add(gray, grad, imgBorder);
    //imshow("Canny", imgBorder);
    vector<Vec3f> circles;
    vector<Rect> boundRect;
    Mat frameRect = frame.clone();
    add(imgBorder, gray, gray);
    HoughCircles(gray, circles, HOUGH_GRADIENT, 1, 25, 300, 0.9, 2, 15);
    vector<Vec3f> circlesFiltered;
    // for(size_t i = 0; i < circles.size(); i++)
    // {
    //     if(circles[i][2] > 10)
    //     {
    //         circlesFiltered.push_back(circles[i]);
    //     }
    // }
    Mat subImg;
    vector<double> mean, stddev;
    for( size_t i = 0; i < circles.size(); i++ )
    {
        Vec3i c = circles[i];
        Point center = Point(c[0], c[1]);
        int radius = c[2];
        boundRect.push_back(Rect(c[0]-c[2], c[1]-c[2], 2*c[2], 2*c[2]));
        //printf("Ball %d: center = (%d, %d), radius = %d\n", (int)i, c[0], c[1], c[2]);
        //printf("Cols: %d, Rows: %d\n", frame.cols, frame.rows);
        if(c[0]-c[2]<0 || c[1]-c[2]<0 || c[0]+c[2]>frame.cols || c[1]+c[2]>frame.rows)
        {
            //printf("Ball %d is out of bounds\n", (int)i);
            continue;
        }
        else
        {
            int halfRad = (int)(c[2]/2);
            // printf("%d: %d\n", i, c[2]);
            // printf("%d: %d\n", i, halfRad);
            subImg = frameRect.colRange(c[0]-halfRad, c[0]+halfRad).rowRange(c[1]-halfRad, c[1]+halfRad);
            meanStdDev(subImg, mean, stddev);
            //printf("Mean len: %d, StdDev len: %d\n", (int)mean.size(), (int)stddev.size());
            //printf("Mean: %f, StdDev: %f\n", mean[0], stddev[0]);
            if(mean[0] > 150 && mean[1] > 150 && mean[2] > 150 && stddev[0] < 25 && stddev[0] < 25 && stddev[0] < 25)
            { // white
                circle(frame, center, radius, Scalar(255, 255, 255), 1, LINE_AA);
                rectangle(frameRect, boundRect[i], Scalar(255, 255, 255), 1, LINE_AA);
            }
            else if(mean[0] < 100 && mean[1] < 100 && mean[2] < 100 && stddev[0] < 30 &&  stddev[1] < 30 &&  stddev[2] < 30)
            { // black
                circle(frame, center, radius, Scalar(0, 0, 0), 1, LINE_AA);
                rectangle(frameRect, boundRect[i], Scalar(0, 0, 0), 1, LINE_AA);
            }
            else if(stddev[0] < 50 && stddev[1] < 50 && stddev[2] < 50)
            { // full red
                circle(frame, center, radius, Scalar(0, 0, 255), 1, LINE_AA);
                rectangle(frameRect, boundRect[i], Scalar(0, 0, 255), 1, LINE_AA);
            }
            else
            { // half green
                circle(frame, center, radius, Scalar(0, 255, 0), 1, LINE_AA);
                rectangle(frameRect, boundRect[i], Scalar(0, 255, 0), 1, LINE_AA);
            }   
        }  
    }
    imshow("detected circles", frame);
    //imshow("detected rectangles", frameRect);
    
    waitKey(0);
}