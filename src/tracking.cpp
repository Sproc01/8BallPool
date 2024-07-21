// Author: Alberto Pasqualetto

#include "tracking.h"
#include "ball.h"
#include <opencv2/tracking.hpp>
#include <iostream>
#include "metrics.h"
#include "util.h"

using namespace cv;

/**
 * @brief Constructor.
 * @param balls pointer to the vector of balls to track.
 */
BilliardTracker::BilliardTracker(Ptr<std::vector<Ball>> balls) { // NOLINT(*-unnecessary-value-param)
	//std::cout<<"constructor balltracker"<<std::endl;
	isInitialized_ = false;

	ballsVec_ = balls;

	ballTrackers_.reserve(ballsVec_->size());
}


/**
 * @brief Create the trackers for all the balls in the vector.
 * Used the first time tracker is called.
*/
void BilliardTracker::createTrackers() {
	for (unsigned short i = 0; i < ballsVec_->size(); i++) {
		Ptr<Tracker> tracker = TrackerCSRT::create();    //parameters go here if necessary
		ballTrackers_.push_back(tracker);
	}

	ballTrackers_.shrink_to_fit();
	//std::cout<<"trackers created"<<std::endl;
}


/**
 * @brief Track the ball with the given index in the input frame.
 * It performs OpenCV Tracker initialization the first time it is called.
 * The returned bounding box is not updated if the IoU with the previous one is too high.
 * @param ballIndex index of the ball to track.
 * @param frame input frame.
 * @param callInit flag that indicates if the tracker has to be initialized.
 * @return the bounding box of the tracked ball.
 */
Rect BilliardTracker::trackOne(unsigned short ballIndex, const Mat &frame, bool callInit /*= false*/) {
	Rect bbox = ballsVec_->at(ballIndex).getBbox();
	ballsVec_->at(ballIndex).setBbox_prec(bbox);

	bool isBboxUpdated = false;
	if (callInit) {
		enlargeRect(bbox, 10);  // enlarge bbox to enhance tracking performance
		ballTrackers_[ballIndex]->init(frame, bbox);
	} else {
		if(ballsVec_->at(ballIndex).getVisibility())	// track only visible balls
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
}


/**
 * @brief Track all the balls in the input frame.
 * It relies on TrackOne.
 * The returned bounding boxes are not updated if the IoU with the previous one is too high.
 * @param frame input frame.
 * @return a pointer to the vector of the tracked balls. It is the same as the one provided to the constructor.
 */
Ptr<std::vector<Ball>> BilliardTracker::trackAll(const Mat &frame) {
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
