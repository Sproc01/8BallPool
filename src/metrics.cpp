// Author: Alberto Pasqualetto

#include "metrics.h"
#include "category.h"
#include "table.h"
#include "minimapConstants.h"
#include <stdexcept>
#include <filesystem>
#include <utility>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <fstream>
#include <iostream>
#include <opencv2/highgui.hpp>

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
	double mAP = mAPDetection(table.ballsPtr(), groundTruthBboxPath.string(), MAP_IOU_THRESHOLD);
	cout << "mAP:" << mAP << endl;
	double mIoU = mIoUSegmentation(segmentedImage, groundTruthMaskPath.string());
	cout << "mIoU: " << mIoU << endl;
}


double APCategory(const Ptr<vector<Ball>> &detectedBalls, const vector<pair<Rect, Category>> &groundTruthBboxes, Category cat, float iouThreshold){
	// Create a vector of bounding boxes only for the detected balls of the chosen category
	vector<Rect> detectedBallsBboxesCat;
	for (const Ball &ball : *detectedBalls){
		if (ball.getCategory() == cat){
			detectedBallsBboxesCat.push_back(ball.getBbox());
		}
	}
	// if there are no balls with that category return 0;
	if(detectedBallsBboxesCat.size()==0)
		return 0;

	// Create a vector of bounding boxes only for the ground truths of the chosen category
	vector<Rect> groundTruthBboxesCat;
	for (const pair<Rect, Category> &groundTruthBall : groundTruthBboxes){
		if (groundTruthBall.second == cat){
			groundTruthBboxesCat.push_back(get<Rect>(groundTruthBall));
		}
	}
	vector<double> IoUs(groundTruthBboxesCat.size(), 0);  // if 0, the ground truth ball has not been assigned to any detected ball

	vector<unsigned short> tp(detectedBallsBboxesCat.size(), 0);
	vector<unsigned short> fp(detectedBallsBboxesCat.size(), 0);

	// Couple each detected ball with the ground truth ball using the highest IoU
	for (int i = 0; i < detectedBallsBboxesCat.size(); i++){
		double maxIoU = 0;
		int maxIoUIndex = -1;
		for (int j = 0; j < groundTruthBboxesCat.size(); j++){
			double iou = IoU(detectedBallsBboxesCat[i], groundTruthBboxesCat[j]);
			if (iou > maxIoU){
				maxIoU = iou;
				maxIoUIndex = j;
			}
		}
		if (maxIoU > iouThreshold && IoUs[maxIoUIndex] <= 0){
			tp[i] = 1;
			IoUs[maxIoUIndex] = maxIoU;
		}
		else{
			fp[i] = 1;
		}
	}

	// TODO: check if sorting by IoU is correct
	// Sort the detections by decreasing IoU
	vector<int> indices(IoUs.size());
	for (int i = 0; i < IoUs.size(); i++){
		indices[i] = i;
	}
	sort(indices.begin(), indices.end(),
		 [&](int a, int b) -> bool {
		return IoUs[a] > IoUs[b];   // decreasing order
	});

	// sort tp and fp according to the sorted indices "in place"
	// cout << tp.size() << endl;
	// cout << fp.size() << endl;
	// cout << indices.size() << endl;
	// segmentation fault because indices and tp not the same size because some IoU can be 0
	vector<unsigned short> tpSorted(tp.size());
	for (int i = 0; i<indices.size(); i++){
		if(IoUs[i] != 0)
			tpSorted[i] = tp[indices[i]];
	}
	tp = tpSorted;
	vector<unsigned short> fpSorted(fp.size());
	for (int i = 0; i<indices.size(); i++){
		if(IoUs[i] != 0)
			fpSorted[i] = fp[indices[i]];
	}
	fp = fpSorted;
	sort(IoUs.begin(), IoUs.end(), greater<>());

//	double recall = sum(tp)[0] / groundTruthBboxesCat.size();
//	double precision = sum(tp)[0] / (sum(tp)[0] + sum(fp)[0]);

	// Compute the cumulative TP and FP
	vector<double> cumTP(tp.size());
	vector<double> cumFP(fp.size());

	cumTP[0] = tp[0];
	for (int i = 1; i < tp.size(); i++){
		cumTP[i] = cumTP[i - 1] + tp[i];
	}
	cumFP[0] = fp[0];
	for (int i = 1; i < fp.size(); i++) {
		cumFP[i] = cumFP[i - 1] + fp[i];
	}

	// Compute the precision and recall for each detection
	vector<double> recallVec(tp.size());
	for (int i = 0; i<tp.size(); i++){
		recallVec[i] = cumTP[i] / groundTruthBboxesCat.size();
	}

	vector<double> precisionVec(tp.size());
	for (int i = 0; i<tp.size(); i++){
		if (cumTP[i] + cumFP[i] == 0){
			precisionVec[i] = 0;
		}
		else{
			precisionVec[i] = cumTP[i] / (cumTP[i] + cumFP[i]);
		}
	}


	// Compute the Average Precision
	double AP = 0;
	for (int t = 0; t<=10; t++){
		double maxPrecision = 0;
		for (int i = 0; i<tp.size(); i++){
			if (recallVec[i] >= static_cast<double>(t) / 10.0 && precisionVec[i] > maxPrecision){
				maxPrecision = precisionVec[i];
			}
		}
		AP += maxPrecision / 11;
	}

	return AP;
}


double mAPDetection(const Ptr<vector<Ball>> &detectedBalls, const string &groundTruthBboxPath, float iouThreshold /*= MAP_IOU_THRESHOLD*/){
	vector<pair<Rect, Category>> groundTruthBboxes = readGroundTruthBboxFile(groundTruthBboxPath);

	double mAP = 0;
	for (Category cat=Category::WHITE_BALL; cat<=Category::STRIPED_BALL; cat=static_cast<Category>(cat+1)){
		mAP += APCategory(detectedBalls, groundTruthBboxes, cat, iouThreshold);
	}

	return mAP / static_cast<double>(Category::STRIPED_BALL - Category::WHITE_BALL + 1);
}


double mIoUCategory(const Mat &segmentedImage, const Mat &groundTruthMask, Category cat){
	Mat segmentedImageCat = (segmentedImage == static_cast<unsigned char>(cat));
	Mat groundTruthMaskCat = (groundTruthMask == static_cast<unsigned char>(cat));
	// imshow("segmentedImageCat", segmentedImageCat);
	// imshow("groundTruthMaskCat", groundTruthMaskCat);
	//waitKey();
	double iou = IoU(segmentedImageCat, groundTruthMaskCat);
	cout<< "cat: " << cat << '\t' << "iou: " << iou << endl;
	return iou;
}

// For balls and playing field segmentation, the mean Intersection over Union (mIoU) metric, that is the average of the IoU computed for each class (background, white ball, black ball, solid color, striped and playing field)
double mIoUSegmentation(const Mat &segmentedImage, const string& groundTruthMaskPath){
	// Convert the segmented image from BGR colors to grayscale category-related colors
	Mat segmentedImageGray = Mat::zeros(segmentedImage.size(), CV_8UC1);
	for (int i = 0; i < segmentedImage.rows; i++){
		for (int j = 0; j < segmentedImage.cols; j++){
			if (segmentedImage.at<Vec3b>(i,j) == BACKGROUND_BGR_COLOR){
				segmentedImageGray.at<uchar>(i, j) = static_cast<uchar>(Category::BACKGROUND);
			}
			else if (segmentedImage.at<Vec3b>(i,j) == WHITE_BGR_COLOR){
				segmentedImageGray.at<uchar>(i, j) = static_cast<uchar>(Category::WHITE_BALL);
			}
			else if (segmentedImage.at<Vec3b>(i,j) == BLACK_BGR_COLOR){
				segmentedImageGray.at<uchar>(i, j) = static_cast<uchar>(Category::BLACK_BALL);
			}
			else if (segmentedImage.at<Vec3b>(i,j) == SOLID_BGR_COLOR){
				segmentedImageGray.at<uchar>(i, j) = static_cast<uchar>(Category::SOLID_BALL);
			}
			else if (segmentedImage.at<Vec3b>(i,j) == STRIPED_BGR_COLOR){
				segmentedImageGray.at<uchar>(i, j) = static_cast<uchar>(Category::STRIPED_BALL);
			}
			else if (segmentedImage.at<Vec3b>(i,j) == PLAYING_FIELD_BGR_COLOR){
				segmentedImageGray.at<uchar>(i, j) = static_cast<uchar>(Category::PLAYING_FIELD);
			}
			else{
				throw invalid_argument("Invalid color");
			}
		}
	}

	Mat groundTruthMask = imread(groundTruthMaskPath, IMREAD_GRAYSCALE);

	double mIoU = 0;

	for (Category cat=Category::BACKGROUND; cat<=Category::PLAYING_FIELD; cat=static_cast<Category>(cat+1)){
		mIoU += mIoUCategory(segmentedImageGray, groundTruthMask, cat);
	}

	return mIoU / static_cast<double>(Category::PLAYING_FIELD - Category::BACKGROUND + 1);
}


//double mIoU(vector<Rect> &rects1, vector<Rect> &rects2){
//	if(rects1.size() != rects2.size()){
//		throw invalid_argument("rects1 and rects2 must have the same size");
//	}
//
//	double sum = 0;
//	for (int i = 0; i < rects1.size(); i++){
//		sum += IoU(rects1[i], rects2[i]);
//	}
//	return sum / rects1.size();
//}


double IoU(const Rect &rect1, const Rect &rect2){
	Rect i = rect1 & rect2;
	Rect u = rect1 | rect2;
	return static_cast<double>(i.area()) / static_cast<double>(u.area());
}

double IoU(const Mat &mask1, const Mat &mask2){
	Mat i = mask1 & mask2;
	Mat u = mask1 | mask2;
	return static_cast<double>(countNonZero(i)) / static_cast<double>(countNonZero(u));
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
