#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
#include <filesystem>

#include "ball.h"
#include "table.h"
#include "detection.h"
#include "segmentation.h"
#include "minimapConstants.h"
#include "metrics.h"
#include "util.h"

using namespace std;
using namespace cv;

/* simple main to test in all videos the detection of the table in all the first frames
 and the detection of the balls in all the first and last frames
 and compute the performance */
int main(int argc, char* argv[]){

	Mat frame;
	Vec2b colorTable;
	Table table;
	Vec<Point2f, 4> tableCorners;
	Mat segmented;
	Mat previousFrame;
	Mat detected;
	bool ret;

	vector<string> name ={"/game1_clip1", "/game1_clip2", "/game1_clip3",
							"/game1_clip4", "/game2_clip1", "/game2_clip2",
							"/game3_clip1", "/game3_clip2", "/game4_clip1",
							"/game4_clip2"};

	// vector<string> nameOther = {"video1_768_580.mp4", "video1_1024_576.mp4", "video1_1726_1080.mp4",
	//  							"video2_1024_576.mp4", "video2_1920_1080.mp4"};

	for(int i = 0; i < name.size(); i++){
		segmented = Mat::zeros(frame.size(), CV_8UC3);
		VideoCapture vid = VideoCapture("../Dataset"+name[i]+name[i]+".mp4");
		//VideoCapture vid = VideoCapture("../Dataset/other_videos_not_deliver/" + nameOther[i]);
		vid.read(frame);
		detectTable(frame, tableCorners, colorTable);
		table = Table(tableCorners, colorTable);
		cout << "--------------" << endl;
		cout << name[i] << endl;
		cout << "------ First frame -------" << endl;
		detectBalls(frame, table, detected);
		//imshow("detected balls", detected);
		imwrite("../Output/Detection"+name[i]+"_detected_balls_first_frame.jpg", detected);
		segmentTable(frame, table, segmented);
		segmentBalls(frame, table.ballsPtr(), segmented);
		imwrite("../Output/Segmentation"+name[i]+"_segmented_balls_first_frame.jpg", segmented);
		//imshow("seg", segmented);
		compareMetrics(table, segmented, "../Dataset"+name[i], FIRST);
		//waitKey(0);
		previousFrame = frame.clone();
		ret = vid.read(frame);
		while (vid.isOpened() && ret){

			previousFrame = frame.clone();
			ret = vid.read(frame);
		}
		cout << "------ Last frame --------" << endl;
		table.clearBalls();
		detectBalls(previousFrame, table, detected);
		//imshow("detected balls", detected);
		imwrite("../Output/Detection"+name[i]+"_detected_balls_last_frame.jpg", detected);
		segmentTable(previousFrame, table, segmented);
		segmentBalls(segmented, table.ballsPtr(), segmented);
		imwrite("../Output/Segmentation"+name[i]+"_segmented_balls_last_frame.jpg", segmented);
		//imshow("seg", segmented);
		compareMetrics(table, segmented, "../Dataset"+name[i], LAST);
		//waitKey(0);
	}
	return 0;
}
