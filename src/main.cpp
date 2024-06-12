// Author: Michele Sprocatti

#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>

#include "ball.h"
#include "table.h"
#include "detection.h"
#include "segmentation.h"
#include "transformation.h"
#include "minimapConstants.h"
#include "tracking.h"

using namespace std;
using namespace cv;



int main()
{
	Table table;
	vector<Ball> balls;
	vector<Point> tableCorners;
	Mat segmented;
	Mat segmentedFrame;
	int frameCount = 0;
	VideoCapture vid = VideoCapture("../Dataset/game1_clip1/game1_clip1.mp4");
	Mat frame;

	// TODO work on first frame
	if (!vid.isOpened() || !vid.read(frame)){
		cout << "Error opening video file" << endl;
		return -1;
	}
	++frameCount;
	imshow("First frame", frame);
	table = detectTable(frame, tableCorners);
	segmentTable(frame, tableCorners, segmented);
	//imshow("segmentedTable", segmented);
	detectBalls(frame, balls, tableCorners);
	// TODO better manage using table.ballsPtr()
	table.addBalls(balls);
	segmentBalls(segmented, balls, segmented);
	//imshow("segmentedBalls", segmented);

	Mat minimap = imread(MINIMAP_PATH);
	Mat tempMinimap = minimap.clone();  // TODO minimap always draws over the same image
	//imshow("minimap", minimap);

	//create minimap with balls
	Mat minimap_with_balls = minimapWithBalls(tempMinimap, table, frame);
	imshow("Minimap with balls", minimap_with_balls);

	BallTracker tracker = BallTracker(table.ballsPtr());
	tracker.trackAll(frame);

	waitKey();
//    return 0;

	while (vid.isOpened()){  // work on middle frames
		bool ret = vid.read(frame);

		// if frame is read correctly ret is True
		if (!ret){
			printf("Can't receive frame (stream end?). Exiting ... maybe end of file\n");
			break;
		}
		++frameCount;

		cout << "Frame number: " << frameCount << endl;
//		imshow("frame " + std::to_string(frameCount), frame);
		//detectTable(frame, tableCorners);
		//detectBalls(frame, balls, tableCorners);
//		segmentedFrame = frame.clone();
		// segmentTable(segmentedFrame, tableCorners);
		// segmentBalls(segmentedFrame, balls);
		// imshow("segmentedFrame", segmentedFrame);
		//if (waitKey(0) == 'q')
		//waitKey(0);

		tracker.trackAll(frame);
		if (!(frameCount % 10)){
			tempMinimap = minimap.clone();
			minimap_with_balls = minimapWithBalls(tempMinimap, table, frame);
			imshow("frame " + to_string(frameCount), frame);
			imshow("Minimap with balls " + to_string(frameCount), minimap_with_balls);

			if (!(frameCount % 30))
				waitKey();
//			waitKey();
		}

//		break;
	}

	// TODO work on last frame

	waitKey();
	return 0;
}

