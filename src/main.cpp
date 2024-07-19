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

/* 	Given a video it detects table and balls in the first frame and track the balls over different frame.
	Using this information then it creates the output video with a minimap superimposed and then detect the balls
	in the last frame. For the detection of the table and of the balls it computes also some performance metrics. */
int main(int argc, char* argv[]){

	//VARIABLES
	filesystem::path videoPath;
	filesystem::path outputPath = "../Output";
	Mat originalFrame, workingFrame;	// originalFrame is the frame read from the video frob which the output is build, workingFrame is the frame to work on, which will be rotated and rescaled if necessary
	Vec2b colorTable;
	Table table;
	Vec<Point2f, 4> tableCorners;
	Mat segmented;
	int frameCount = 0;
	Mat lastFrame;
	Mat detected;
	Mat res;
	bool isRotated = false;

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
	if (!vid.isOpened() || !vid.read(originalFrame)){
		cout << "Error opening video file" << endl;
		return -1;
	}

	workingFrame = originalFrame.clone();
	inscriptInHorizontalFrame(workingFrame, TABLE_WIDTH, TABLE_HEIGHT);
	imshow("Inscripted horizontal frame", workingFrame);

	string videoName = videoPath.stem().string();
	string outputVideoName = videoName + "_output.mp4";
	outputPath = outputPath / outputVideoName;
	++frameCount;
	//TODO: check output in frame when the resolution is bigger (it is cropped)
	//imshow("First frame", frame);
	//TODO use minimap.h
	filesystem::path tempOutputPath = filesystem::temp_directory_path() / outputVideoName;
	int codec = VideoWriter::fourcc('m', 'p', '4', 'v');
	VideoWriter vidOutput = VideoWriter();
	double fps = vid.get(CAP_PROP_FPS);
	vidOutput.open(tempOutputPath.string(), codec, fps, originalFrame.size(), true);

	//DETECT AND SEGMENT TABLE
	detectTable(workingFrame, tableCorners, colorTable);
	table = Table(tableCorners, colorTable);
	segmentTable(workingFrame, table, segmented);
	imshow("segmentedTable", segmented);

	//DETECT AND SEGMENT BALLS
	detectBalls(workingFrame, table, detected);
	imshow("detectedBalls", detected);

	segmentBalls(segmented, table.ballsPtr(), segmented);
	imshow("segmentedBalls", segmented);
	cout << "Metrics first frame:" << endl;

	// compareMetrics(table, segmented, videoPath.parent_path().string(), FIRST);

	//TRANSFORMATION
	Vec<Point2f, 4>  img_corners = table.getBoundaries();
	table.setTransform(computeTransformation(segmented, img_corners));
	table.setBoundaries(img_corners);

	//MINIMAP
	Mat minimap = imread(MINIMAP_PATH);
	Mat minimap_with_track = minimap.clone();
	Mat minimap_with_balls = minimap.clone();
	// vector<unsigned char> minimapVec(MINIMAP_DATA, MINIMAP_DATA + MINIMAP_DATA_SIZE);
	// Mat minimap = imdecode(minimapVec, cv::IMREAD_UNCHANGED);
	// Mat minimap = imread(MINIMAP_PATH);
	// imshow("minimap", minimap);

	Mat transform =  table.getTransform(); //TODO: change and return value (check if working)
	minimap_with_balls = drawMinimap(minimap_with_track, transform, table.ballsPtr());
	//imshow("Minimap with balls", minimap_with_balls);
	createOutputImage(originalFrame, minimap_with_balls, res);
	//imshow("result", res);
	vidOutput.write(res);

	//TRACKER
	BallTracker tracker = BallTracker(table.ballsPtr());
	tracker.trackAll(workingFrame);
	// waitKey();

	//VIDEO WITH MINIMAP
	time_point start = high_resolution_clock::now();

	while (vid.isOpened() && vid.read(originalFrame)){  // work on middle frames
		//cout << "Frame number: " << ++frameCount << endl;
		workingFrame = originalFrame.clone();
		inscriptInHorizontalFrame(workingFrame, TABLE_WIDTH, TABLE_HEIGHT);

 		//VIDEO WITH MINIMAP
		tracker.trackAll(workingFrame);
		minimap_with_balls = drawMinimap(minimap_with_track, transform, table.ballsPtr());
		createOutputImage(originalFrame, minimap_with_balls, res);
		//imshow("result", res);
		vidOutput.write(res);

		lastFrame = originalFrame.clone();
	}
	time_point stop = high_resolution_clock::now();
	minutes duration = duration_cast<minutes>(stop - start);
	cout << "Time to create the video: " << duration.count() <<" minutes" << endl;
	vidOutput.release();

	// work on last frame
	workingFrame = originalFrame.clone();
	inscriptInHorizontalFrame(workingFrame, TABLE_WIDTH, TABLE_HEIGHT);
	table.clearBalls();
	detectBalls(lastFrame, table, detected);
	segmentTable(lastFrame, table, segmented);
	imshow("detected balls", detected);
	segmentBalls(segmented, table.ballsPtr(), segmented);
	imshow("segmentedBalls", segmented);
	cout << "Metrics last frame:" << endl;
	// compareMetrics(table, segmented, videoPath.parent_path().string(), LAST);

	// write to a temp file first, then rename to the final name only if everything went well
	filesystem::rename(tempOutputPath, outputPath);

	waitKey(0);

	return 0;
}
