#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
#include <filesystem>
//#include "../img/minimap.h"

#include "ball.h"
#include "table.h"
#include "detection.h"
#include "segmentation.h"
#include "minimapConstants.h"
#include "metrics.h"
#include "util.h"

using namespace std;
using namespace cv;

int main(int argc, char* argv[])
{
	Mat frame;
	Vec2b colorTable;
	Table table;
	vector<Ball> balls;
	Vec<Point2f, 4> tableCorners;
	Mat segmented;


	vector<string> name ={"/game1_clip1", "/game1_clip2", "/game1_clip3",
							"/game1_clip4", "/game2_clip1", "/game2_clip2",
							"/game3_clip1", "/game3_clip2", "/game4_clip1",
							"/game4_clip2"};

	for(int i = 0; i < name.size(); i++)
	{
		segmented = Mat::zeros(frame.size(), CV_8UC3);
		balls.clear();
		VideoCapture vid = VideoCapture("../Dataset"+name[i]+name[i]+".mp4");
		vid.read(frame);
		detectTable(frame, tableCorners, colorTable);
		table = Table(tableCorners, colorTable);
		cout << "--------------" << endl;
		detectBalls(frame, table, balls);
		table.addBalls(balls);
		segmentTable(frame, table, segmented);
		segmentBalls(frame, balls, segmented);
		imshow("seg", segmented);
		compareMetrics(table, segmented, "../Dataset"+name[i], FIRST);
		waitKey(0);
	}
	return 0;
}
