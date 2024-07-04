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
	if (argc != 2){
		cout << "Usage: " << argv[0] << " <video_path>" << endl;
//		return -1;  // TODO re add
		argv[1] = "../Dataset/game1_clip1/game1_clip1.mp4"; // TODO remove
	}
	filesystem::path videoPath = filesystem::path(argv[1]);


	string pathOutput = "../Output/video.mp4";
	Table table;
	vector<Ball> balls;
	Vec<Point2f, 4> tableCorners;
	Mat segmented;
	Mat segmentedFrame;
	int frameCount = 0;
	Mat res;

	// vector<string> name ={"/game1_clip1/game1_clip1.mp4", "/game1_clip2/game1_clip2.mp4", "/game1_clip3/game1_clip3.mp4",
	// 						"/game1_clip4/game1_clip4.mp4", "/game2_clip1/game2_clip1.mp4", "/game2_clip2/game2_clip2.mp4",
	// 						"/game3_clip1/game3_clip1.mp4", "/game3_clip2/game3_clip2.mp4", "/game4_clip1/game4_clip1.mp4",
	// 						"/game4_clip2/game4_clip2.mp4"};
	// Mat frame;
	// Vec2b colorTable;
	// for(int i = 0; i < name.size(); i++)
	// {
	// 	segmented = Mat::zeros(frame.size(), CV_8UC3);
	// 	balls.clear();
	// 	VideoCapture vid = VideoCapture("../Dataset"+name[i]);
	// 	vid.read(frame);
	// 	detectTable(frame, tableCorners, colorTable);
	// 	//cout << "new image" << endl;
	// 	detectBalls(frame, balls, tableCorners, colorTable);
	// 	// segmentTable(frame, tableCorners, colorTable, segmented);
	// 	// // imshow("segT", segmented);
	// 	// segmentBalls(frame, balls, segmented);
	// 	// imshow("segB", segmented);
	// 	// waitKey(0);
	// }

	VideoCapture vid = VideoCapture("../Dataset/game1_clip1/game1_clip1.mp4");
	Mat frame;
	Vec2b colorTable;

	// TODO work on first frame
	if (!vid.isOpened() || !vid.read(frame)){
	 	cout << "Error opening video file" << endl;
	 	return -1;
	}
	++frameCount;
	imshow("First frame", frame);
	int codec = VideoWriter::fourcc('m', 'p', '4', 'v');
	VideoWriter vidOutput = VideoWriter();
	double fps = vid.get(CAP_PROP_FPS);
	vidOutput.open(pathOutput, codec, fps, frame.size(), 1);
	detectTable(frame, tableCorners, colorTable);
	table = Table(tableCorners, colorTable);
	segmentTable(frame, tableCorners, colorTable, segmented);
	// imshow("segmentedTable", segmented);
	detectBalls(frame, balls, tableCorners, colorTable);
	// TODO better manage using table.ballsPtr()
	table.addBalls(balls);
	segmentBalls(segmented, balls, segmented);
	// imshow("segmentedBalls", segmented);

	//compute transformation
	Vec<Point2f, 4>  img_corners = table.getBoundaries();
	table.setTransform(computeTransformation(frame, img_corners));
	table.setBoundaries(img_corners);

	Mat minimap = imread(MINIMAP_PATH);
	Mat minimap_with_track = minimap.clone();
	Mat minimap_with_balls = minimap.clone();
//	vector<unsigned char> minimapVec(MINIMAP_DATA, MINIMAP_DATA + MINIMAP_DATA_SIZE);
//	Mat minimap = imdecode(minimapVec, cv::IMREAD_UNCHANGED);
//	Mat minimap = imread(MINIMAP_PATH);
//	imshow("minimap", minimap);

	//compute transformation

	//create minimap with balls
	Mat transform;
	table.getTransform(transform); //TODO: getTranform(transform)?
	minimap_with_balls = drawMinimap(minimap_with_track, transform, *table.ballsPtr());
	imshow("Minimap with balls", minimap_with_balls);

	BallTracker tracker = BallTracker(table.ballsPtr());
	tracker.trackAll(frame);
	createOutputImage(frame, minimap_with_balls, res);
	//imshow("result", res);
	vidOutput.write(res);
	// TODO calculate metrics using videoPath.file_parent()
	waitKey(0);

	auto start = high_resolution_clock::now();
	while (vid.isOpened()){  // work on middle frames
	 	bool ret = vid.read(frame);

	 	// if frame is read correctly ret is True
	 	if (!ret){
	 		printf("Can't receive frame (stream end?). Exiting ... maybe end of file\n");
	 		break;
	 	}
	 	++frameCount;

	 	cout << "Frame number: " << frameCount << endl;
		//imshow("frame " + std::to_string(frameCount), frame);
 		// detectTable(frame, tableCorners, colorTable);
		// waitKey(0);
 		// detectBalls(frame, balls, tableCorners, colorTable);
 		// segmentTable(frame, tableCorners, colorTable, segmentedFrame);
		// segmentBalls(segmentedFrame, balls);
 		// imshow("segmentedFrame", segmentedFrame);
 		// if (waitKey(0) == 'q')
		// 	return 0;

		tracker.trackAll(frame);
		//tempMinimap = minimap.clone();
		minimap_with_balls = drawMinimap(minimap_with_track, transform, *table.ballsPtr());


		if((frameCount % 10) == 0) {
			imshow("frame " + to_string(frameCount), frame);
			imshow("Minimap with balls " + to_string(frameCount), minimap_with_balls);
			waitKey(0);
		}

		createOutputImage(frame, minimap_with_balls, res);
		// imshow("result", res);
		vidOutput.write(res);
		// if (!(frameCount % 30))
		// 	waitKey();
		// TODO write frame on video
	}
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<seconds>(stop - start);
	cout << duration.count() << endl;
	vidOutput.release();

        // 	// TODO work on last frame

 	waitKey(0);
 	return 0;
}
