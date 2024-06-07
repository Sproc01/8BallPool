// Author: Alberto Pasqualetto

#include "table.h"

#include <stdexcept>
#include <utility>
#include <iostream>

cv::Vec<cv::Point, 4> Table::getBoundaries() const {
	if(boundaries_ == cv::Vec<cv::Point, 4>{cv::Point{0, 0}, cv::Point{0, 0}, cv::Point{0, 0}, cv::Point{0, 0}})
		throw std::runtime_error("bbox is uninitialized");

	return boundaries_;
}

cv::Vec3b Table::getColor() const {
	if (color_ == cv::Vec3b(0, 0, 0))   // black(0,0,0) = uninitialized
		throw std::runtime_error("color is uninitialized");

	return color_;
}

cv::Mat Table::getTransform() const {
	if (transform_.empty())
		throw std::runtime_error("transform is uninitialized");

	return transform_;
}

/*
 * Returns a cv::Ptr (shared pointer) to the vector of balls.
 */
cv::Ptr<std::vector<Ball>> Table::getBallsPtr() { // TODO is it ok to let the pointer be managed outside?
	if (balls_->empty())
		std::cout<<"There are no balls! (pointer returned anyway)"<<std::endl;

	return balls_;
}


void Table::setBoundaries(const cv::Vec<cv::Point, 4> &boundaries) {
	boundaries_ = boundaries;
}

void Table::setColor(cv::Vec3b color) { // NOLINT(*-unnecessary-value-param)
	color_ = color;
}

void Table::setTransform(cv::Mat transform) {
	transform_ = std::move(transform);
}


void Table::addBall(Ball ball) {
	balls_->push_back(ball);
}

void Table::removeBall(int index) {
	balls_->erase(balls_->begin() + index);
}

void Table::clearBalls() {
	balls_->clear();
}


cv::Rect Table::getBbox() const {   // FIXME implement this based on the boundaries order
	throw std::runtime_error("Not implemented yet");
	return cv::Rect();
}
