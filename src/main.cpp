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

using namespace std;
using namespace cv;


int main(int argc, char* argv[]){
	if (argc != 2){
		cout << "Usage: " << argv[0] << " <video_path>" << endl;
//		return -1;  // TODO re add
		argv[1] = "../Dataset/game1_clip1/game1_clip1.mp4"; // TODO remove
	}
	filesystem::path videoPath = filesystem::path(argv[1]);


	Table table;
	vector<Ball> balls;
	Vec<Point2f, 4> tableCorners;
	Mat segmented;
	Mat segmentedFrame;
	int frameCount = 0;
	// vector<string> name ={"/game1_clip1/game1_clip1.mp4", "/game1_clip2/game1_clip2.mp4", "/game1_clip3/game1_clip3.mp4",
	// 						"/game1_clip4/game1_clip4.mp4", "/game2_clip1/game2_clip1.mp4", "/game2_clip2/game2_clip2.mp4",
	// 						"/game3_clip1/game3_clip1.mp4", "/game3_clip2/game3_clip2.mp4", "/game4_clip1/game4_clip1.mp4",
	// 						"/game4_clip2/game4_clip2.mp4"};
	// Mat frame;
	// Vec2b colorTable;
	// for(int i = 0; i < name.size(); i++)
	// {
	// 	VideoCapture vid = VideoCapture("../Dataset"+name[i]);
	// 	vid.read(frame);
	// 	detectTable(frame, tableCorners, colorTable);
	// 	segmentTable(frame, tableCorners, colorTable, segmented);
	// 	waitKey(0);
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
	detectTable(frame, tableCorners, colorTable);
	table = Table(tableCorners, colorTable);
	segmentTable(frame, tableCorners, colorTable, segmented);
	imshow("segmentedTable", segmented);
	detectBalls(frame, balls, tableCorners, colorTable);
	// TODO better manage using table.ballsPtr()
	table.addBalls(balls);
	segmentBalls(segmented, balls, segmented);
	imshow("segmented", segmented);
	waitKey(0);
	double s = mIoUSegmentation(segmented,"../Dataset/game1_clip1/masks/frame_first.png");
	cout << s<< endl;
	imshow("segmentedBalls", segmented);
	waitKey(0);


//	std::vector<unsigned char> minimapVec(MINIMAP_DATA, MINIMAP_DATA + MINIMAP_DATA_SIZE);
//	Mat minimap = cv::imdecode(minimapVec, cv::IMREAD_UNCHANGED);
//	 Mat minimap = imread(MINIMAP_PATH);

	// Mat tempMinimap = minimap.clone();  // TODO minimapData always draws over the same image
	// create minimap with balls
	// Mat minimap_with_balls = minimapWithBalls(tempMinimap, table, frame);
	// imshow("Minimap with balls", minimap_with_balls);

	// BallTracker tracker = BallTracker(table.ballsPtr());
	// tracker.trackAll(frame);

	// TODO calculate metrics using videoPath.file_parent()

	// waitKey();

	// while (vid.isOpened()){  // work on middle frames
	// 	bool ret = vid.read(frame);

	// 	// if frame is read correctly ret is True
	// 	if (!ret){
	// 		printf("Can't receive frame (stream end?). Exiting ... maybe end of file\n");
	// 		break;
	// 	}
	// 	++frameCount;

	// 	cout << "Frame number: " << frameCount << endl;
// //		imshow("frame " + std::to_string(frameCount), frame);
// 		//detectTable(frame, tableCorners);
// 		//detectBalls(frame, balls, tableCorners);
// //		segmentedFrame = frame.clone();
// 		// segmentTable(segmentedFrame, tableCorners);
// 		// segmentBalls(segmentedFrame, balls);
// 		// imshow("segmentedFrame", segmentedFrame);
// 		//if (waitKey(0) == 'q')
// 		//waitKey(0);

		// tracker.trackAll(frame);
		// if (!(frameCount % 30)){
		// 	tempMinimap = minimap.clone();
		// 	minimap_with_balls = minimapWithBalls(tempMinimap, table, frame);
		// 	imshow("frame " + to_string(frameCount), frame);
		// 	imshow("Minimap with balls " + to_string(frameCount), minimap_with_balls);
		// 	if (!(frameCount % 30))
		// 		waitKey();
		// }

		// TODO write frame on video
	//}

// 	// TODO work on last frame

// 	waitKey();
 	return 0;
}

