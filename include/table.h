// Author: Alberto Pasqualetto

#ifndef TABLE_H
#define TABLE_H

#include <opencv2/core/types.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/mat.hpp>
#include <vector>
#include "ball.h"

/**
 * Implementation the billiard table.
 * The class contains the information about the table: the boundaries, the color range, the transformation matrix to the minimap and the vector of balls.
 */
class Table {
	cv::Vec<cv::Point2f, 4> boundaries_;
	cv::Vec2b colorRange_;	// color range of the table expressed as the 2 V boundaries of the range in HSV format
	cv::Mat transform_;
	cv::Ptr<std::vector<Ball>> balls_;

public:
	/**
	* @brief Constructor of Table when the boundaries, the color of the table, the transformation matrix and the vector of balls are known.
	* @param boundaries boundaries of the table.
	* @param colorRange color range of the table. Expressed as the 2 V boundaries of the range in HSV format.
	* @param transform transformation matrix.
	* @param balls vector of balls.
	*/
	Table(cv::Vec<cv::Point2f, 4> boundaries, cv::Vec2b colorRange, cv::Mat transform, cv::Ptr<std::vector<Ball>> balls): boundaries_(boundaries), colorRange_(colorRange), transform_(transform), balls_(balls) { if (balls_ == nullptr) balls_ = cv::makePtr<std::vector<Ball>>(); }
	/**
	* @brief Constructor of Table when the boundaries, the color of the table and the transformation matrix are known.
	* @param boundaries boundaries of the table.
	* @param colorRange color range of the table. Expressed as the 2 V boundaries of the range in HSV format.
	* @param transform transformation matrix.
	*/
	Table(cv::Vec<cv::Point2f, 4> boundaries, cv::Vec2b colorRange, cv::Mat transform): Table(boundaries, colorRange, transform, nullptr) {}
	/**
	* @brief Constructor of Table when the boundaries and the color of the table are known.
	* @param boundaries boundaries of the table.
	* @param colorRange color range of the table. Expressed as the 2 V boundaries of the range in HSV format.
	*/
	Table(cv::Vec<cv::Point2f, 4> boundaries, cv::Vec2b colorRange): Table(boundaries, colorRange, cv::Mat::eye(3, 3, CV_64F)) {}
	/**
	* @brief Constructor of Table when only the boundaries are known.
	* @param boundaries boundaries of the table.
	*/
	Table(cv::Vec<cv::Point2f, 4> boundaries): Table(boundaries, cv::Vec2b(0, 0)) {}
	/**
	* @brief Constructor of an empty Table.
	*/
	Table(): Table(cv::Vec<cv::Point2f, 4>{cv::Point{0, 0}, cv::Point{0, 0}, cv::Point{0, 0}, cv::Point{0, 0}}) {}


	/**
	 * @brief Return the boundaries of the Table.
	 * @return the boundaries of the Table.
	 * @throw runtime_error if the boundaries are uninitialized.
	 */
	cv::Vec<cv::Point2f, 4> getBoundaries() const;

	/**
	 * @brief Return the color range of the Table.
	 * @return the color range of the Table expressed as the 2 V boundaries of the range in HSV format.
	 * @throw runtime_error if the color is uninitialized.
	 */
	cv::Vec2b getColorRange() const;

	/**
	 * @brief Return the transformation matrix for the Table.
	 * @return the transformation matrix.
	 * @throw runtime_error if the transform is uninitialized.
	 */
	cv::Mat getTransform() const;

	/**
	 * @brief Return a shared pointer of the vector of Ball relative to this Table.
	 * @return a cv::Ptr (shared pointer) to the vector of Ball.
	 */
	cv::Ptr<std::vector<Ball>> ballsPtr();


	/**
	 * @brief Set the boundaries of the Table.
	 * @param boundaries vector of the four boundaries of the Table.
	 */
	void setBoundaries(const cv::Vec<cv::Point2f, 4> &boundaries);

	/**
	 * @brief Set the color range of the Table.
	 * @param colorRange color range of the Table expressed as the 2 V boundaries of the range in HSV format.
	 */
	void setColorRange(cv::Vec2b colorRange);

	/**
	 * @brief Set the transform matrix of the Table.
	 * @param transform transform matrix of the Table.
	 */
	void setTransform(const cv::Mat &transform);


	/**
	 * @brief Add a Ball to the set of balls.
	 * @param ball Ball object.
	 */
	void addBall(Ball ball);

	/**
	 * @brief Remove a Ball from the set of balls using its index.
	 * @param index position of the Ball to remove.
	 */
	void removeBall(int index);

	/**
	 * @brief Add a vector of Ball to the set of balls.
	 * @param balls vector of Ball.
	 */
	void addBalls(const std::vector<Ball> &balls);

	/**
	 * @brief Remove all balls in the Table.
	 */
	void clearBalls();
};


#endif //TABLE_H
