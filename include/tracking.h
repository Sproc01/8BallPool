// Author: Alberto Pasqualetto

#ifndef TRACKING_H
#define TRACKING_H

#include "ball.h"
#include <opencv2/tracking.hpp>
#include <vector>


class BallTracker {
	// Index in ballsVec_ is the same as the index in ballTrackers_
	std::vector<cv::Ptr<cv::Tracker>> ballTrackers_;
	cv::Ptr<std::vector<Ball>> ballsVec_;
	bool isInitialized_;

	void createTrackers();

public:
	explicit BallTracker(cv::Ptr<std::vector<Ball>> balls);

	cv::Rect trackOne(unsigned short ballIndex, const cv::Mat &frame, bool callInit);

	cv::Ptr<std::vector<Ball>> trackAll(const cv::Mat &frame);
};

void enlargeRect(cv::Rect &rect, int factor);

#endif // TRACKING_H
