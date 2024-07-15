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

/* 	Given a video it detects table and balls in the first frame and track the balls over different frame.
	Using this information then it creates the output video with a minimap superimposed and then detect the balls
	in the last frame. For the detection of the table and of the balls it computes also some performance metrics. */
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
	int frameCount = 0;
	Mat previousFrame;
	Mat res;

	//INPUT
	if (argc == 2){
		videoPath = filesystem::path(argv[1]);
	}
	else if (argc == 1) { //TODO: remove at the end
		videoPath = filesystem::path("../Dataset/game1_clip1/game1_clip1.mp4");
	}
	else {
		cout << "Error of number of parameters: insert one parameter" << endl;
		return -1;
	}
	cout << "Video path: " << videoPath << endl;

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
	//imshow("First frame", frame);
	int codec = VideoWriter::fourcc('m', 'p', '4', 'v');
	VideoWriter vidOutput = VideoWriter();
	double fps = vid.get(CAP_PROP_FPS);
	//TODO: remove the video if some error occour, or if the execution is closed before end (it is corrupted)
	vidOutput.open(pathOutput, codec, fps, frame.size(), true);

	//DETECT AND SEGMENT TABLE
	detectTable(frame, tableCorners, colorTable);
	table = Table(tableCorners, colorTable);
	segmentTable(frame, table, segmented);
	// imshow("segmentedTable", segmented);

	//DETECT AND SEGMENT BALLS
	detectBalls(frame, table, balls);
	// TODO better manage using table.ballsPtr()
	table.addBalls(balls);
	segmentBalls(segmented, balls, segmented);
	// imshow("segmentedBalls", segmented);
	cout << "Metrics first frame:" << endl;
	compareMetrics(table, segmented, videoPath.parent_path(), FIRST);


	//TRANSFORMATION
	Vec<Point2f, 4>  img_corners = table.getBoundaries();
	table.setTransform(computeTransformation(frame, segmented, img_corners));
	table.setBoundaries(img_corners);

	//MINIMAP
	Mat minimap = imread(MINIMAP_PATH);
	Mat minimap_with_track = minimap.clone();
	Mat minimap_with_balls = minimap.clone();
	// vector<unsigned char> minimapVec(MINIMAP_DATA, MINIMAP_DATA + MINIMAP_DATA_SIZE);
	// Mat minimap = imdecode(minimapVec, cv::IMREAD_UNCHANGED);
	// Mat minimap = imread(MINIMAP_PATH);
	// imshow("minimap", minimap);

	Mat transform;
	table.getTransform(transform); //TODO: getTranform(transform)?
	minimap_with_balls = drawMinimap(minimap_with_track, transform, *table.ballsPtr());
	//imshow("Minimap with balls", minimap_with_balls);
	createOutputImage(frame, minimap_with_balls, res);
	//imshow("result", res);
	vidOutput.write(res);

	//TRACKER
	BallTracker tracker = BallTracker(table.ballsPtr());
	tracker.trackAll(frame);

	//VIDEO WITH MINIMAP
	time_point start = high_resolution_clock::now();
	bool ret = vid.read(frame);
	while (vid.isOpened() && ret){  // work on middle frames
		//cout << "Frame number: " << ++frameCount << endl;

 		//VIDEO WITH MINIMAP
		tracker.trackAll(frame);
		minimap_with_balls = drawMinimap(minimap_with_track, transform, *table.ballsPtr());
		createOutputImage(frame, minimap_with_balls, res);
		//imshow("result", res);
		vidOutput.write(res);
		/*
		// show minimap status every 10 frame
		if((frameCount % 10) == 0) {
			imshow("frame " + to_string(frameCount), frame);
			imshow("Minimap with balls " + to_string(frameCount), minimap_with_balls);
			waitKey(0);
		}
		*/
		//waitKey(0);
		previousFrame = frame.clone();
		ret = vid.read(frame);
	}
	time_point stop = high_resolution_clock::now();
	chrono::minutes duration = duration_cast<minutes>(stop - start);
	cout << "Time to create the video: " << duration.count() <<" minutes" << endl;
	vidOutput.release();

	// work on last frame
	balls.clear();
	table.clearBalls();
	detectBalls(previousFrame, table, balls);
	table.addBalls(balls);
	segmentTable(previousFrame, table, segmented);
	segmentBalls(segmented, balls, segmented);
	imshow("segmentedBalls", segmented);
	cout << "Metrics last frame:" << endl;
	compareMetrics(table, segmented, videoPath.parent_path(), LAST);
	waitKey(0);
	return 0;
}
