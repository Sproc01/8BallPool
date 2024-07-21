// Author: Michele Sprocatti

#include <opencv2/videoio.hpp>
#include <iostream>
#include <filesystem>

#include "ball.h"
#include "table.h"
#include "detection.h"
#include "segmentation.h"
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


	double IoU_white = 0;
	double IoU_black = 0;
	double IoU_solid = 0;
	double IoU_striped = 0;
	double IoU_playingField = 0;
	double IoU_background = 0;
	double mIoU = 0;

	vector<double> metricsIoU;

	vector<string> filename ={"/game1_clip1", "/game1_clip2", "/game1_clip3",
								"/game1_clip4", "/game2_clip1", "/game2_clip2",
								"/game3_clip1", "/game3_clip2", "/game4_clip1",
								"/game4_clip2"};


	for (int i = 0; i < filename.size(); i++) {

		VideoCapture vid = VideoCapture("../Dataset"+filename[i]+filename[i]+".mp4");
		vid.read(frame);
		detectTable(frame, tableCorners, colorTable);
		table = Table(tableCorners, colorTable);
		detectBalls(frame, table, detected);
		segmentTable(frame, table, segmented);
		segmentBalls(frame, table.ballsPtr(), segmented);

		// save the results mAP
		separateResultBalls(table.ballsPtr(), detectedBallWhite, detectedBallBlack, detectedBallSolid, detectedBallStriped);
		gt = readGroundTruthBboxFile("../Dataset"+filename[i]+"/bounding_boxes/frame_first_bbox.txt");
		separateResultGT(gt, groundTruthBboxWhite, groundTruthBboxBlack, groundTruthBboxSolid, groundTruthBboxStriped);

		// save result mIOU
		metricsIoU = compareMetricsIoU(segmented, "../Dataset"+filename[i], FIRST);
		IoU_background += metricsIoU[0];
		IoU_white += metricsIoU[1];
		IoU_black += metricsIoU[2];
		IoU_solid += metricsIoU[3];
		IoU_striped += metricsIoU[4];
		IoU_playingField += metricsIoU[5];


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

		// save the results mAP
		separateResultBalls(table.ballsPtr(), detectedBallWhite, detectedBallBlack, detectedBallSolid, detectedBallStriped);
		gt = readGroundTruthBboxFile("../Dataset"+filename[i]+"/bounding_boxes/frame_last_bbox.txt");
		separateResultGT(gt, groundTruthBboxWhite, groundTruthBboxBlack, groundTruthBboxSolid, groundTruthBboxStriped);

		// save result mIOU
		metricsIoU = compareMetricsIoU(segmented, "../Dataset"+filename[i], LAST);
		IoU_background += metricsIoU[0];
		IoU_white += metricsIoU[1];
		IoU_black += metricsIoU[2];
		IoU_solid += metricsIoU[3];
		IoU_striped += metricsIoU[4];
		IoU_playingField += metricsIoU[5];
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
	IoU_white /= filename.size()*2;
	IoU_black /= filename.size()*2;
	IoU_solid /= filename.size()*2;
	IoU_striped /= filename.size()*2;
	IoU_playingField /= filename.size()*2;
	IoU_background /= filename.size()*2;
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
