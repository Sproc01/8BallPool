// Author: Alberto Pasqualetto

#include "table.h"

#include <stdexcept>
#include <utility>


cv::Rect Table::getBbox() const {
	if(bbox_.empty())
		throw std::runtime_error("bbox is uninitialized");

	return bbox_;
}

cv::Vec3b Table::getColor() const {
	if (color_ == cv::Vec3b(0, 0, 0))   // black = uninitialized
		throw std::runtime_error("color is uninitialized");

	return color_;
}

cv::Mat Table::getTransform() const {
	if (transform_.empty())
		throw std::runtime_error("transform is uninitialized");

	return transform_;
}

std::vector<Ball> Table::getBalls() const {
	if (balls_.empty())
		throw std::runtime_error("balls is uninitialized");

	return balls_;
}


void Table::setBbox(cv::Rect bbox) {
	bbox_ = bbox;
}

void Table::setColor(cv::Vec3b color) {
	color_ = color;
}

void Table::setTransform(cv::Mat transform) {
	transform_ = std::move(transform);
}

void Table::setBalls(std::vector<Ball> balls) {
	balls_ = balls; // TODO check if it's a copy or a reference (copy constructor? use std::move?)
}


void Table::addBall(Ball ball) {
	balls_.push_back(ball);
}

void Table::removeBall(int index) {
	balls_.erase(balls_.begin() + index);
}

void Table::clearBalls() {
	balls_.clear();
}
