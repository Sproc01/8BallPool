// Author: Michela Schibuola

#include "ball.h"

#include <stdexcept>
#include <opencv2/opencv.hpp>

using namespace cv;

/**
 * @brief Return the rectangle containing the ball.
 * @return the bbox of the ball.
 * @throw runtime_error if the bbox is empty.
 */
Rect Ball::getBbox() const {
	if (bbox_.empty())
		throw std::runtime_error("bbox is uninitialized");

	return bbox_;
}

/**
 * @brief Return the rectangle containing the ball in the previous frame.
 * @return the bbox of the ball in the previous frame.
 * @throw runtime_error if the bbox is empty.
 */
Rect Ball::getBbox_prec() const {
    if (bbox_prec_.empty())
        throw std::runtime_error("bbox_prec is uninitialized");

    return bbox_prec_;
}

/**
 * @brief Return the category of the ball.
 * @return the category of the ball.
 * @throw runtime_error if the category does not correspond to a ball.
 */
Category Ball::getCategory() const {
	if (category_ == 0 || category_ == 5)
		throw std::runtime_error("category does not correspond to a ball");

	return category_;
}

/**
 * @brief Return the center of the rectangle containing the ball.
 * @return the center of the bbox of the ball.
 */
Point2f Ball::getBBoxCenter() const {
	return Point(bbox_.x + bbox_.width / 2, bbox_.y + bbox_.height / 2);
}

/**
 * @brief Return the center of the rectangle containing the ball in the previous frame.
 * @return the center of the bbox of the ball in the previous frame.
 */
Point2f Ball::getBboxCenter_prec() const {
	return Point(bbox_prec_.x + bbox_prec_.width / 2, bbox_prec_.y + bbox_prec_.height / 2);
}

/**
 * @brief Return if the ball is inside the table area.
 * @return true if the ball is inside the table area, false otherwise.
 */
bool Ball::getVisibility() const {
	return visible_;
}

/**
 * @brief Set a value to the rectangle of the ball.
 * @param bbox the new bbox position.
 */
void Ball::setBbox(Rect bbox) {
	bbox_ = bbox;
}

/**
 * @brief Set a value to the rectangle of the ball in the previous frame.
 * @param bbox_prec the new bbox position of the previous frame.
 */
void Ball::setBbox_prec(Rect bbox_prec) {
	bbox_prec_ = bbox_prec;
}

/**
 * @brief Set a value to the category of the ball.
 * @param category the new category.
 * @throw runtime_error if the category does not correspond to a ball.
 */
void Ball::setCategory(Category category) {
	if (category_ == 0 || category_ == 5)
		throw std::runtime_error("category does not correspond to a ball");

	category_ = category;
}

/**
 * @brief Set a value to the visibility of the ball.
 * @param visible the new visibility value.
 */
void Ball::setVisibility(bool visible) {
	visible_ = visible;
}
