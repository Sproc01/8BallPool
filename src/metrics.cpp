// Author: Alberto Pasqualetto

#include "metrics.h"
#include "category.h"
#include "table.h"
#include <stdexcept>
#include <filesystem>
#include <utility>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <fstream>
#include <iostream>

using namespace std;
using namespace cv;

/**
 *
 * @param table
 * @param segmentedImage
 * @param folderPath
 * @param frameN Set to FIRST for the first frame, LAST for the last frame
 */
void compareMetrics(Table &table, Mat &segmentedImage, const string &folderPath, FrameN frameN){
	filesystem::path groundTruthBboxPath;
	filesystem::path groundTruthMaskPath;
	switch (frameN) {
		case FIRST:
			groundTruthBboxPath = filesystem::path(folderPath) / "bounding_boxes" / "frame_first_bbox.txt";
			groundTruthMaskPath = filesystem::path(folderPath) / "masks" / "frame_first.png";
			break;
		case LAST:
			groundTruthBboxPath = filesystem::path(folderPath) / "bounding_boxes" / "frame_last_bbox.txt";
			groundTruthMaskPath = filesystem::path(folderPath) / "masks" / "frame_last.png";
			break;
		default:
			throw invalid_argument("frameN must be FIRST or LAST");
	}

	// For ball localization, the mean Average Precision (mAP) calculated at IoU threshold 0.5
	float mAP = mAPDetection(table.ballsPtr(), groundTruthBboxPath.string(), MAP_IOU_THRESHOLD);

	float mIoU = mIoUSegmentation(segmentedImage, groundTruthMaskPath.string());
	cout << "mIoU: " << mIoU << endl;
}

float mAPDetection(const Ptr<vector<Ball>> &detectedBalls, const string &groundTruthBboxPath, float iouThreshold) {
	vector<pair<Rect, Category>> groundTruthBboxes = readGroundTruthBboxFile(groundTruthBboxPath);

	float mAP = 0;
}

float mIoUCategory(const Mat &segmentedImage, const Mat &groundTruthMask, Category cat){
	Mat segmentedImageCat = (segmentedImage == static_cast<int>(cat));
	Mat groundTruthMaskCat = (groundTruthMask == static_cast<int>(cat));
	return IoU(segmentedImageCat, groundTruthMaskCat);
}

// For balls and playing field segmentation, the mean Intersection over Union (mIoU) metric, that is the average of the IoU computed for each class (background, white ball, black ball, solid color, striped and playing field)
float mIoUSegmentation(const Mat &segmentedImage, const string& groundTruthMaskPath){
	Mat groundTruthMask = imread(groundTruthMaskPath, IMREAD_GRAYSCALE);

	float mIoU = 0;

	for (Category cat=Category::BACKGROUND; cat<=Category::PLAYING_FIELD; cat=static_cast<Category>(cat+1)){
		mIoU += mIoUCategory(segmentedImage, groundTruthMask, cat);
	}

	return mIoU / static_cast<float>(Category::PLAYING_FIELD - Category::BACKGROUND + 1);
}


//float mIoU(vector<Rect> &rects1, vector<Rect> &rects2){
//	if(rects1.size() != rects2.size()){
//		throw invalid_argument("rects1 and rects2 must have the same size");
//	}
//
//	float sum = 0;
//	for (int i = 0; i < rects1.size(); i++){
//		sum += IoU(rects1[i], rects2[i]);
//	}
//	return sum / rects1.size();
//}


//float IoU(Rect &rect1, Rect &rect2){
//	Rect i = rect1 & rect2;
//	Rect u = rect1 | rect2;
//	return static_cast<float>(i.area() / u.area());
//}

float IoU(const Mat &mask1, const Mat &mask2){
	Mat i = mask1 & mask2;
	Mat u = mask1 | mask2;
	return static_cast<float>(countNonZero(i)) / static_cast<float>(countNonZero(u));
}


vector<pair<Rect, Category>> readGroundTruthBboxFile(const string &filename) {
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
