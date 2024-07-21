// Author: Alberto Pasqualetto

#include "tracking.h"
#include "ball.h"
#include <opencv2/tracking.hpp>
#include <iostream>
#include "metrics.h"

using namespace cv;

BallTracker::BallTracker(Ptr<std::vector<Ball>> balls) { // NOLINT(*-unnecessary-value-param)
	//std::cout<<"constructor balltracker"<<std::endl;
	isInitialized_ = false;

	ballsVec_ = balls;

//	ballsVec_.shrink_to_fit();
	ballTrackers_.reserve(ballsVec_->size());
}


void BallTracker::createTrackers() {
	for (unsigned short i = 0; i < ballsVec_->size(); i++) {
		Ptr<Tracker> tracker = TrackerCSRT::create();    //parameters go here if necessary
		ballTrackers_.push_back(tracker);
	}

	ballTrackers_.shrink_to_fit();
	//std::cout<<"trackers created"<<std::endl;
}


Rect BallTracker::trackOne(unsigned short ballIndex, const Mat &frame, bool callInit) {
	Rect bbox = ballsVec_->at(ballIndex).getBbox();
	ballsVec_->at(ballIndex).setBbox_prec(bbox);

	bool isBboxUpdated = false;
	if (callInit) {
		enlargeRect(bbox, 10);  // enlarge bbox to enhance tracking performance
		ballTrackers_[ballIndex]->init(frame, bbox);
	} else {
		if(ballsVec_->at(ballIndex).getVisibility())
		{
			isBboxUpdated = ballTrackers_[ballIndex]->update(frame, bbox);
			const float IOU_THRESHOLD = 0.7;
			if (isBboxUpdated && IoU(ballsVec_->at(ballIndex).getBbox_prec(), bbox) > IOU_THRESHOLD) {  // if IoU is too high, do not update: the shift is not significant
				isBboxUpdated = false;
			} else {
				ballsVec_->at(ballIndex).setBbox(bbox); // do not update if shift is too little (use IoU)
			}
		}
	}
	//std::cout<< "Ball " << ballIndex << " updated? " << isBboxUpdated << " current bbox: " << bbox << std::endl;

	return bbox;

//		return cv::Point(bbox.x + bbox.width / 2, bbox.y + bbox.height / 2);
}


Ptr<std::vector<Ball>> BallTracker::trackAll(const Mat &frame) {
	//std::cout<<"trackAll, initialized: "<<isInitialized_<<std::endl;
	if (!isInitialized_) {
		createTrackers();
		for (unsigned short i = 0; i < ballsVec_->size(); i++) {
			trackOne(i, frame, true);
		}
		isInitialized_ = true;
	} else {
		for (unsigned short i = 0; i < ballsVec_->size(); i++) {
			trackOne(i, frame, false);
		}
	}

	return ballsVec_;
}


void enlargeRect(Rect &rect, int factor) {
	rect.x -= factor;
	rect.y -= factor;
	rect.width += 2 * factor;
	rect.height += 2 * factor;
}
