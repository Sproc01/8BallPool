// Author: Alberto Pasqualetto

#ifndef TABLE_H
#define TABLE_H

#include <opencv2/core/types.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/mat.hpp>
#include <vector>
#include "ball.h"

constexpr unsigned short MAX_BALLS = 16;


class Table {
	cv::Vec<cv::Point2f, 4> boundaries_;  // TODO define order of points
	cv::Vec2b colorRange_;
	cv::Mat transform_;
	cv::Ptr<std::vector<Ball>> balls_;

public:
	Table(cv::Vec<cv::Point2f, 4> boundaries, cv::Vec2b colorRange, cv::Mat transform, cv::Ptr<std::vector<Ball>> balls): boundaries_(boundaries), colorRange_(colorRange), transform_(transform), balls_(balls) { if (balls_ == nullptr) balls_ = cv::makePtr<std::vector<Ball>>(); }
	Table(cv::Vec<cv::Point2f, 4> boundaries, cv::Vec2b colorRange, cv::Mat transform): Table(boundaries, colorRange, transform, nullptr) {}
	Table(cv::Vec<cv::Point2f, 4> boundaries, cv::Vec2b colorRange): Table(boundaries, colorRange, cv::Mat::eye(3, 3, CV_64F)) {}
	Table(cv::Vec<cv::Point2f, 4> boundaries): Table(boundaries, cv::Vec2b(0, 0)) {}
	Table(): Table(cv::Vec<cv::Point2f, 4>{cv::Point{0, 0}, cv::Point{0, 0}, cv::Point{0, 0}, cv::Point{0, 0}}) {}

	cv::Vec<cv::Point2f, 4> getBoundaries() const;
	cv::Vec2b getColor() const;
	bool getTransform(cv::Mat &transformMatrix) const;
	cv::Ptr<std::vector<Ball>> ballsPtr();

	void setBoundaries(const cv::Vec<cv::Point2f, 4> &boundaries);
	void setColor(cv::Vec2b color);
	void setTransform(const cv::Mat &transform);

	void addBall(Ball ball);
	void removeBall(int index);
	void addBalls(const std::vector<Ball> &balls);
	void clearBalls();

	cv::Rect getBbox() const;
};


#endif //TABLE_H
