// Author: Michele Sprocatti

#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
#include <filesystem>
#include <chrono>

//#include "../img/minimap.h"
#include "ball.h"
#include "table.h"
#include "detection.h"
#include "segmentation.h"
#include "transformation.h"
#include "minimapConstants.h"
#include "tracking.h"
#include "metrics.h"
#include "util.h"

using namespace std;
using namespace cv;
using namespace chrono;

//TODO: remove folders Dataset/other_videos and output_detection

/* 	Given a video it detects table and balls in the first frame and track the balls over different frame.
	Using this information then it creates the output video with a minimap superimposed and then detect the balls
	in the last frame. For the detection of the table and of the balls it computes also some performance metrics. */
int main(int argc, char* argv[]){

	//VARIABLES
	filesystem::path videoPath;
	filesystem::path outputPath = "../Output";
	Mat frame;
	Vec2b colorTable;
	Table table;
	Vec<Point2f, 4> tableCorners;
	Mat segmented;
	int frameCount = 0;
	Mat lastFrame;
	Mat detected;
	Mat res;
	bool toRotate, toResize;
	short leftBorderLength, rightBorderLength;
	vector<double> metricsAP;
	vector<double> metricsIoU;

	//INPUT
	// TODO rotate image if vertical; resize to be inscribed in current sizes, centered in the Mat; use this to calculate minimap; write to video file the original unrotated, unscaled image with the calculated image superimposed
	if (argc == 2){
		videoPath = filesystem::path(argv[1]);
	}
	else if (argc == 1) { //TODO: remove at the end
		videoPath = filesystem::path("../Dataset/game1_clip1/game1_clip1.mp4");
		videoPath = filesystem::path("../Dataset/other_videos_not_deliver/game1_clip1_vertical.mp4");
	}
	else {
		cout << "Error of number of parameters: insert one parameter" << endl;
		return -1;
	}
	cout << "Video path: " << videoPath << endl;

	//START THE VIDEO
	VideoCapture vid = VideoCapture(videoPath.string());
	// work on first frame
	if (!vid.isOpened() || !vid.read(frame)){
		cout << "Error opening video file" << endl;
		return -1;
	}

	const unsigned short ORIGINAL_WIDTH = frame.cols;
	const unsigned short ORIGINAL_HEIGHT = frame.rows;

	calculateInscriptionParameters(frame, TABLE_WIDTH, TABLE_HEIGHT, toRotate, toResize, leftBorderLength, rightBorderLength);

	string videoName = videoPath.stem().string();
	string outputVideoName = videoName + "_output.mp4";
	outputPath = outputPath / outputVideoName;
	++frameCount;
	//imshow("First frame", frame);

	filesystem::path tempOutputPath = filesystem::temp_directory_path() / outputVideoName;
	int codec = VideoWriter::fourcc('m', 'p', '4', 'v');
	VideoWriter vidOutput = VideoWriter();
	double fps = vid.get(CAP_PROP_FPS);
	vidOutput.open(tempOutputPath.string(), codec, fps, frame.size(), true);

	//DETECT AND SEGMENT TABLE
	doInscript(frame, TABLE_WIDTH, TABLE_HEIGHT, toRotate, toResize, leftBorderLength, rightBorderLength);
	detectTable(frame, tableCorners, colorTable);
	table = Table(tableCorners, colorTable);
	segmentTable(frame, table, segmented);
	undoInscript(segmented, ORIGINAL_WIDTH, ORIGINAL_HEIGHT, toRotate, toResize, leftBorderLength, rightBorderLength);
	imshow("segmentedTable", segmented);

	//DETECT AND SEGMENT BALLS
	detectBalls(frame, table, detected);
	undoInscript(detected, ORIGINAL_WIDTH, ORIGINAL_HEIGHT, toRotate, toResize, leftBorderLength, rightBorderLength);
	imshow("detectedBalls", detected);

	segmentBalls(segmented, table.ballsPtr(), segmented);
	undoInscript(segmented, ORIGINAL_WIDTH, ORIGINAL_HEIGHT, toRotate, toResize, leftBorderLength, rightBorderLength);
	imshow("segmentedBalls", segmented);
	cout << "Metrics first frame:" << endl;
	metricsAP = compareMetricsAP(table, videoPath.parent_path().string(), FIRST);
	metricsIoU = compareMetricsIoU(segmented, videoPath.parent_path().string(), FIRST);
	for(int i = 0; i < metricsAP.size(); i++)
		cout << "AP for category " << i+1 << ": " << metricsAP[i] << endl;

	for(int i = 0; i < metricsIoU.size(); i++)
		cout << "IoU for category " << i << ": " << metricsIoU[i] << endl;


	//TRANSFORMATION
	Vec<Point2f, 4>  img_corners = table.getBoundaries();
	doInscript(segmented, TABLE_WIDTH, TABLE_HEIGHT, toRotate, toResize, leftBorderLength, rightBorderLength);
	table.setTransform(computeTransformation(segmented, img_corners));
	table.setBoundaries(img_corners);

	//MINIMAP
	Mat minimap = imread(MINIMAP_PATH);
	Mat minimap_with_track = minimap.clone();
	Mat minimap_with_balls = minimap.clone();
	//TODO use minimap.h
	// vector<unsigned char> minimapVec(MINIMAP_DATA, MINIMAP_DATA + MINIMAP_DATA_SIZE);
	// Mat minimap = imdecode(minimapVec, cv::IMREAD_UNCHANGED);
	// Mat minimap = imread(MINIMAP_PATH);
	// imshow("minimap", minimap);

	Mat transform =  table.getTransform();
	minimap_with_balls = drawMinimap(minimap_with_track, transform, table.ballsPtr());
	//imshow("Minimap with balls", minimap_with_balls);
	undoInscript(frame, ORIGINAL_WIDTH, ORIGINAL_HEIGHT, toRotate, toResize, leftBorderLength, rightBorderLength);
	createOutputImage(frame, minimap_with_balls, res);
	//imshow("result", res);
	vidOutput.write(res);

	//TRACKER
	BallTracker tracker = BallTracker(table.ballsPtr());
	doInscript(frame, TABLE_WIDTH, TABLE_HEIGHT, toRotate, toResize, leftBorderLength, rightBorderLength);
	tracker.trackAll(frame);
	// waitKey();

	//VIDEO WITH MINIMAP
	time_point start = high_resolution_clock::now();

	while (vid.isOpened() && vid.read(frame)){  // work on middle frames
		//cout << "Frame number: " << ++frameCount << endl;

		doInscript(frame, TABLE_WIDTH, TABLE_HEIGHT, toRotate, toResize, leftBorderLength, rightBorderLength);

 		//VIDEO WITH MINIMAP
		tracker.trackAll(frame);
		minimap_with_balls = drawMinimap(minimap_with_track, transform, table.ballsPtr());

		undoInscript(frame, ORIGINAL_WIDTH, ORIGINAL_HEIGHT, toRotate, toResize, leftBorderLength, rightBorderLength);
		createOutputImage(frame, minimap_with_balls, res);
		//imshow("result", res);
		vidOutput.write(res);

		lastFrame = frame.clone();
	}
	time_point stop = high_resolution_clock::now();
	minutes duration = duration_cast<minutes>(stop - start);
	cout << "Time to create the video: " << duration.count() <<" minutes" << endl;
	vidOutput.release();

	// work on last frame
	doInscript(lastFrame, TABLE_WIDTH, TABLE_HEIGHT, toRotate, toResize, leftBorderLength, rightBorderLength);
	table.clearBalls();
	detectBalls(lastFrame, table, detected);
	segmentTable(lastFrame, table, segmented);
	undoInscript(detected, ORIGINAL_WIDTH, ORIGINAL_HEIGHT, toRotate, toResize, leftBorderLength, rightBorderLength);
	imshow("detected balls", detected);
	segmentBalls(segmented, table.ballsPtr(), segmented);
	undoInscript(segmented, ORIGINAL_WIDTH, ORIGINAL_HEIGHT, toRotate, toResize, leftBorderLength, rightBorderLength);
	imshow("segmentedBalls", segmented);
	cout << "Metrics last frame:" << endl;
	metricsAP = compareMetricsAP(table, videoPath.parent_path().string(), LAST);
	metricsIoU = compareMetricsIoU(segmented, videoPath.parent_path().string(), LAST);

	for(int i = 0; i < metricsAP.size(); i++)
		cout << "AP for category " << static_cast<Category>(i+1) << ": " << metricsAP[i] << endl;

	for(int i = 0; i < metricsIoU.size(); i++)
		cout << "IoU for category " << static_cast<Category>(i) << ": " << metricsIoU[i] << endl;
	waitKey(0);
	// write to a temp file first, then rename to the final name
	filesystem::copy(tempOutputPath, outputPath, filesystem::copy_options::overwrite_existing);
	filesystem::remove(tempOutputPath);
	return 0;
}
