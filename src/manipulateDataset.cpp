#include <stdexcept>
#include <filesystem>
#include <utility>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <fstream>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <opencv2/imgproc.hpp>

#include "category.h"

using namespace std;
using namespace cv;

vector<pair<Rect, Category>> readGroundTruthBbox(const string &filename) {
	ifstream file(filename);
	if (!file.is_open()){
		throw invalid_argument("File not found");
	}

	vector<pair<Rect, Category>> bboxes;
	string line;
	while (getline(file, line)){
		istringstream iss(line);
		int x, y, w, h;
		short cat;
		iss >> x >> y >> w >> h >> cat;
		bboxes.push_back(make_pair(Rect(x, y, w, h), static_cast<Category>(cat)));
	}

	return bboxes;
}


/**
 * @brief Find the point in the 90° clockwise rotated image corresponding to the input point.
 * @param point point with still to be rotated image coordinates.
 * @param destImg destination image (used to get its size).
 * @return rotated Point2f.
 */
Point2f rotatePoint(Point2f point, const Mat &destImg) {
	float old_row = point.y;
	point.y = point.x;
	point.x = destImg.cols - old_row;

	return point;
}

/**
 * @brief Find the point in the 90° counter-clockwise rotated image corresponding to the input point.
 * @param point point with still to be "unrotated" image coordinates.
 * @param destImg destination image (used to get its size).
 * @return "unrotated" Point2f.
 */
Point2f unrotatePoint(Point2f point, const Mat &destImg) {
	float old_row = point.y;
	point.y = destImg.rows - point.x;
	point.x = old_row;

	return point;
}

/**
 * @brief Find the same rect in the 90° clockwise rotated image corresponding to the input point.
 * @param rect Rect with still to be rotated image coordinates.
 * @param destImg destination image (used to get its size).
 * @return rotated Rect.
 */
Rect rotateRect(Rect rect, const Mat &destImg) {
	Point2f oldTR(rect.tl().x + rect.width, rect.tl().y);
	Point2f oldBL(rect.tl().x, rect.tl().y + rect.height);

	return Rect(rotatePoint(oldTR, destImg), rotatePoint(oldBL, destImg));
}

/**
 * @brief Find the same rect in the 90° counter-clockwise rotated image corresponding to the input point.
 * @param rect Rect with still to be unrotated image coordinates.
 * @param destImg destination image (used to get its size).
 * @return "unrotated" Rect.
 */
Rect unrotateRect(Rect rect, const Mat &destImg) {
	Point2f oldTR(rect.tl().x + rect.width, rect.tl().y);
	Point2f oldBL(rect.tl().x, rect.tl().y + rect.height);

	return Rect(unrotatePoint(oldTR, destImg), unrotatePoint(oldBL, destImg));
}

/*void rotateBboxesFile() {
	Mat vertImg(1024, 576, CV_8UC3, Scalar(0, 0, 0));
	Mat horImg(576, 1024, CV_8UC3, Scalar(0, 0, 0));

	// string filename = "../Dataset/game1_clip1_vertical/bounding_boxes/frame_first_bbox.txt";
	string filename = "../Dataset/game1_clip1_vertical/bounding_boxes/frame_last_bbox.txt";

	vector<pair<Rect,Category>> gts = readGroundTruthBbox(filename);

	vector<pair<Rect,Category>> rotated;

	for (auto gt : gts){
		cout << "Rect: " << gt.first << " Category: " << gt.second << endl;
		rotated.push_back(make_pair(rotateRect(gt.first, vertImg), gt.second));
		// rotated.push_back(make_pair(unrotateRect(gt.first, horImg), gt.second));
		cout<< "Rotated: " << rotated.back().first << " Category: " << rotated.back().second << endl;
	}

	// write to file one bounding box per line
	ofstream file(filename);
	for (auto r : rotated){
		file << r.first.x << " " << r.first.y << " " << r.first.width << " " << r.first.height << " " << static_cast<int>(r.second) << endl;
	}
}*/

void rotateVideo() {
	// Clockwise
	filesystem::path filename("../Dataset/game1_clip1_576_1024/game1_clip1_576_1024.mp4");

	// VIDEO
	VideoCapture in(filename.string());
	Mat frame;
	if (!in.isOpened() || !in.read(frame)){
		throw invalid_argument("Video not found");
	}

	Mat destImg(Size(frame.rows, frame.cols), CV_8UC3, Scalar(0, 0, 0));
	VideoWriter out(filename.string()+"temp.mp4", VideoWriter::fourcc('m', 'p', '4', 'v'),  in.get(CAP_PROP_FPS), destImg.size());

	do {
		rotate(frame, frame, ROTATE_90_CLOCKWISE);
		out.write(frame);
	} while(in.read(frame));

	in.release();
	out.release();
	cout<<filename.string()+"temp.mp4"<<endl;
	filesystem::remove(filename);
	filesystem::rename(filename.string()+"temp.mp4", filename);


	// IMAGES
	string frame_first_filename = (filename.parent_path() / "frames" / "frame_first.png").string();
	string frame_last_filename = (filename.parent_path() / "frames" / "frame_last.png").string();
	string mask_first_filename = (filename.parent_path() / "masks" / "frame_first.png").string();
	string mask_last_filename = (filename.parent_path() / "masks" / "frame_last.png").string();

	Mat frame_first = imread(frame_first_filename);
	Mat frame_last = imread(frame_last_filename);
	Mat mask_first = imread(mask_first_filename, IMREAD_GRAYSCALE);
	Mat mask_last = imread(mask_last_filename, IMREAD_GRAYSCALE);

	rotate(frame_first, frame_first, ROTATE_90_CLOCKWISE);
	rotate(frame_last, frame_last, ROTATE_90_CLOCKWISE);
	rotate(mask_first, mask_first, ROTATE_90_CLOCKWISE);
	rotate(mask_last, mask_last, ROTATE_90_CLOCKWISE);

	imwrite(frame_first_filename, frame_first);
	imwrite(frame_last_filename, frame_last);
	imwrite(mask_first_filename, mask_first);
	imwrite(mask_last_filename, mask_last);


	// BBOXES
	string bbox_first_filename = (filename.parent_path() / "bounding_boxes" / "frame_first_bbox.txt").string();
	string bbox_last_filename = (filename.parent_path() / "bounding_boxes" / "frame_last_bbox.txt").string();

	vector<pair<Rect,Category>> gts_first = readGroundTruthBbox(bbox_first_filename);
	vector<pair<Rect,Category>> gts_last = readGroundTruthBbox(bbox_last_filename);

	for (auto &gt : gts_first) {
		gt.first = rotateRect(gt.first, destImg);
	}

	for (auto &gt : gts_last) {
		gt.first = rotateRect(gt.first, destImg);
	}

	ofstream bbox_first_file(bbox_first_filename);
	for (auto c : gts_first){
		bbox_first_file << c.first.x << " " << c.first.y << " " << c.first.width << " " << c.first.height << " " << static_cast<int>(c.second) << endl;
	}

	ofstream bbox_last_file(bbox_last_filename);
	for (auto c : gts_last){
		bbox_last_file << c.first.x << " " << c.first.y << " " << c.first.width << " " << c.first.height << " " << static_cast<int>(c.second) << endl;
	}
}


void cropVideo () {
	Size outputSize(900, 576);
	filesystem::path filename("../Dataset/game1_clip1_900_576/game1_clip1_900_576.mp4");

	// VIDEO
	VideoCapture in(filename.string());
	if (!in.isOpened()){
		throw invalid_argument("Video not found");
	}

	VideoWriter out(filename.string()+"temp.mp4", VideoWriter::fourcc('m', 'p', '4', 'v'),  in.get(CAP_PROP_FPS), outputSize);

	Mat frame;
	Mat prevFrame;
	while (in.read(frame)) {
		out.write(frame(Rect((frame.cols-outputSize.width)/2, (frame.rows-outputSize.height)/2, outputSize.width, outputSize.height)));
		frame.copyTo(prevFrame);
	}

	in.release();
	out.release();
	cout<<filename.string()+"temp.mp4"<<endl;
	filesystem::remove(filename);
	filesystem::rename(filename.string()+"temp.mp4", filename);

	// IMAGES
	string frame_first_filename = (filename.parent_path() / "frames" / "frame_first.png").string();
	string frame_last_filename = (filename.parent_path() / "frames" / "frame_last.png").string();
	string mask_first_filename = (filename.parent_path() / "masks" / "frame_first.png").string();
	string mask_last_filename = (filename.parent_path() / "masks" / "frame_last.png").string();

	Mat frame_first = imread(frame_first_filename);
	Mat frame_last = imread(frame_last_filename);
	Mat mask_first = imread(mask_first_filename, IMREAD_GRAYSCALE);
	Mat mask_last = imread(mask_last_filename, IMREAD_GRAYSCALE);

	imwrite(frame_first_filename, frame_first(Rect((frame_first.cols-outputSize.width)/2, (prevFrame.rows-outputSize.height)/2, outputSize.width, outputSize.height)));
	imwrite(frame_last_filename, frame_last(Rect((frame_last.cols-outputSize.width)/2, (prevFrame.rows-outputSize.height)/2, outputSize.width, outputSize.height)));
	imwrite(mask_first_filename, mask_first(Rect((mask_first.cols-outputSize.width)/2, (prevFrame.rows-outputSize.height)/2, outputSize.width, outputSize.height)));
	imwrite(mask_last_filename, mask_last(Rect((mask_last.cols-outputSize.width)/2, (prevFrame.rows-outputSize.height)/2, outputSize.width, outputSize.height)));


	// BBOXES
	string bbox_first_filename = (filename.parent_path() / "bounding_boxes" / "frame_first_bbox.txt").string();
	string bbox_last_filename = (filename.parent_path() / "bounding_boxes" / "frame_last_bbox.txt").string();

	vector<pair<Rect,Category>> gts_first = readGroundTruthBbox(bbox_first_filename);
	vector<pair<Rect,Category>> gts_last = readGroundTruthBbox(bbox_last_filename);

	for (auto &gt : gts_first) {
		gt.first.x -= (frame_first.cols-outputSize.width)/2;
		gt.first.y -= (frame_first.rows-outputSize.height)/2;
	}

	for (auto &gt : gts_last) {
		gt.first.x -= (frame_last.cols-outputSize.width)/2;
		gt.first.y -= (frame_last.rows-outputSize.height)/2;
	}

	ofstream bbox_first_file(bbox_first_filename);
	for (auto c : gts_first){
		bbox_first_file << c.first.x << " " << c.first.y << " " << c.first.width << " " << c.first.height << " " << static_cast<int>(c.second) << endl;
	}

	ofstream bbox_last_file(bbox_last_filename);
	for (auto c : gts_last){
		bbox_last_file << c.first.x << " " << c.first.y << " " << c.first.width << " " << c.first.height << " " << static_cast<int>(c.second) << endl;
	}
}


void resizeVideo () {
	float ratio = 2;
	filesystem::path filename("../Dataset/game1_clip1_2048_1152/game1_clip1_2048_1152.mp4");

	// VIDEO
	VideoCapture in(filename.string());
	Mat frame;
	if (!in.isOpened() || !in.read(frame)){
		throw invalid_argument("Video not found");
	}

	VideoWriter out(filename.string()+"temp.mp4", VideoWriter::fourcc('m', 'p', '4', 'v'),  in.get(CAP_PROP_FPS), Size(frame.cols*ratio, frame.rows*ratio));

	do {
		resize(frame, frame, Size(frame.cols*ratio, frame.rows*ratio));
		out.write(frame);
	} while (in.read(frame));

	in.release();
	out.release();
	cout<<filename.string()+"temp.mp4"<<endl;
	filesystem::remove(filename);
	filesystem::rename(filename.string()+"temp.mp4", filename);


	// IMAGES
	string frame_first_filename = (filename.parent_path() / "frames" / "frame_first.png").string();
	string frame_last_filename = (filename.parent_path() / "frames" / "frame_last.png").string();
	string mask_first_filename = (filename.parent_path() / "masks" / "frame_first.png").string();
	string mask_last_filename = (filename.parent_path() / "masks" / "frame_last.png").string();

	Mat frame_first = imread(frame_first_filename);
	Mat frame_last = imread(frame_last_filename);
	Mat mask_first = imread(mask_first_filename, IMREAD_GRAYSCALE);
	Mat mask_last = imread(mask_last_filename, IMREAD_GRAYSCALE);

	resize(frame_first, frame_first, Size(frame_first.cols*ratio, frame_first.rows*ratio));
	resize(frame_last, frame_last, Size(frame_last.cols*ratio, frame_last.rows*ratio));
	resize(mask_first, mask_first, Size(mask_first.cols*ratio, mask_first.rows*ratio));
	resize(mask_last, mask_last, Size(mask_last.cols*ratio, mask_last.rows*ratio));

	imwrite(frame_first_filename, frame_first);
	imwrite(frame_last_filename, frame_last);
	imwrite(mask_first_filename, mask_first);
	imwrite(mask_last_filename, mask_last);


	// BBOXES
	string bbox_first_filename = (filename.parent_path() / "bounding_boxes" / "frame_first_bbox.txt").string();
	string bbox_last_filename = (filename.parent_path() / "bounding_boxes" / "frame_last_bbox.txt").string();

	vector<pair<Rect,Category>> gts_first = readGroundTruthBbox(bbox_first_filename);
	vector<pair<Rect,Category>> gts_last = readGroundTruthBbox(bbox_last_filename);

	for (auto &gt : gts_first) {
		gt.first.x *= ratio;
		gt.first.y *= ratio;
		gt.first.width *= ratio;
		gt.first.height *= ratio;
	}

	for (auto &gt : gts_last) {
		gt.first.x *= ratio;
		gt.first.y *= ratio;
		gt.first.width *= ratio;
		gt.first.height *= ratio;
	}

	ofstream bbox_first_file(bbox_first_filename);
	for (auto c : gts_first){
		bbox_first_file << c.first.x << " " << c.first.y << " " << c.first.width << " " << c.first.height << " " << static_cast<int>(c.second) << endl;
	}

	ofstream bbox_last_file(bbox_last_filename);
	for (auto c : gts_last){
		bbox_last_file << c.first.x << " " << c.first.y << " " << c.first.width << " " << c.first.height << " " << static_cast<int>(c.second) << endl;
	}
}

int main() {
	/* PREPARE FOLDERS AND VIDEO FILES WITH CHANGED NAMES FIRST */
	rotateVideo();
	// cropVideo();
	// resizeVideo();
	return 0;
}
