// Author: Alberto Pasqualetto

#ifndef TRACKING_H
#define TRACKING_H

#include "ball.h"
#include <opencv2/tracking.hpp>
#include <vector>

/**
 * @brief Class that tracks all the balls in the input image, it relies on OpenCV TrackerCSRT class.
 */
class BallTracker {
	std::vector<cv::Ptr<cv::Tracker>> ballTrackers_;	// vector of OpenCV Tracker objects, one for each ball. The index in the vector is the same as the index in ballsVec_.
	cv::Ptr<std::vector<Ball>> ballsVec_;	// pointer to the vector of balls to track.
	bool isInitialized_;	// flag that indicates if the trackers have already been initialized.

	/**
	 * @brief Create the trackers for all the balls in the vector.
	 */
	void createTrackers();

public:
	/**
	 * @brief Constructor.
	 * @param balls pointer to the vector of balls to track.
	 */
	explicit BallTracker(cv::Ptr<std::vector<Ball>> balls);

	/**
	 * @brief Track the ball with the given index in the input frame.
	 * The returned bounding box is not updated if the IoU with the previous one is too high.
	 * @param ballIndex index of the ball to track.
	 * @param frame input frame.
	 * @param callInit flag that indicates if the tracker has to be initialized.
	 * @return the bounding box of the tracked ball.
	 */
	cv::Rect trackOne(unsigned short ballIndex, const cv::Mat &frame, bool callInit = false);

	/**
	 * @brief Track all the balls in the input frame.
	 * A returned bounding box is not updated if the IoU with the previous one is too high.
	 * @param frame input frame.
	 * @return a pointer to the vector of the tracked balls. It is the same as the one provided to the constructor.
	 */
	cv::Ptr<std::vector<Ball>> trackAll(const cv::Mat &frame);
};

#endif // TRACKING_H
