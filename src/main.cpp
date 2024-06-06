// Author: Michele Sprocatti

#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include "objectDetection.h"
#include <iostream>
#include "ball.h"
#include "table.h"
#include "segment.h"

using namespace std;
using namespace cv;

int main()
{
    vector<Ball> balls;
    vector<Point> tableCorners;
    Mat segmentedFrame;
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
        detectTable(frame, tableCorners);
        detectBalls(frame, balls, tableCorners);
        segmentedFrame = frame.clone();
        // segmentTable(segmentedFrame, tableCorners);
        // segmentBalls(segmentedFrame, balls);
        // imshow("segmentedFrame", segmentedFrame);
        //if (waitKey(0) == 'q')
        //waitKey(0);
        break;

    }
    //Pipeline etc
    return 0;
}
