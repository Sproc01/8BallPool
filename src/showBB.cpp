#include <stdexcept>
#include <filesystem>
#include <utility>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <fstream>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <opencv2/imgproc.hpp>

#include "category.h"
#include "minimapConstants.h"

using namespace std;
using namespace cv;

vector<pair<Rect, Category>> readGroundTruthBbox(const string &filename) {
	ifstream file(filename);
	if (!file.is_open()){
		throw invalid_argument("File not found");
	}

	vector<pair<Rect, Category>> bboxes;
	string line;
	while (getline(file, line)){
		istringstream iss(line);
		int x, y, w, h;
		short cat;
		iss >> x >> y >> w >> h >> cat;
		bboxes.push_back(make_pair(Rect(x, y, w, h), static_cast<Category>(cat)));
	}

	return bboxes;
}

int main() {
	vector<pair<Rect,Category>> gts = readGroundTruthBbox("../Dataset/game1_clip1_vertical/bounding_boxes/frame_first_bbox.txt");
	Mat img = imread("../Dataset/game1_clip1_vertical/frames/frame_first.png");

	for (auto &gt : gts) {
		switch(gt.second){
			case SOLID_BALL:
				rectangle(img, gt.first, SOLID_BGR_COLOR, 1, LINE_AA);
			break;
			case STRIPED_BALL:
				rectangle(img, gt.first, STRIPED_BGR_COLOR, 1, LINE_AA);
			break;
			case WHITE_BALL:
				rectangle(img, gt.first, WHITE_BGR_COLOR, 1, LINE_AA);
			break;
			case BLACK_BALL:
				rectangle(img, gt.first, BLACK_BGR_COLOR, 1, LINE_AA);
			break;
			default:
				// do nothing if not a ball
					break;
		}
	}

	imshow("output", img);
	waitKey();
	return 0;
}
