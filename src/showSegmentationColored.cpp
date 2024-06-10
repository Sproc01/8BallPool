#include <opencv2/highgui.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int main(){
	Mat segmented = imread("../Dataset/game1_clip1/masks/frame_first.png", IMREAD_GRAYSCALE);

	Mat segmentedColored(segmented.rows, segmented.cols, CV_8UC3);
	for (int i = 0; i < segmented.rows; i++){
		for (int j = 0; j < segmented.cols; j++){
			switch (segmented.at<uchar>(i, j)){
				case 0: // background
					segmentedColored.at<Vec3b>(i, j) = Vec3b(0, 0, 0);
					break;
				case 1: // cue ball
					segmentedColored.at<Vec3b>(i, j) = Vec3b(255, 255, 255);
					break;
				case 2: // black ball
					segmentedColored.at<Vec3b>(i, j) = Vec3b(100, 100, 100);
					break;
				case 3: // solid ball
					segmentedColored.at<Vec3b>(i, j) = Vec3b(0, 0, 255);
					break;
				case 4: // striped ball
					segmentedColored.at<Vec3b>(i, j) = Vec3b(255, 0, 0);
					break;
				case 5: // playing field
					segmentedColored.at<Vec3b>(i, j) = Vec3b(0, 255, 0);
					break;
				default:// in case of error
					segmentedColored.at<Vec3b>(i, j) = Vec3b(255, 0, 255);
					break;
			}
		}
	}
	imshow("segmentedColored", segmentedColored);
	waitKey();
	return 0;
}
