// Author: Michele Sprocatti

#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include "objectDetection.h"
#include <iostream>
#include "ball.h"
#include "table.h"

using namespace std;
using namespace cv;

int main()
{
    vector<Ball> balls;
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
        //if (waitKey(0) == 'q')
        break;

    }
    //Pipeline etc
    return 0;
}
