// Author: Michele Sprocatti

#include <opencv2/opencv.hpp>
#include "ObjectDetection.h"
#include <iostream>

using namespace cv;
using namespace std;


// Gives the equation of the line passing through two points in the form y = mx + q
void equationFormula(double x1, double y1, double x2, double y2, double &m, double &q){
	m = (y2-y1)/(x2-x1);
	q = -x1*m + y1;
}

std::vector<Scalar> rgbColors = {Scalar(255, 0, 0), Scalar(0, 255, 0), Scalar(0, 0, 255)};
Vec2b histogram(const Mat &img){
	imshow("Cropped image (hist)", img);
	Mat thisImg;
	cvtColor(img, thisImg, COLOR_BGR2HSV);
	Mat hist;
	int histSize = 8; // number of bins
	float range[] = {0, 179+1}; // range (upper bound is exclusive)
	const float* histRange[] = { range };
//	const int channels[] = {0, 1, 2};
	const int channels[] = {0}; // only H channel
	int hist_w = 512, hist_h = 400;
	Mat histImg(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));

	// Evaluate only H channel
	int hCh_idx = 0;
	const int c[] = {hCh_idx};
	calcHist(&thisImg, 1, c, Mat(), hist, 1, &histSize, histRange);
	int bin_w = cvRound((double) hist_w/histSize);
	normalize(hist, hist, 0, histImg.rows, NORM_MINMAX, -1, Mat());
	for(int i = 1; i < histSize; i++){
		line(histImg, Point(bin_w*(i-1), hist_h - cvRound(hist.at<float>(i-1))),
			 Point(bin_w*(i), hist_h - cvRound(hist.at<float>(i))),
			 rgbColors[hCh_idx], 2, 8, 0);
	}

	cv::Mat argmax;
	cv::reduceArgMax(hist, argmax, 0);
	cout<<hist<<endl;
	cout << argmax << endl;
	int start = range[1] / histSize * argmax.at<int>(0);
	int diameter = (range[1] / histSize);
	imshow("Histogram", histImg);

	return Vec2b(start, start + diameter);
}


void detectTable(cv::Mat &frame)
{
    Mat imgGray, imgLine, imgBorder;
	int rowsover4 = frame.rows/4, colsover4 = frame.cols/4;
	Vec2b colorRange = histogram(frame.rowRange(rowsover4, 3*rowsover4).colRange(colsover4, 3*colsover4));
	cout<<"Color range: "<<colorRange<<endl;
	Mat thisImg;
	cvtColor(frame, thisImg, COLOR_BGR2HSV);
	Mat mask;
	inRange(thisImg, Scalar(colorRange[0], 20, 20), Scalar(colorRange[1], 255, 255), mask); // S>20; V>20 to be a color and not black or white
	imshow("Mask", mask);
//	thisImg.copyTo(frame);
//	bitwise_and(frame, frame, frame, mask);
//	frame.copyTo(frame, mask);
//	imshow("Masked", frame);

//	mask.copyTo(frame);

//	frame.convertTo(frame,CV_32F);
//    frame = frame + 1;
//    cv::log(frame,frame);
//    cv::convertScaleAbs(frame,frame);
//    cv::normalize(frame,frame,0,255,cv::NORM_MINMAX);

	Mat kernel = getStructuringElement(MORPH_RECT, Size(11,11));
	morphologyEx(mask, mask, MORPH_CLOSE, kernel);
	imshow("Morphology", mask);


	GaussianBlur(mask, mask, Size(11,11), 0);
    imshow("Gaussian Blur", mask);
//    cvtColor(frame, imgGray, COLOR_BGR2GRAY);
    Canny(mask, imgBorder, 50, 60);
    imshow("Canny Result", imgBorder);
    vector<Vec2f> lines;
    vector<Vec2f> linesFiltered;
    imgLine = frame.clone();
    HoughLines(imgBorder, lines, 1, CV_PI/180, 130);
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
     imshow("Line", imgLine);
    // waitKey(0);
}

void detectBalls(cv::Mat &frame)
{
    Mat gray, gradX, gradY, abs_grad_x, abs_grad_y, grad, imgBorder;
    cvtColor(frame, gray, COLOR_BGR2GRAY);
//    Canny(gray, imgBorder, 40, 130);
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
//    add(imgBorder, gray, gray);
    HoughCircles(gray, circles, HOUGH_GRADIENT, 1, 25, 130, 10, 5, 15);
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
    imshow("detected rectangles", frameRect);

    waitKey(0);
}
