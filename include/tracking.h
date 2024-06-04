// Author: Alberto Pasqualetto

#ifndef TRACKING_H
#define TRACKING_H

#include "ball.h"
#include <opencv2/tracking.hpp>
#include <vector>


class BallTracker {
private:
	std::vector<cv::Ptr<cv::Tracker>> ballTrackers_;
	std::vector<Ball> ballsVec_;
	bool isInitialized_;

	void createTrackers();

public:
	BallTracker(const std::vector<Ball> &balls);

	cv::Rect trackOne(unsigned short ballIndex, const cv::Mat &frame, bool callInit);

	std::vector<Ball> trackAll(const cv::Mat &frame);
};


#endif // TRACKING_H
