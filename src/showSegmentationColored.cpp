#include <opencv2/highgui.hpp>
#include "category.h"
#include "minimapConstants.h"
#include <iostream>

using namespace std;
using namespace cv;

int main(){
	Mat segmented = imread("../Dataset/game1_clip1/masks/frame_first.png", IMREAD_GRAYSCALE);

	Mat segmentedColored(segmented.rows, segmented.cols, CV_8UC3);
	for (int i = 0; i < segmented.rows; i++){
		for (int j = 0; j < segmented.cols; j++){
			switch (segmented.at<uchar>(i, j)){
				case Category::BACKGROUND:
					segmentedColored.at<Vec3b>(i, j) = BACKGROUND_BGR_COLOR;
					break;
				case Category::WHITE_BALL:
					segmentedColored.at<Vec3b>(i, j) = WHITE_BGR_COLOR;
					break;
				case Category::BLACK_BALL:
					segmentedColored.at<Vec3b>(i, j) = BLACK_BGR_COLOR;
					break;
				case Category::SOLID_BALL:
					segmentedColored.at<Vec3b>(i, j) = SOLID_BGR_COLOR;
					break;
				case Category::STRIPED_BALL:
					segmentedColored.at<Vec3b>(i, j) = STRIPED_BGR_COLOR;
					break;
				case Category::PLAYING_FIELD:
					segmentedColored.at<Vec3b>(i, j) = PLAYING_FIELD_BGR_COLOR;
					break;
				default:    // in case of error
					segmentedColored.at<Vec3b>(i, j) = Vec3b(255, 255, 0);
					break;
			}
		}
	}
	imshow("segmentedColored", segmentedColored);
	waitKey();
	return 0;
}
