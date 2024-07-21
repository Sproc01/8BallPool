// Author: Michela Schibuola

#ifndef BALL_H
#define BALL_H

#include <opencv2/core/types.hpp>
#include "category.h"

/**
 * Implementation of a ball.
 *
 * The class contains the information about a ball: the position, the category, the visibility
 * and the position of the same ball in a previous frame.
 */
class Ball {
	cv::Rect bbox_;
	Category category_;
	cv::Rect bbox_prec_;
	bool visible_;

public:
	/**
	* @brief Constructor of ball when current and previous positions are known.
	* @param bbox bbox of the ball.
	* @param category category of the ball.
	* @param bbox_prec bbox_prec of the ball.
	* @param visible visibility of the ball.
	*/
	Ball(cv::Rect bbox, Category category, cv::Rect bbox_prec, bool visible = true) : bbox_(bbox), category_(category), bbox_prec_(bbox_prec), visible_(visible) {}
	/**
	* @brief Constructor of ball when just the current is known.
	* @param bbox bbox of the ball.
	* @param category category of the ball.
	* @param visible visibility of the ball.
	*/
	Ball(cv::Rect bbox, Category category, bool visible = true) : bbox_(bbox), category_(category),bbox_prec_(cv::Rect(-1, -1, -1, -1)),visible_(visible) {}

	/**
	* @brief Return the rectangle containing the ball.
	* @return the bbox of the ball.
	* @throw runtime_error if the bbox is empty.
	*/
	cv::Rect getBbox() const;

	/**
	 * @brief Return the rectangle containing the ball in the previous frame.
	 * @return the bbox of the ball in the previous frame.
	 * @throw runtime_error if the bbox is empty.
	 */
	cv::Rect getBbox_prec() const;

	/**
	* @brief Return the category of the ball.
	* @return the category of the ball.
	* @throw runtime_error if the category does not correspond to a ball.
	*/
	Category getCategory() const;

	/**
	 * @brief Return the center of the rectangle containing the ball.
	 * @return the center of the bbox of the ball.
	 */
	cv::Point2f getBBoxCenter() const;

	/**
	 * @brief Return the center of the rectangle containing the ball in the previous frame.
	 * @return the center of the bbox of the ball in the previous frame.
	 */
	cv::Point2f getBboxCenter_prec() const;

	/**
	 * @brief Return if the ball is inside the table area.
	 * @return true if the ball is inside the table area, false otherwise.
	 */
	bool getVisibility() const;

	/**
	 * @brief Set a value to the rectangle of the ball.
	 * @param bbox the new bbox position.
	 */
	void setBbox(const cv::Rect &bbox);

	/**
	 * @brief Set a value to the rectangle of the ball in the previous frame.
	 * @param bbox_prec the new bbox position of the previous frame.
	 */
	void setBbox_prec(const cv::Rect &bbox_prec);

	/**
	 * @brief Set a value to the category of the ball.
	 * @param category the new category.
	 * @throw runtime_error if the category does not correspond to a ball.
	 */
	void setCategory(Category category);

	/**
	 * @brief Set a value to the visibility of the ball.
	 * @param visible the new visibility value.
	 */
	void setVisibility(bool visible);
};

#endif // BALL_H
