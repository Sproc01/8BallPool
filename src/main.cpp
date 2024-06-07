// Author: Michele Sprocatti

#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include "detection.h"
#include <iostream>
#include "ball.h"
#include "table.h"
#include "segment.h"

using namespace std;
using namespace cv;

int main()
{
	Table table;
    vector<Ball> balls;
    vector<Point> tableCorners;
    Mat segmented;
    Mat segmentedFrame;
    VideoCapture vid = VideoCapture("../Dataset/game1_clip1/game1_clip1.mp4");
    Mat frame;

	// TODO work on first frame
	if (!vid.isOpened() || !vid.read(frame)){
		cout << "Error opening video file" << endl;
		return -1;
	}

	imshow("First frame", frame);
	detectTable(frame, tableCorners);
    segmentTable(frame, tableCorners, segmented);
    //imshow("segmentedTable", segmented);
	detectBalls(frame, balls, tableCorners); // TODO change to table.getBalls()
    segmentBalls(segmented, balls, segmented);
    //imshow("segmentedBalls", segmented);
    waitKey(0);
    return 0;

    // while (vid.isOpened())  // work on middle frames
    // {
    //     bool ret = vid.read(frame);

    //     // if frame is read correctly ret is True
    //     if (!ret)
    //     {
    //         printf("Can't receive frame (stream end?). Exiting ... maybe end of file\n");
    //         break;
    //     }
    //     imshow("frame", frame);
    //     //detectTable(frame, tableCorners);
    //     //detectBalls(frame, balls, tableCorners);
    //     segmentedFrame = frame.clone();
    //     // segmentTable(segmentedFrame, tableCorners);
    //     // segmentBalls(segmentedFrame, balls);
    //     // imshow("segmentedFrame", segmentedFrame);
    //     //if (waitKey(0) == 'q')
    //     //waitKey(0);
    //     break;

    // }

	// // TODO work on last frame

    // return 0;
}
