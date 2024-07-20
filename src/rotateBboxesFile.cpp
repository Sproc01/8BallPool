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

#include "category.h"

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


/**
 * @brief Find the point in the 90° clockwise rotated image corresponding to the input point.
 * @param point point with still to be rotated image coordinates.
 * @param destImg destination image (used to get its size).
 * @return rotated Point2f.
 */
Point2f rotatePoint(Point2f point, const Mat &destImg) {
	float old_row = point.y;
	point.y = point.x;
	point.x = destImg.cols - old_row;

	return point;
}

/**
 * @brief Find the point in the 90° counter-clockwise rotated image corresponding to the input point.
 * @param point point with still to be "unrotated" image coordinates.
 * @param destImg destination image (used to get its size).
 * @return "unrotated" Point2f.
 */
Point2f unrotatePoint(Point2f point, const Mat &destImg) {
	float old_row = point.y;
	point.y = destImg.rows - point.x;
	point.x = old_row;

	return point;
}

/**
 * @brief Find the same rect in the 90° clockwise rotated image corresponding to the input point.
 * @param rect Rect with still to be rotated image coordinates.
 * @param destImg destination image (used to get its size).
 * @return rotated Rect.
 */
Rect rotateRect(Rect rect, const Mat &destImg) {
	Point2f oldTR(rect.tl().x + rect.width, rect.tl().y);
	Point2f oldBL(rect.tl().x, rect.tl().y + rect.height);

	return Rect(rotatePoint(oldTR, destImg), rotatePoint(oldBL, destImg));
}

/**
 * @brief Find the same rect in the 90° counter-clockwise rotated image corresponding to the input point.
 * @param rect Rect with still to be unrotated image coordinates.
 * @param destImg destination image (used to get its size).
 * @return "unrotated" Rect.
 */
Rect unrotateRect(Rect rect, const Mat &destImg) {
	Point2f oldTR(rect.tl().x + rect.width, rect.tl().y);
	Point2f oldBL(rect.tl().x, rect.tl().y + rect.height);

	return Rect(unrotatePoint(oldTR, destImg), unrotatePoint(oldBL, destImg));
}

void main() {
	Mat vertImg(1024, 576, CV_8UC3, Scalar(0, 0, 0));
	Mat horImg(576, 1024, CV_8UC3, Scalar(0, 0, 0));

	// string filename = "../Dataset/game1_clip1_vertical/bounding_boxes/frame_first_bbox.txt";
	string filename = "../Dataset/game1_clip1_vertical/bounding_boxes/frame_last_bbox.txt";

	auto gts = readGroundTruthBbox(filename);

	vector<pair<Rect,Category>> rotated;

	for (auto gt : gts){
		cout << "Rect: " << gt.first << " Category: " << gt.second << endl;
		rotated.push_back(make_pair(rotateRect(gt.first, vertImg), gt.second));
		// rotated.push_back(make_pair(unrotateRect(gt.first, horImg), gt.second));
		cout<< "Rotated: " << rotated.back().first << " Category: " << rotated.back().second << endl;
	}

	// write to file one bounding box per line
	ofstream file(filename);
	for (auto r : rotated){
		file << r.first.x << " " << r.first.y << " " << r.first.width << " " << r.first.height << " " << static_cast<int>(r.second) << endl;
	}
}
