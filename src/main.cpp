// Author: Michele Sprocatti

#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
#include <filesystem>
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

#include <chrono>
using namespace std::chrono;

using namespace std;
using namespace cv;

int main(int argc, char* argv[]){

	//VARIABLES
	filesystem::path videoPath;
	string pathOutput;
	Mat frame;
	Vec2b colorTable;
	Table table;
	vector<Ball> balls;
	Vec<Point2f, 4> tableCorners;
	Mat segmented;
	Mat segmentedFrame;
	int frameCount = 0;
	Mat res;

	//INPUT
	if (argc == 2){
		videoPath = filesystem::path(argv[1]);
	}
	else if (argc == 1) { //TODO: remove at the end
		videoPath = filesystem::path("../Dataset/game4_clip1/game4_clip1.mp4");
	}
	else {
		cout << "Error of number of parameters: insert one parameter" << endl;
		return -1;
	}
	cout << "Video path: " << videoPath << endl;


	vector<string> name ={"/game1_clip1/game1_clip1.mp4", "/game1_clip2/game1_clip2.mp4", "/game1_clip3/game1_clip3.mp4",
							"/game1_clip4/game1_clip4.mp4", "/game2_clip1/game2_clip1.mp4", "/game2_clip2/game2_clip2.mp4",
							"/game3_clip1/game3_clip1.mp4", "/game3_clip2/game3_clip2.mp4", "/game4_clip1/game4_clip1.mp4",
							"/game4_clip2/game4_clip2.mp4"};

	for(int i = 0; i < name.size(); i++)
	{
		segmented = Mat::zeros(frame.size(), CV_8UC3);
		balls.clear();
		VideoCapture vid = VideoCapture("../Dataset"+name[i]);
		vid.read(frame);
		detectTable(frame, tableCorners, colorTable);
		table = Table(tableCorners, colorTable);
		detectBalls(frame, balls, tableCorners, colorTable);
		table.addBalls(balls);
		segmentTable(frame, tableCorners, colorTable, segmented);
		segmentBalls(frame, balls, segmented);
		imshow("seg", segmented);
		waitKey(0);
	}

	//START THE VIDEO
	VideoCapture vid = VideoCapture(videoPath.string());
	// TODO work on first frame
	if (!vid.isOpened() || !vid.read(frame)){
		cout << "Error opening video file" << endl;
		return -1;
	}
	string videoName = videoPath.stem().string();
	pathOutput = "../Output/" + videoName + "_output.mp4";
	++frameCount;
	//TODO: check output in frame when the resolution is bigger (it is cropped)
	imshow("First frame", frame);
	int codec = VideoWriter::fourcc('m', 'p', '4', 'v');
	VideoWriter vidOutput = VideoWriter();
	double fps = vid.get(CAP_PROP_FPS);
	//TODO: remove the video if some error occour, or if the execution is closed before end (it is corrupted)
	vidOutput.open(pathOutput, codec, fps, frame.size(), true);

	// //DETECT AND SEGMENT TABLE
	// detectTable(frame, tableCorners, colorTable);
	// table = Table(tableCorners, colorTable);
	// segmentTable(frame, tableCorners, colorTable, segmented);
	// // imshow("segmentedTable", segmented);

	// //DETECT AND SEGMENT BALLS
	// detectBalls(frame, balls, tableCorners, colorTable);
	// // TODO better manage using table.ballsPtr()
	// table.addBalls(balls);
	// segmentBalls(segmented, balls, segmented);
	// // imshow("segmentedBalls", segmented);

	// //TRANSFORMATION
	// Vec<Point2f, 4>  img_corners = table.getBoundaries();
	// table.setTransform(computeTransformation(frame, img_corners));
	// table.setBoundaries(img_corners);

	// //MINIMAP
	// Mat minimap = imread(MINIMAP_PATH);
	// Mat minimap_with_track = minimap.clone();
	// Mat minimap_with_balls = minimap.clone();
	//	vector<unsigned char> minimapVec(MINIMAP_DATA, MINIMAP_DATA + MINIMAP_DATA_SIZE);
	//	Mat minimap = imdecode(minimapVec, cv::IMREAD_UNCHANGED);
	//	Mat minimap = imread(MINIMAP_PATH);
	//	imshow("minimap", minimap);

	Mat transform;
	table.getTransform(transform); //TODO: getTranform(transform)?
	minimap_with_balls = drawMinimap(minimap_with_track, transform, *table.ballsPtr());
	imshow("Minimap with balls", minimap_with_balls);

	//TRACKER
	BallTracker tracker = BallTracker(table.ballsPtr());
	tracker.trackAll(frame);
	createOutputImage(frame, minimap_with_balls, res);
	//imshow("result", res);
	vidOutput.write(res);
	// TODO calculate metrics using videoPath.file_parent()
	waitKey(0);

	//VIDEO WITH MINIMAP
	auto start = high_resolution_clock::now();
	while (vid.isOpened()){  // work on middle frames
	 	bool ret = vid.read(frame);
		//cout << "Frame number: " << ++frameCount << endl;

// 	//TRACKER
// 	BallTracker tracker = BallTracker(table.ballsPtr());
// 	tracker.trackAll(frame);
// 	createOutputImage(frame, minimap_with_balls, res);
// 	//imshow("result", res);
// 	vidOutput.write(res);
// 	// TODO calculate metrics using videoPath.file_parent()
// 	waitKey(0);

// 	//VIDEO WITH MINIMAP
// 	auto start = high_resolution_clock::now();
// 	while (vid.isOpened()){  // work on middle frames
// 	 	bool ret = vid.read(frame);

// // 	BallTracker tracker = BallTracker(table.ballsPtr());
// // 	tracker.trackAll(frame);
// // 	createOutputImage(frame, minimap_with_balls, res);
// // 	//imshow("result", res);
// // 	vidOutput.write(res);
// // 	// TODO calculate metrics using videoPath.file_parent()

// // 	while (vid.isOpened()){  // work on middle frames
// // 	 	bool ret = vid.read(frame);

// 		tracker.trackAll(frame);
// 		minimap_with_balls = drawMinimap(minimap_with_track, transform, *table.ballsPtr());

// 		/*
// 		//show minimap status every 10 frame
// 		if((frameCount % 10) == 0) {
// 			imshow("frame " + to_string(frameCount), frame);
// 			imshow("Minimap with balls " + to_string(frameCount), minimap_with_balls);
// 			waitKey(0);
// 		}
// 		*/

// 		createOutputImage(frame, minimap_with_balls, res);
// 		// imshow("result", res);
// 		vidOutput.write(res);

// 		// TODO write frame on video
// 	}
// 	auto stop = high_resolution_clock::now();
// 	auto duration = duration_cast<minutes>(stop - start);
// 	cout << "Time to create the video: " << duration.count() <<" minutes" << endl;
// 	vidOutput.release();

	// TODO work on last frame

// 		tracker.trackAll(frame);
// 		tempMinimap = minimap.clone();
// 		minimap_with_balls = minimapWithBalls(tempMinimap, table, frame);
// 		// imshow("frame " + to_string(frameCount), frame);
// 		// imshow("Minimap with balls " + to_string(frameCount), minimap_with_balls);
// 		createOutputImage(frame, minimap_with_balls, res);
// 		// imshow("result", res);
// 		vidOutput.write(res);
// 		// if (!(frameCount % 30))
// 		// 	waitKey();
// 		// TODO write frame on video
// 	}
// 	vidOutput.release();

// // 	// TODO work on last frame

//  	waitKey(0);

	return 0;
}
