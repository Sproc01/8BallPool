// Author: Alberto Pasqualetto

#include "metrics.h"
#include "category.h"
#include "table.h"
#include "ball.h"
#include "minimap.h"
#include <stdexcept>
#include <filesystem>
#include <stdexcept>
#include <utility>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <fstream>


using namespace std;
using namespace cv;


/**
 * @brief Compute the Intersection over Union between the segmented image and the ground truth mask.
 * @param segmentedImage segmented image.
 * @param folderPath path to the folder containing the ground truth masks.
 * @param frameN Set to FIRST for the first frame, LAST for the last frame.
 * @return std::vector<double> vector of IoU values for each category.
 * @throw invalid_argument if segmentedImage is empty.
 */
vector<double> compareMetricsIoU(const Mat &segmentedImage, const string &folderPath, const FrameN &frameN) {
	if (segmentedImage.empty())
		throw invalid_argument("Empty segmentedImage");
	filesystem::path groundTruthMaskPath;
	switch (frameN) {
		case FIRST:
			groundTruthMaskPath = filesystem::path(folderPath) / "masks" / "frame_first.png";
			break;
		case LAST:
			groundTruthMaskPath = filesystem::path(folderPath) / "masks" / "frame_last.png";
			break;
		default:
			throw invalid_argument("frameN must be FIRST or LAST");
	}

	// For ball localization, the mean Average Precision (mAP) calculated at IoU threshold 0.5
	vector<double> IoUs = IoUSegmentation(segmentedImage, groundTruthMaskPath.string());
	return IoUs;
}

/**
 * @brief Compute the Average Precision (mAP) for ball detection.
 * @param table Table object containing the detected balls.
 * @param folderPath path to the folder containing the ground truth bounding boxes.
 * @param frameN Set to FIRST for the first frame, LAST for the last frame.
 * @return std::vector<double> vector of AP values for each category.
 */
vector<double> compareMetricsAP(Table &table, const string &folderPath, const FrameN &frameN) {
	filesystem::path groundTruthBboxPath;
	switch (frameN) {
		case FIRST:
			groundTruthBboxPath = filesystem::path(folderPath) / "bounding_boxes" / "frame_first_bbox.txt";
			break;
		case LAST:
			groundTruthBboxPath = filesystem::path(folderPath) / "bounding_boxes" / "frame_last_bbox.txt";
			break;
		default:
			throw invalid_argument("frameN must be FIRST or LAST");
	}

	vector<double> APs = APDetection(table.ballsPtr(), groundTruthBboxPath.string(), MAP_IOU_THRESHOLD);
	return APs;
}


/**
 * @brief Read the ground truth bounding boxes from a file with format "x y w h category".
 * @param filename path to the file.
 * @return std::vector<std::pair<cv::Rect, Category>> vector of pairs of rectangles and categories that represent the ground truth bounding boxes.
 * @throw invalid_argument if the file does not exist.
 */
vector<pair<Rect, Category>> readGroundTruthBboxFile(const string &filename) {
	ifstream file(filename);
	if (!file.is_open()) {
		throw invalid_argument("File not found");
	}

	vector<pair<Rect, Category>> bboxes;
	string line;
	while (getline(file, line)) {
		istringstream iss(line);
		int x, y, w, h;
		short cat;
		iss >> x >> y >> w >> h >> cat;
		bboxes.push_back(make_pair(Rect(x, y, w, h), static_cast<Category>(cat)));
	}

	return bboxes;
}


/**
 * @brief Compute the Average Precision (AP) for balls detection.
 * @param detectedBalls vector of detected balls.
 * @param groundTruthBboxPath path to the file containing the ground truth bounding boxes.
 * @param iouThreshold Intersection over Union threshold.
 * @return std::vector<double> vector of AP values for each category.
 * @throw invalid_argument if the vector of detected balls is empty.
 */
vector<double> APDetection(Ptr<vector<Ball>> detectedBalls, const string &groundTruthBboxPath, float iouThreshold /*= MAP_IOU_THRESHOLD*/){
	if(detectedBalls->empty())
		throw invalid_argument("Empty detectedBalls");

	vector<pair<Rect, Category>> groundTruthBboxes = readGroundTruthBboxFile(groundTruthBboxPath);
	vector<double> APs;
	for (Category cat = Category::WHITE_BALL; cat <= Category::STRIPED_BALL; cat = static_cast<Category>(cat + 1)) {
		APs.push_back(APBallCategory(detectedBalls, groundTruthBboxes, cat, iouThreshold));
	}

	return APs;
}

/**
 * @brief Compute the Intersection over Union between the segmented image and the ground truth mask.
 * @param segmentedImage segmented image.
 * @param groundTruthMaskPath path to the ground truth mask.
 * @return std::vector<double> vector of IoU values for each category.
 * @throw invalid_argument if the segmented image is empty.
 */
vector<double> IoUSegmentation(const Mat &segmentedImage, const string &groundTruthMaskPath) {
	if (segmentedImage.empty())
		throw invalid_argument("Empty segmentedImage");

	// Convert the segmented image from BGR colors to grayscale category-related colors
	Mat segmentedImageGray = Mat::zeros(segmentedImage.size(), CV_8UC1);
	for (int i = 0; i < segmentedImage.rows; i++) {
		for (int j = 0; j < segmentedImage.cols; j++) {
			if (segmentedImage.at<Vec3b>(i, j) == BACKGROUND_BGR_COLOR) {
				segmentedImageGray.at<uchar>(i, j) = static_cast<uchar>(Category::BACKGROUND);
			} else if (segmentedImage.at<Vec3b>(i, j) == WHITE_BGR_COLOR) {
				segmentedImageGray.at<uchar>(i, j) = static_cast<uchar>(Category::WHITE_BALL);
			} else if (segmentedImage.at<Vec3b>(i, j) == BLACK_BGR_COLOR) {
				segmentedImageGray.at<uchar>(i, j) = static_cast<uchar>(Category::BLACK_BALL);
			} else if (segmentedImage.at<Vec3b>(i, j) == SOLID_BGR_COLOR) {
				segmentedImageGray.at<uchar>(i, j) = static_cast<uchar>(Category::SOLID_BALL);
			} else if (segmentedImage.at<Vec3b>(i, j) == STRIPED_BGR_COLOR) {
				segmentedImageGray.at<uchar>(i, j) = static_cast<uchar>(Category::STRIPED_BALL);
			} else if (segmentedImage.at<Vec3b>(i, j) == PLAYING_FIELD_BGR_COLOR) {
				segmentedImageGray.at<uchar>(i, j) = static_cast<uchar>(Category::PLAYING_FIELD);
			} else {
				throw invalid_argument("Invalid color");
			}
		}
	}

	Mat groundTruthMask = imread(groundTruthMaskPath, IMREAD_GRAYSCALE);

	vector<double> IoUs;

	for (Category cat = Category::BACKGROUND; cat <= Category::PLAYING_FIELD; cat = static_cast<Category>(cat + 1)) {
		IoUs.push_back(IoUCategory(segmentedImageGray, groundTruthMask, cat));
	}

	return IoUs;
}

/**
 * @brief Compute the Average Precision (AP) for ball detection of a specific category.
 * @param detectedBalls vector of detected balls.
 * @param groundTruthBboxes vector of pairs of (Rect, Category) that represent the ground truth bounding boxes.
 * @param cat Category.
 * @param iouThreshold Intersection over Union threshold.
 * @return double AP value.
 * @throw invalid_argument if the vector of detected balls is empty or if the vector of ground truth bounding boxes is empty.
 */
double APBallCategory(Ptr<vector<Ball>> &detectedBalls, const vector<pair<Rect, Category>> &groundTruthBboxes, Category cat, float iouThreshold){
	if(detectedBalls->empty())
		throw invalid_argument("Empty detectedBalls");

	if (groundTruthBboxes.empty())
		throw invalid_argument("Empty groundTruthBboxes");

	// Create a vector of bounding boxes only for the detected balls of the chosen category
	vector<Rect> detectedBallsBboxesCat;
	for (const Ball &ball : *detectedBalls) {
		if (ball.getCategory() == cat) {
			detectedBallsBboxesCat.push_back(ball.getBbox());
		}
	}

	// Create a vector of bounding boxes only for the ground truths of the chosen category
	vector<Rect> groundTruthBboxesCat;
	for (const pair<Rect, Category> &groundTruthBall : groundTruthBboxes) {
		if (groundTruthBall.second == cat) {
			groundTruthBboxesCat.push_back(get<Rect>(groundTruthBall));
		}
	}

	if (detectedBallsBboxesCat.empty() && groundTruthBboxesCat.empty())
		return 1; // if there are no balls with that category in both gt and detected return 1

	if (detectedBallsBboxesCat.empty() && !groundTruthBboxesCat.empty())
		return 0; // if there are no balls with that category in detected but not in gt return 0

	vector<bool> assignedGroundTruths(groundTruthBboxesCat.size(), false);

	// IoUs, tp and fp vectors share the same indexing
	vector<double> IoUs; // if 0, the ground truth ball has not been assigned to any detected ball

	vector<unsigned short> tp;
	vector<unsigned short> fp;

	// Couple each detected ball with the ground truth ball using the highest IoU
	for (int i = 0; i < detectedBallsBboxesCat.size(); i++) {
		double maxIoU = 0;
		int maxIoUIndex = -1;
		for (int j = 0; j < groundTruthBboxesCat.size(); j++) {
			// if there are more ground truths than detected balls, the unassigned ones will be false negatives
			double iou = IoU(detectedBallsBboxesCat[i], groundTruthBboxesCat[j]);
			if (iou > maxIoU) {
				maxIoU = iou;
				maxIoUIndex = j;
			}
		}
		if ((maxIoUIndex != -1 && !assignedGroundTruths[maxIoUIndex]) && maxIoU > iouThreshold) {
			// if there is a ground truth which is not already assigned and true positive
			assignedGroundTruths[maxIoUIndex] = true;
			tp.push_back(1);
			fp.push_back(0);
		} else {
			tp.push_back(0);
			fp.push_back(1);
		}
		IoUs.push_back(maxIoU);
	}

	// Sort the detections by decreasing IoU using a index vector
	vector<int> indices(IoUs.size());
	for (int i = 0; i < IoUs.size(); i++) {
		indices[i] = i;
	}
	sort(indices.begin(), indices.end(),
		 [&](int a, int b) -> bool {
		return IoUs[a] > IoUs[b];   // decreasing order
	});

	// sort tp and fp according to the sorted indices "in place"
	vector<unsigned short> tpSorted(tp.size());
	for (int i = 0; i < indices.size(); i++) {
		tpSorted[i] = tp[indices[i]];
	}
	tp = tpSorted;

	vector<unsigned short> fpSorted(fp.size());
	for (int i = 0; i < indices.size(); i++) {
		fpSorted[i] = fp[indices[i]];
	}
	fp = fpSorted;

	// Compute the cumulative TP and FP
	vector<double> cumTP(tp.size());
	vector<double> cumFP(fp.size());

	cumTP[0] = tp[0];
	for (int i = 1; i < tp.size(); i++) {
		cumTP[i] = cumTP[i - 1] + tp[i];
	}
	cumFP[0] = fp[0];
	for (int i = 1; i < fp.size(); i++) {
		cumFP[i] = cumFP[i - 1] + fp[i];
	}

	// Compute the precision and recall for each detection
	vector<double> precisionVec(tp.size());
	for (int i = 0; i < tp.size(); i++) {
		precisionVec[i] = (cumTP[i] + cumFP[i] != 0) ? cumTP[i] / (cumTP[i] + cumFP[i]) : 0;
	}

	vector<double> recallVec(tp.size());
	for (int i = 0; i < tp.size(); i++) {
		recallVec[i] = (groundTruthBboxesCat.size() != 0) ? cumTP[i] / groundTruthBboxesCat.size() : 1;
	}

	// Compute the Average Precision
	double AP = 0;
	for (int t = 0; t <= 10; t++) {
		double maxPrecision = 0;
		for (int i = 0; i < tp.size(); i++) { // pick the maximum precision for each recall step
			if (recallVec[i] >= static_cast<double>(t) / 10.0 && precisionVec[i] > maxPrecision) {
				maxPrecision = precisionVec[i];
			}
		}
		AP += maxPrecision / 11;
	}

	return AP;
}

/**
 * @brief Compute the Intersection over Union between the segmented image and the ground truth mask of a specific category.
 * @param segmentedImage segmented image.
 * @param groundTruthMask ground truth mask.
 * @param cat Category.
 * @return double IoU value.
 * @throw invalid_argument if the segmented image is empty or if the ground truth mask is empty.
 */
double IoUCategory(const Mat &segmentedImage, const Mat &groundTruthMask, const Category &cat) {
	if (segmentedImage.empty() || groundTruthMask.empty())
		throw invalid_argument("Empty image");

	Mat segmentedImageCat = (segmentedImage == static_cast<unsigned char>(cat));
	Mat groundTruthMaskCat = (groundTruthMask == static_cast<unsigned char>(cat));
	// imshow("segmentedImageCat", segmentedImageCat);
	// imshow("groundTruthMaskCat", groundTruthMaskCat);
	//waitKey();
	double iou = IoU(segmentedImageCat, groundTruthMaskCat);
	return iou;
}


/**
 * @brief Compute the Intersection over Union between two Rect.
 * @param rect1 first Rect.
 * @param rect2 second Rect.
 * @return double IoU value.
 * @throw invalid_argument if one of the two Rect is empty.
 */
double IoU(const Rect &rect1, const Rect &rect2) {
	if (rect1.empty() || rect2.empty())
		throw invalid_argument("Empty rectangle");

	Rect i = rect1 & rect2;
	Rect u = rect1 | rect2;
	return (u.area() != 0) ? static_cast<double>(i.area()) / static_cast<double>(u.area()) : 1.0;
}

/**
 * @brief Compute the Intersection over Union between two binary masks.
 * @param mask1 first binary mask.
 * @param mask2 second binary mask.
 * @return double IoU value.
 * @throw invalid_argument if one of the two masks is empty.
 */
double IoU(const Mat &mask1, const Mat &mask2) {
	if (mask1.empty() || mask2.empty())
		throw invalid_argument("Empty mask");

	Mat i = mask1 & mask2;
	Mat u = mask1 | mask2;
	return (countNonZero(u) != 0) ? static_cast<double>(countNonZero(i)) / static_cast<double>(countNonZero(u)) : 1.0;
}
