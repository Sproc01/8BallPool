// Author: Michele Sprocatti

#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include "ObjectDetection.h"
#include <iostream>
#include "ball.h"
#include "table.h"
#include "tracking.h"

using namespace std;
using namespace cv;



int main()
{
    Ptr<vector<Ball>> balls = new vector<Ball>;
	BallTracker tracker = BallTracker(balls); // TODO think if it is ok to already init here
    VideoCapture vid = VideoCapture("../Dataset/game1_clip1/game1_clip1.mp4");
    Mat frame;
    while (vid.isOpened())
    {
        bool ret = vid.read(frame);

        // if frame is read correctly ret is True
        if (!ret)
        {
            printf("Can't receive frame (stream end?). Exiting ... maybe end of file\n");
            break;
        }
        imshow("frame", frame);
        detectTable(frame);
        detectBalls(frame, balls);
		tracker.trackAll(frame);
        //if (waitKey(0) == 'q')
        break;

    }
    //Pipeline etc
	waitKey();
    return 0;
}
