// Author: Alberto Pasqualetto

#include <opencv2/highgui.hpp>
#include "ball.h"
#include "table.h"
#include "minimap.h"
#include "metrics.h"
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

// simple main to see the ground truths and to test the code for the metrics
int main(){
	Mat segmented = imread("../Dataset/game2_clip1/masks/frame_last.png", IMREAD_GRAYSCALE);
	vector<double> metricsAP;
	vector<double> metricsIoU;
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
					//segmentedColored.at<Vec3b>(i, j) = Vec3b(255, 255, 0);
					break;
			}
		}
	}
	imshow("segmentedColored", segmentedColored);
	const string str = "../Dataset/game2_clip1/bounding_boxes/frame_last_bbox.txt";
	ifstream file(str);
	Table table;
	vector<Ball> bboxes;
	string line;
	while (getline(file, line)){
		istringstream iss(line);
		int x, y, w, h;
		short cat;
		iss >> x >> y >> w >> h >> cat;
		bboxes.push_back(Ball(Rect(x, y, w, h), static_cast<Category>(cat)));
	}
	table.addBalls(bboxes);
	metricsAP = compareMetricsAP(table, "../Dataset/game2_clip1", LAST);
	metricsIoU = compareMetricsIoU(segmentedColored, "../Dataset/game2_clip1", LAST);
	for(int c = 0; c < metricsAP.size(); c++)
		cout << "AP for category " << c+1 << ": " << metricsAP[c] << endl;

	for(int c = 0; c < metricsIoU.size(); c++)
		cout << "IoU for category " << c << ": " << metricsIoU[c] << endl;
	waitKey();
	return 0;
}
