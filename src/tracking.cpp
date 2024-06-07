// Author: Alberto Pasqualetto

#include "tracking.h"
#include "ball.h"
#include <opencv2/tracking.hpp>
#include <iostream>


BallTracker::BallTracker(cv::Ptr<std::vector<Ball>> balls) { // NOLINT(*-unnecessary-value-param)
	std::cout<<"constructor balltracker"<<std::endl;
	isInitialized_ = false;

	ballsVec_ = balls;

//	ballsVec_.shrink_to_fit();
	ballTrackers_.reserve(ballsVec_->size());   // TODO at the moment this is useless
}


void BallTracker::createTrackers() {
	for (unsigned short i = 0; i < ballsVec_->size(); i++) {
		cv::Ptr<cv::Tracker> tracker = cv::TrackerCSRT::create();    //parameters go here if necessary
		ballTrackers_.push_back(tracker);
	}

	ballTrackers_.shrink_to_fit();
	std::cout<<"trackers created"<<std::endl;
}


cv::Rect BallTracker::trackOne(unsigned short ballIndex, const cv::Mat &frame, bool callInit) {
	cv::Rect bbox = ballsVec_->at(ballIndex).getBbox();
	ballsVec_->at(ballIndex).setBbox_prec(bbox);

	bool isBboxUpdated = false;

	if (callInit) {
		ballTrackers_[ballIndex]->init(frame, bbox);
	} else {
		isBboxUpdated = ballTrackers_[ballIndex]->update(frame, bbox);
		ballsVec_->at(ballIndex).setBbox(bbox);
	}
	std::cout<< "Ball " << ballIndex << " updated? " << isBboxUpdated << " current bbox: " << bbox << std::endl;

	return bbox;


//		return cv::Point(bbox.x + bbox.width / 2, bbox.y + bbox.height / 2);
}


cv::Ptr<std::vector<Ball>> BallTracker::trackAll(const cv::Mat &frame) {
	std::cout<<"trackAll, initialized: "<<isInitialized_<<std::endl;
	if (!isInitialized_) {
		createTrackers();
	}

	for (unsigned short i = 0; i < ballsVec_->size(); i++) {
		if (!isInitialized_){
			trackOne(i, frame, true);
		}
		else {
			trackOne(i, frame, false);
		}
	}

	return ballsVec_;
}

