// Author: Alberto Pasqualetto

#include "table.h"

#include <stdexcept>
#include <utility>
#include <opencv2/core.hpp>

using namespace cv;

/**
 * @brief Return the boundaries of the Table.
 * @return the boudaries of the table.
 * @throw runtime_error if the boundaries are uninitialized.
 */
Vec<Point2f, 4> Table::getBoundaries() const {
	if(boundaries_ == Vec<Point2f, 4>{Point2f{0, 0}, Point2f{0, 0}, Point2f{0, 0}, Point2f{0, 0}})
		throw std::runtime_error("boundaries are uninitialized");

	return boundaries_;
}

/**
 * @brief Return the color range of the Table.
 * @return the color range of the Table expressed as the 2 V boundaries of the range in HSV format.
 * @throw runtime_error if the color is uninitialized.
 */
Vec2b Table::getColorRange() const {
	if (colorRange_ == Vec2b(0, 0))   // black for both boundaries = uninitialized
		throw std::runtime_error("color is uninitialized");

	return colorRange_;
}

/**
 * @brief Return the transformation matrix for the Table.
 * @return the transformation matrix.
 * @throw runtime_error if the transform is uninitialized.
 */
Mat Table::getTransform() const {
	if (!norm(transform_, Mat::eye(3, 3, CV_64F), NORM_L1))
		throw std::runtime_error("transform is uninitialized");

	return transform_;
}

/**
 * @brief Return a shared pointer of the vector of Ball relative to this Table.
 * @return a cv::Ptr (shared pointer) to the vector of Ball.
 */
Ptr<std::vector<Ball>> Table::ballsPtr() {
	return balls_;
}

/**
 * @brief Set the boundaries of the Table.
 * @param boundaries vector of the four boundaries of the Table.
 */
void Table::setBoundaries(const Vec<Point2f, 4> &boundaries) {
	boundaries_ = boundaries;
}

/**
 * @brief Set the color range of the Table.
 * @param colorRange color range of the Table expressed as the 2 V boundaries of the range in HSV format.
 */
void Table::setColorRange(Vec2b colorRange) { // NOLINT(*-unnecessary-value-param)
	colorRange_ = colorRange;
}

/**
 * @brief Set the transform matrix of the Table.
 * @param transform transform matrix of the Table.
 */
void Table::setTransform(const Mat &transform) {
	transform_ = std::move(transform);
}

/**
 * @brief Add a Ball to the set of balls.
 * @param ball object Ball.
 */
void Table::addBall(Ball ball) {
	balls_->push_back(ball);
}

/**
 * @brief Remove a Ball from the set of balls using its index.
 * @param index position of the Ball to remove.
 */
void Table::removeBall(int index) {
	balls_->erase(balls_->begin() + index);
}

/**
 * @brief Add a vector of Ball to the set of balls.
 * @param balls vector of Ball.
 */
void Table::addBalls(const std::vector<Ball> &balls) {
	for (auto &ball : balls){
		addBall(ball);
	}
}

/**
 * @brief Remove all balls in the Table.
 */
void Table::clearBalls() {
	balls_->clear();
}
