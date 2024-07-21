// Author: Michele Sprocatti

#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
#include <filesystem>
#include <chrono>

#include "minimap.h"
#include "ball.h"
#include "table.h"
#include "detection.h"
#include "segmentation.h"
#include "transformation.h"
#include "tracking.h"
#include "metrics.h"
#include "util.h"

using namespace std;
using namespace cv;
using namespace chrono;

/* 	Given a video, it detects table and balls in the first frame and tracks the balls over different frames.
	Using this information then it creates the output video with a minimap superimposed and then detects the balls
	in the last frame. For the detection of the table and of the balls it computes also some performance metrics. */
int main(int argc, char *argv[]) {
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
	const int FRAME_VISUALITAION_STEP = 60;

	//INPUT
	if (argc == 2) {
		videoPath = filesystem::path(argv[1]);
	}
	else {
		cout << "Error of number of parameters: insert one parameter" << endl;
		return -1;
	}
	cout << "Video path: " << videoPath << endl;

	//START THE VIDEO
	VideoCapture vid = VideoCapture(videoPath.string());

	// work on first frame
	if (!vid.isOpened() || !vid.read(frame)) {
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
	//imshow("segmentedTable", segmented);

	//DETECT AND SEGMENT BALLS
	detectBalls(frame, table);
	drawBoundingBoxes(frame, table, detected);
	imshow("detected balls first frame", detected);

	segmentBalls(segmented, table.ballsPtr(), segmented);
	imshow("segmented balls first frame", segmented);
	cout << "Metrics first frame:" << endl;
	metricsAP = compareMetricsAP(table, videoPath.parent_path().string(), FIRST);
	metricsIoU = compareMetricsIoU(segmented, videoPath.parent_path().string(), FIRST);
	for (int c = 0; c < metricsAP.size(); c++)
		cout << "AP for category " << c + 1 << ": " << metricsAP[c] << endl;

	for (int c = 0; c < metricsIoU.size(); c++)
		cout << "IoU for category " << c << ": " << metricsIoU[c] << endl;

	waitKey(0);

	//TRANSFORMATION
	Vec<Point2f, 4> imgCorners = table.getBoundaries();
	table.setTransform(computeTransformation(segmented, imgCorners));
	table.setBoundaries(imgCorners);

	//MINIMAP
	// The original is the png provided but we converted it to an header
	// Mat minimap = imread(MINIMAP_PATH);
	vector<unsigned char> minimapVec(MINIMAP_DATA, MINIMAP_DATA + MINIMAP_DATA_SIZE);
	Mat minimap = imdecode(minimapVec, IMREAD_COLOR);


	Mat minimapWithTrack = minimap.clone();
	Mat minimapWithBalls = minimap.clone();
	// imshow("minimap", minimap);

	Mat transform = table.getTransform();
	minimapWithBalls = drawMinimap(minimapWithTrack, transform, table.ballsPtr());
	//imshow("Minimap with balls", minimapWithBalls);
	createOutputImage(frame, minimapWithBalls, res);
	//imshow("result", res);
	vidOutput.write(res);

	//TRACKER
	BilliardTracker tracker = BilliardTracker(table.ballsPtr());
	tracker.trackAll(frame);

	//VIDEO WITH MINIMAP
	time_point start = high_resolution_clock::now();
	bool ret = vid.read(frame);
	while (vid.isOpened() && ret) { // work on middle frames

		++frameCount;
 		//VIDEO WITH MINIMAP
		tracker.trackAll(frame);
		minimapWithBalls = drawMinimap(minimapWithTrack, transform, table.ballsPtr());
		createOutputImage(frame, minimapWithBalls, res);
		//imshow("result", res);
		vidOutput.write(res);
		// show status every X frame
		if (frameCount % FRAME_VISUALITAION_STEP == 0) {
			// enlarge and shrink are needed because for the tracking
			// we enlarge the bounding box to have a better detection
			for(int i = 0; i < table.ballsPtr()->size(); i++){
				Rect r = table.ballsPtr()->at(i).getBbox();
				shrinkRect(r, 10);
				table.ballsPtr()->at(i).setBbox(r);
			}
			segmentTable(frame, table, segmented);
			segmentBalls(segmented, table.ballsPtr(), segmented);
			drawBoundingBoxes(frame, table, detected);
			//imshow("frame " + to_string(frameCount), frame);
			imshow("segmented balls " + to_string(frameCount) + " frame", segmented);
			imshow("detected balls " + to_string(frameCount) + " frame", detected);
			imshow("Minimap with balls " + to_string(frameCount) + " frame", minimapWithBalls);
			for(int i = 0; i < table.ballsPtr()->size(); i++){
				Rect r = table.ballsPtr()->at(i).getBbox();
				enlargeRect(r, 10);
				table.ballsPtr()->at(i).setBbox(r);
			}
			waitKey(0);
		}

		previousFrame = frame.clone();
		ret = vid.read(frame);
	}

	time_point stop = high_resolution_clock::now();
	minutes duration = duration_cast<minutes>(stop - start);
	cout << "Time to create the video: " << duration.count() << " minutes" << endl;
	vidOutput.release();

	imwrite("../Output/minimap/" + videoName + "_minimap.png", minimapWithBalls);

	// work on last frame
	table.clearBalls();
	detectBalls(previousFrame, table);
	drawBoundingBoxes(previousFrame, table, detected);
	imshow("detected balls last frame", detected);
	segmentTable(previousFrame, table, segmented);
	segmentBalls(segmented, table.ballsPtr(), segmented);
	imshow("segmented balls last frame", segmented);
	cout << "Metrics last frame:" << endl;
	metricsAP = compareMetricsAP(table, videoPath.parent_path().string(), LAST);
	metricsIoU = compareMetricsIoU(segmented, videoPath.parent_path().string(), LAST);

	for (int c = 0; c < metricsAP.size(); c++)
		cout << "AP for category " << c + 1 << ": " << metricsAP[c] << endl;

	for (int c = 0; c < metricsIoU.size(); c++)
		cout << "IoU for category " << c << ": " << metricsIoU[c] << endl;

	// write to a temp file first, then rename to the final name
	filesystem::copy(tempOutputPath, outputPath, filesystem::copy_options::overwrite_existing);
	filesystem::remove(tempOutputPath);
	waitKey(0);
	return 0;
}
