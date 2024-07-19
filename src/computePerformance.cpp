// Author: Michele Sprocatti

#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
#include <filesystem>

#include "ball.h"
#include "table.h"
#include "detection.h"
#include "segmentation.h"
#include "minimapConstants.h"
#include "metrics.h"
#include "util.h"

using namespace std;
using namespace cv;

/* Simple main to compute the performance across the dataset.*/
int main(){

	Mat frame, previousFrame, segmented, detected, segmentedImageGray;
	Vec2b colorTable;
	Vec<Point2f, 4> tableCorners;
	Table table;


	vector<Ball> detectedBallWhite;
	vector<Ball> detectedBallBlack;
	vector<Ball> detectedBallSolid;
	vector<Ball> detectedBallStriped;
	vector<pair<Rect, Category>> groundTruthBboxWhite;
	vector<pair<Rect, Category>> groundTruthBboxBlack;
	vector<pair<Rect, Category>> groundTruthBboxSolid;
	vector<pair<Rect, Category>> groundTruthBboxStriped;


	vector<pair<Rect, Category>> gt;


	vector<Mat> segmentedFound;
	vector<Mat> segmentedGT;
	double IoU_white = 0;
	double IoU_black = 0;
	double IoU_solid = 0;
	double IoU_striped = 0;
	double IoU_playingField = 0;
	double IoU_background = 0;
	double mIoU = 0;

	vector<string> filename ={"/game1_clip1", "/game1_clip2", "/game1_clip3",
								"/game1_clip4", "/game2_clip1", "/game2_clip2",
								"/game3_clip1", "/game3_clip2", "/game4_clip1",
								"/game4_clip2"};


	for(int i = 0; i < filename.size(); i++){

		VideoCapture vid = VideoCapture("../Dataset"+filename[i]+filename[i]+".mp4");
		vid.read(frame);
		detectTable(frame, tableCorners, colorTable);
		table = Table(tableCorners, colorTable);
		detectBalls(frame, table, detected);
		segmentTable(frame, table, segmented);
		segmentBalls(frame, table.ballsPtr(), segmented);
		segmentedImageGray = Mat::zeros(segmented.size(), CV_8UC1);

		// save result mIOU
		for (int j = 0; j < segmented.cols; j++){
			if (segmented.at<Vec3b>(i,j) == BACKGROUND_BGR_COLOR){
				segmentedImageGray.at<uchar>(i, j) = static_cast<uchar>(Category::BACKGROUND);
			}
			else if (segmented.at<Vec3b>(i,j) == WHITE_BGR_COLOR){
				segmentedImageGray.at<uchar>(i, j) = static_cast<uchar>(Category::WHITE_BALL);
			}
			else if (segmented.at<Vec3b>(i,j) == BLACK_BGR_COLOR){
				segmentedImageGray.at<uchar>(i, j) = static_cast<uchar>(Category::BLACK_BALL);
			}
			else if (segmented.at<Vec3b>(i,j) == SOLID_BGR_COLOR){
				segmentedImageGray.at<uchar>(i, j) = static_cast<uchar>(Category::SOLID_BALL);
			}
			else if (segmented.at<Vec3b>(i,j) == STRIPED_BGR_COLOR){
				segmentedImageGray.at<uchar>(i, j) = static_cast<uchar>(Category::STRIPED_BALL);
			}
			else if (segmented.at<Vec3b>(i,j) == PLAYING_FIELD_BGR_COLOR){
				segmentedImageGray.at<uchar>(i, j) = static_cast<uchar>(Category::PLAYING_FIELD);
			}
		}
		segmentedFound.push_back(segmentedImageGray);
		segmentedGT.push_back(imread("../Dataset"+filename[i]+"/masks/frame_first.png", IMREAD_GRAYSCALE));

		// save the results mAP
		for(int i = 0; i < table.ballsPtr()->size(); i++){
			Ball ball = table.ballsPtr()->at(i);
			if(ball.getCategory() == WHITE_BALL){
				detectedBallWhite.push_back(ball);
			}
			else if(ball.getCategory() == BLACK_BALL){
				detectedBallBlack.push_back(ball);
			}
			else if(ball.getCategory() == SOLID_BALL){
				detectedBallSolid.push_back(ball);
			}
			else if(ball.getCategory() == STRIPED_BALL){
				detectedBallStriped.push_back(ball);
			}
		}
		gt = readGroundTruthBboxFile("../Dataset"+filename[i]+"/bounding_boxes/frame_first_bbox.txt");
		for(int i = 0; i < gt.size(); i++){
			if(gt[i].second == WHITE_BALL){
				groundTruthBboxWhite.push_back(gt[i]);
			}
			else if(gt[i].second == BLACK_BALL){
				groundTruthBboxBlack.push_back(gt[i]);
			}
			else if(gt[i].second == SOLID_BALL){
				groundTruthBboxSolid.push_back(gt[i]);
			}
			else if(gt[i].second == STRIPED_BALL){
				groundTruthBboxStriped.push_back(gt[i]);
			}
		}

		// rest of the video
		previousFrame = frame.clone();
		bool ret = vid.read(frame);
		while (vid.isOpened() && ret){
			previousFrame = frame.clone();
			ret = vid.read(frame);
		}

		// last frame
		table.clearBalls();
		detectBalls(previousFrame, table, detected);
		segmentTable(previousFrame, table, segmented);
		segmentBalls(segmented, table.ballsPtr(), segmented);
		segmentedImageGray = Mat::zeros(segmented.size(), CV_8UC1);
		// save result mIOU
		for (int j = 0; j < segmented.cols; j++){
			if (segmented.at<Vec3b>(i,j) == BACKGROUND_BGR_COLOR){
				segmentedImageGray.at<uchar>(i, j) = static_cast<uchar>(Category::BACKGROUND);
			}
			else if (segmented.at<Vec3b>(i,j) == WHITE_BGR_COLOR){
				segmentedImageGray.at<uchar>(i, j) = static_cast<uchar>(Category::WHITE_BALL);
			}
			else if (segmented.at<Vec3b>(i,j) == BLACK_BGR_COLOR){
				segmentedImageGray.at<uchar>(i, j) = static_cast<uchar>(Category::BLACK_BALL);
			}
			else if (segmented.at<Vec3b>(i,j) == SOLID_BGR_COLOR){
				segmented.at<uchar>(i, j) = static_cast<uchar>(Category::SOLID_BALL);
			}
			else if (segmented.at<Vec3b>(i,j) == STRIPED_BGR_COLOR){
				segmentedImageGray.at<uchar>(i, j) = static_cast<uchar>(Category::STRIPED_BALL);
			}
			else if (segmented.at<Vec3b>(i,j) == PLAYING_FIELD_BGR_COLOR){
				segmentedImageGray.at<uchar>(i, j) = static_cast<uchar>(Category::PLAYING_FIELD);
			}
		}
		segmentedFound.push_back(segmentedImageGray);
		segmentedGT.push_back(imread("../Dataset"+filename[i]+"/masks/frame_last.png", IMREAD_GRAYSCALE));

		// save the results mAP
		for(int i = 0; i < table.ballsPtr()->size(); i++){
			Ball ball = table.ballsPtr()->at(i);
			if(ball.getCategory() == WHITE_BALL){
				detectedBallWhite.push_back(ball);
			}
			else if(ball.getCategory() == BLACK_BALL){
				detectedBallBlack.push_back(ball);
			}
			else if(ball.getCategory() == SOLID_BALL){
				detectedBallSolid.push_back(ball);
			}
			else if(ball.getCategory() == STRIPED_BALL){
				detectedBallStriped.push_back(ball);
			}
		}
		gt = readGroundTruthBboxFile("../Dataset"+filename[i]+"/bounding_boxes/frame_last_bbox.txt");
		for(int i = 0; i < gt.size(); i++){
			if(gt[i].second == WHITE_BALL){
				groundTruthBboxWhite.push_back(gt[i]);
			}
			else if(gt[i].second == BLACK_BALL){
				groundTruthBboxBlack.push_back(gt[i]);
			}
			else if(gt[i].second == SOLID_BALL){
				groundTruthBboxSolid.push_back(gt[i]);
			}
			else if(gt[i].second == STRIPED_BALL){
				groundTruthBboxStriped.push_back(gt[i]);
			}
		}
	}

	// compute the mAP
	Ptr<vector<Ball>> detectedBallWhitePtr = makePtr<vector<Ball>>(detectedBallWhite);
	Ptr<vector<Ball>> detectedBallBlackPtr = makePtr<vector<Ball>>(detectedBallBlack);
	Ptr<vector<Ball>> detectedBallSolidPtr = makePtr<vector<Ball>>(detectedBallSolid);
	Ptr<vector<Ball>> detectedBallStripedPtr = makePtr<vector<Ball>>(detectedBallStriped);
	double mAP = 0;
	mAP += APBallCategory(detectedBallWhitePtr, groundTruthBboxWhite, WHITE_BALL, 0.5);
	mAP += APBallCategory(detectedBallBlackPtr, groundTruthBboxBlack, BLACK_BALL, 0.5);
	mAP += APBallCategory(detectedBallSolidPtr, groundTruthBboxSolid, SOLID_BALL, 0.5);
	mAP += APBallCategory(detectedBallStripedPtr, groundTruthBboxStriped, STRIPED_BALL, 0.5);
	mAP /= 4;
	cout << "mAP: " << mAP << endl;

	// compute the mIoU
	for(int i = 0; i < segmentedFound.size(); i++){
		IoU_white += mIoUCategory(segmentedFound[i], segmentedGT[i], WHITE_BALL);
		IoU_black += mIoUCategory(segmentedFound[i], segmentedGT[i], BLACK_BALL);
		IoU_solid += mIoUCategory(segmentedFound[i], segmentedGT[i], SOLID_BALL);
		IoU_striped += mIoUCategory(segmentedFound[i], segmentedGT[i], STRIPED_BALL);
		IoU_playingField += mIoUCategory(segmentedFound[i], segmentedGT[i], PLAYING_FIELD);
		IoU_background += mIoUCategory(segmentedFound[i], segmentedGT[i], BACKGROUND);
	}
	cout << "IoU white: " << IoU_white << endl;
	cout << "IoU black: " << IoU_black << endl;
	cout << "IoU solid: " << IoU_solid << endl;
	cout << "IoU striped: " << IoU_striped << endl;
	cout << "IoU playing field: " << IoU_playingField << endl;
	cout << "IoU background: " << IoU_background << endl;
	IoU_white /= segmentedFound.size();
	IoU_black /= segmentedFound.size();
	IoU_solid /= segmentedFound.size();
	IoU_striped /= segmentedFound.size();
	IoU_playingField /= segmentedFound.size();
	IoU_background /= segmentedFound.size();
	cout << "IoU white: " << IoU_white << endl;
	cout << "IoU black: " << IoU_black << endl;
	cout << "IoU solid: " << IoU_solid << endl;
	cout << "IoU striped: " << IoU_striped << endl;
	cout << "IoU playing field: " << IoU_playingField << endl;
	cout << "IoU background: " << IoU_background << endl;
	mIoU = (IoU_white + IoU_black + IoU_solid + IoU_striped + IoU_background + IoU_playingField) / 6;
	cout << "mIoU: " << mIoU << endl;

	return 0;
}
