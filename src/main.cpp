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
	Mat frame;
	Vec2b colorTable;
	Table table;
	Vec<Point2f, 4> tableCorners;
	Mat segmented;
	int frameCount = 0;
	Mat previousFrame;
	Mat detected;
	Mat res;
	vector<double> metricsAP;
	vector<double> metricsIoU;

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
	// work on first frame
	if (!vid.isOpened() || !vid.read(frame)){
		cout << "Error opening video file" << endl;
		return -1;
	}
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
	detectTable(frame, tableCorners, colorTable);
	table = Table(tableCorners, colorTable);
	segmentTable(frame, table, segmented);
	imshow("segmentedTable", segmented);

	//DETECT AND SEGMENT BALLS
	detectBalls(frame, table, detected);
	imshow("detected balls", detected);

	segmentBalls(segmented, table.ballsPtr(), segmented);
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
	table.setTransform(computeTransformation(segmented, img_corners));
	table.setBoundaries(img_corners);

	//MINIMAP
	// The original is the png provided but we converted it to an header
	// Mat minimap = imread(MINIMAP_PATH);
	// Mat minimap_with_track = minimap.clone();
	// Mat minimap_with_balls = minimap.clone();
	vector<unsigned char> minimapVec(MINIMAP_DATA, MINIMAP_DATA + MINIMAP_DATA_SIZE);
	Mat minimap = imdecode(minimapVec, cv::IMREAD_UNCHANGED);
	Mat minimap = imread(MINIMAP_PATH);
	imshow("minimap", minimap);

	Mat transform =  table.getTransform();
	minimap_with_balls = drawMinimap(minimap_with_track, transform, table.ballsPtr());
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
		minimap_with_balls = drawMinimap(minimap_with_track, transform, table.ballsPtr());
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
	minutes duration = duration_cast<minutes>(stop - start);
	cout << "Time to create the video: " << duration.count() <<" minutes" << endl;
	vidOutput.release();

	// work on last frame
	table.clearBalls();
	detectBalls(previousFrame, table, detected);
	imshow("detected balls", detected);
	segmentTable(previousFrame, table, segmented);
	segmentBalls(segmented, table.ballsPtr(), segmented);
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
