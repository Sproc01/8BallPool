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
	cv::Vec<cv::Point, 4> boundaries_;  // TODO define order of points
	cv::Vec3b color_;
	cv::Mat transform_;
	cv::Ptr<std::vector<Ball>> balls_;

public:
	Table(cv::Vec<cv::Point, 4> boundaries, cv::Vec3b color, cv::Mat transform, std::vector<Ball> &balls): boundaries_(boundaries), color_(color), transform_(transform), balls_(new std::vector<Ball>(balls)) {}
	Table(cv::Vec<cv::Point, 4> boundaries, cv::Vec3b color, cv::Mat transform): boundaries_(boundaries), color_(color), transform_(transform), balls_(cv::Ptr<std::vector<Ball>>()) {}
	Table(cv::Vec<cv::Point, 4> boundaries, cv::Vec3b color): boundaries_(boundaries), color_(color), transform_(cv::Mat::eye(3, 3, CV_64F)), balls_(cv::Ptr<std::vector<Ball>>()) {}
	Table(cv::Vec<cv::Point, 4> boundaries): boundaries_(boundaries), color_(cv::Vec3b(0, 0, 0)), transform_(cv::Mat::eye(3, 3, CV_64F)), balls_(cv::Ptr<std::vector<Ball>>()) {}
	Table(): boundaries_(cv::Vec<cv::Point, 4>{cv::Point{0, 0}, cv::Point{0, 0}, cv::Point{0, 0}, cv::Point{0, 0}}), color_(cv::Vec3b(0, 0, 0)), transform_(cv::Mat::eye(3, 3, CV_64F)), balls_(cv::Ptr<std::vector<Ball>>()) {}

	cv::Vec<cv::Point, 4> getBoundaries() const;
	cv::Vec3b getColor() const;
	cv::Mat getTransform() const;
	cv::Ptr<std::vector<Ball>> getBallsPtr();

	void setBoundaries(const cv::Vec<cv::Point, 4> &boundaries);
	void setColor(cv::Vec3b color);
	void setTransform(cv::Mat transform);

	void addBall(Ball ball);
	void removeBall(int index);
	void clearBalls();

	cv::Rect getBbox() const;
};


#endif //TABLE_H
