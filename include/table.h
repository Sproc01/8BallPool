// Author: Alberto Pasqualetto

#ifndef TABLE_H
#define TABLE_H

#include <opencv2/core/types.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/mat.hpp>
#include <vector>
#include "ball.h"

/**
 * Implementation the table.
 *
 * The class contains the information about the table: the boundaries, the color range, the transformation matrix to the minimap and the vector of balls.
 */
class Table {
	cv::Vec<cv::Point2f, 4> boundaries_;
	cv::Vec2b colorRange_;
	cv::Mat transform_;
	cv::Ptr<std::vector<Ball>> balls_;

public:
	/**
	* @brief Constructor of table when the boundaries, the color of the table, the transformation matrix and the vector of balls are known.
	* @param boundaries boundaries of the table.
	* @param colorRange color range of the table.
	* @param transform transformation matrix.
	* @param balls vector of balls.
	*/
	Table(cv::Vec<cv::Point2f, 4> boundaries, cv::Vec2b colorRange, cv::Mat transform, cv::Ptr<std::vector<Ball>> balls): boundaries_(boundaries), colorRange_(colorRange), transform_(transform), balls_(balls) { if (balls_ == nullptr) balls_ = cv::makePtr<std::vector<Ball>>(); }
	/**
	* @brief Constructor of table when the boundaries, the color of the table and the transformation matrix are known.
	* @param boundaries boundaries of the table.
	* @param colorRange color range of the table.
	* @param transform transformation matrix.
	*/
	Table(cv::Vec<cv::Point2f, 4> boundaries, cv::Vec2b colorRange, cv::Mat transform): Table(boundaries, colorRange, transform, nullptr) {}
	/**
	* @brief Constructor of table when the boundaries and the color of the table are known.
	* @param boundaries boundaries of the table.
	* @param colorRange color range of the table.
	*/
	Table(cv::Vec<cv::Point2f, 4> boundaries, cv::Vec2b colorRange): Table(boundaries, colorRange, cv::Mat::eye(3, 3, CV_64F)) {}
	/**
	* @brief Constructor of table when only the boundaries are known.
	* @param boundaries boundaries of the table.
	*/
	Table(cv::Vec<cv::Point2f, 4> boundaries): Table(boundaries, cv::Vec2b(0, 0)) {}
	/**
	* @brief Constructor of an empty table.
	*/
	Table(): Table(cv::Vec<cv::Point2f, 4>{cv::Point{0, 0}, cv::Point{0, 0}, cv::Point{0, 0}, cv::Point{0, 0}}) {}


	/**
	 * @brief Return the boundaries of the table.
	 * @return the boudaries of the table.
	 * @throw runtime_error if the boundaries are uninitialized.
	 */
	cv::Vec<cv::Point2f, 4> getBoundaries() const;

	/**
	 * @brief Return the color of the table.
	 * @return the color of the table.
	 * @throw runtime_error if the color is uninitialized.
	 */
	cv::Vec2b getColor() const;

	/**
	 * @brief Return the transformation matrix for the table.
	 * @return the transformation matrix.
	 * @throw runtime_error if the transform is uninitialized.
	 */
	cv::Mat getTransform() const;

	/**
	 * @brief Return a shared pointer of the vector of balls.
	 * @return a cv::Ptr (shared pointer) to the vector of balls.
	 */
	cv::Ptr<std::vector<Ball>> ballsPtr();


	/**
	 * @brief Set the boundaries of the table.
	 * @param boundaries vector of the four boundaries of the table.
	 */
	void setBoundaries(const cv::Vec<cv::Point2f, 4> &boundaries);

	/**
	 * @brief Set the color of the table.
	 * @param color color of the table.
	 */
	void setColor(cv::Vec2b color);

	/**
	 * @brief Set the transform matrix of the table.
	 * @param transform transform matrix of the table.
	 */
	void setTransform(const cv::Mat &transform);


	/**
	 * @brief Add a ball to the set of balls.
	 * @param ball object ball.
	 */
	void addBall(Ball ball);

	/**
	 * @brief Remove a ball from the set of balls using its index.
	 * @param index position of the ball to remove.
	 */
	void removeBall(int index);

	/**
	 * @brief Add a vector of balls to the set of balls.
	 * @param balls vector of balls.
	 */
	void addBalls(const std::vector<Ball> &balls);

	/**
	 * @brief Remove all balls.
	 */
	void clearBalls();
};


#endif //TABLE_H
