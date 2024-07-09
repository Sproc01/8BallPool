// Author: Michela Schibuola

#include "ball.h"

#include <stdexcept>
#include <utility>
#include <opencv2/opencv.hpp>

using namespace cv;

Rect Ball::getBbox() const {
	if (bbox_.empty())
		throw std::runtime_error("bbox is uninitialized");

	return bbox_;
}

Category Ball::getCategory() const {
	if (category_ == 0)
		throw std::runtime_error("category is uninitialized");

	return category_;
}

Rect Ball::getBbox_prec() const {
	if (bbox_prec_.empty())
		throw std::runtime_error("bbox_prec is uninitialized");

	return bbox_prec_;
}

void Ball::setBbox(Rect bbox) {
	bbox_ = bbox;
}

void Ball::setCategory(Category category) {
	category_ = category;
}

void Ball::setBbox_prec(Rect bbox_prec) {
	bbox_prec_ = bbox_prec;
}

Point2f Ball::getBBoxCenter() const {
	return Point(bbox_.x + bbox_.width / 2, bbox_.y + bbox_.height / 2);
}

Point2f Ball::getBboxCenter_prec() const {
	return Point(bbox_prec_.x + bbox_prec_.width / 2, bbox_prec_.y + bbox_prec_.height / 2);
}
