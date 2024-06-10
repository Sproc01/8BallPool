// Author: Alberto Pasqualetto

#include "table.h"

#include <stdexcept>
#include <utility>
#include <iostream>
#include <opencv2/core.hpp>

cv::Vec<cv::Point, 4> Table::getBoundaries() const {
	if(boundaries_ == cv::Vec<cv::Point, 4>{cv::Point{0, 0}, cv::Point{0, 0}, cv::Point{0, 0}, cv::Point{0, 0}})
		throw std::runtime_error("bbox is uninitialized");

	return boundaries_;
}

cv::Vec2b Table::getColor() const {
	if (colorRange_ == cv::Vec2b(0, 0))   // black(0,0,0) = uninitialized
		throw std::runtime_error("color is uninitialized");

	return colorRange_;
}

bool Table::getTransform(cv::Mat &transformMatrix) const {
	if (!cv::norm(transform_, cv::Mat::eye(3, 3, CV_64F), cv::NORM_L1))
		return false;
//		throw std::runtime_error("transform is uninitialized");

	transformMatrix = transform_;

	return true;
}

/*
 * Returns a cv::Ptr (shared pointer) to the vector of balls.
 */
cv::Ptr<std::vector<Ball>> Table::ballsPtr() { // TODO is it ok to let the pointer be managed outside?
	if (balls_->empty())
		std::cout<<"There are no balls! (pointer returned anyway)"<<std::endl;

	return balls_;
}


void Table::setBoundaries(const cv::Vec<cv::Point, 4> &boundaries) {
	boundaries_ = boundaries;
}

void Table::setColor(cv::Vec2b color) { // NOLINT(*-unnecessary-value-param)
	colorRange_ = color;
}

void Table::setTransform(const cv::Mat &transform) {
	transform_ = std::move(transform);
}


void Table::addBall(Ball ball) {
	balls_->push_back(ball);
}

void Table::removeBall(int index) {
	balls_->erase(balls_->begin() + index);
}

void Table::addBalls(const std::vector<Ball> &balls) {
	for (auto &ball : balls){
		addBall(ball);
	}
}

void Table::clearBalls() {
	balls_->clear();
}


cv::Rect Table::getBbox() const {   // FIXME implement this based on the boundaries order
	throw std::runtime_error("Not implemented yet");
	return cv::Rect();
}
