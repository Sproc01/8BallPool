// Author: Alberto Pasqualetto

#include "table.h"

#include <stdexcept>
#include <utility>
#include <iostream>
#include <opencv2/core.hpp>

using namespace cv;

Vec<Point2f, 4> Table::getBoundaries() const {
	if(boundaries_ == Vec<Point2f, 4>{Point2f{0, 0}, Point2f{0, 0}, Point2f{0, 0}, Point2f{0, 0}})
		throw std::runtime_error("bbox is uninitialized");

	return boundaries_;
}

Vec2b Table::getColor() const {
	if (colorRange_ == Vec2b(0, 0))   // black for both boundaries = uninitialized
		throw std::runtime_error("color is uninitialized");

	return colorRange_;
}

Mat Table::getTransform() const {
	if (!norm(transform_, Mat::eye(3, 3, CV_64F), NORM_L1))
		// return false;
		throw std::runtime_error("transform is uninitialized");

	return transform_;
}

/*
 * Returns a cv::Ptr (shared pointer) to the vector of balls.
 */
Ptr<std::vector<Ball>> Table::ballsPtr() {
	return balls_;
}


void Table::setBoundaries(const Vec<Point2f, 4> &boundaries) {
	boundaries_ = boundaries;
}

void Table::setColor(Vec2b color) { // NOLINT(*-unnecessary-value-param)
	colorRange_ = color;
}

void Table::setTransform(const Mat &transform) {
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


Rect Table::getBbox() const {   // FIXME implement this based on the boundaries order
	throw std::runtime_error("Not implemented yet");
	return Rect();
}
