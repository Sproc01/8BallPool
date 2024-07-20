// Author: Michela Schibuola

#ifndef BALL_H
#define BALL_H

#include <opencv2/core/types.hpp>
#include "category.h"

class Ball {
	cv::Rect bbox_;
	Category category_;
	cv::Rect bbox_prec_;
	bool visible_;

	public:
		Ball(cv::Rect bbox, Category category, cv::Rect bbox_prec, bool visible = true) : bbox_(bbox), category_(category),
		                                                             bbox_prec_(bbox_prec), visible_(visible) {
		}
		Ball(cv::Rect bbox, Category category, bool visible = true) : bbox_(bbox), category_(category),
								bbox_prec_(cv::Rect(-1, -1, -1, -1)), visible_(visible) {
		}

		cv::Rect getBbox() const;
		Category getCategory() const;
		cv::Rect getBbox_prec() const;
		cv::Point2f getBBoxCenter() const;
		cv::Point2f getBboxCenter_prec() const;
		bool getVisibility() const;

		void setBbox(cv::Rect bbox);
		void setCategory(Category category);
		void setBbox_prec(cv::Rect bbox_prec);
		void setVisibility(bool visible);
};

#endif // BALL_H
