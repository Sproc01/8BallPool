// Author: Michela Schibuola

#include <opencv2/core/types.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/mat.hpp>
#include "category.h"

#ifndef BALL_H
#define BALL_H

class Ball
{
    cv::Rect bbox_;
    Category category_;
    cv::Rect bbox_prec_;

public:
    Ball(cv::Rect bbox, Category category, cv::Rect bbox_prec) : bbox_(bbox), category_(category), bbox_prec_(bbox_prec) {}
    Ball(cv::Rect bbox, Category category) : bbox_(bbox), category_(category), bbox_prec_(cv::Rect(0, 0, 0, 0)) {}

    cv::Rect getBbox() const;
    Category getCategory() const;
    cv::Rect getBbox_prec() const;
    cv::Point2f getBBoxCenter();

    void setBbox(cv::Rect bbox);
    void setCategory(Category category);
    void setBbox_prec(cv::Rect bbox_prec);
};

#endif // BALL_H
