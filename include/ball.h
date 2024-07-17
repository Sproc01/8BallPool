// Author: Michela Schibuola

#ifndef BALL_H
#define BALL_H

#include <opencv2/core/types.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/mat.hpp>
#include "category.h"

class Ball {
	cv::Rect bbox_;
	Category category_;
	cv::Rect bbox_prec_;

	public:
		Ball(cv::Rect bbox, Category category, cv::Rect bbox_prec) : bbox_(bbox), category_(category),
		                                                             bbox_prec_(bbox_prec) {
		}
		//TODO: bbox_prec_ empty? add function to check -1, -1?
		Ball(cv::Rect bbox, Category category) : bbox_(bbox), category_(category), bbox_prec_(cv::Rect(-1, -1, 0, 0)) {
		}

		cv::Rect getBbox() const;
		Category getCategory() const;
		cv::Rect getBbox_prec() const;
		cv::Point2f getBBoxCenter() const;
		cv::Point2f getBboxCenter_prec() const;

		void setBbox(cv::Rect bbox);
		void setCategory(Category category);
		void setBbox_prec(cv::Rect bbox_prec);
};

#endif // BALL_H
