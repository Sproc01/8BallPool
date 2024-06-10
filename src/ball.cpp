// Author: Michela Schibuola

#include "ball.h"

#include <stdexcept>
#include <utility>
#include <opencv2/opencv.hpp>

using namespace cv;

cv::Rect Ball::getBbox() const 
{
    if (bbox_.empty())
        throw std::runtime_error("bbox is uninitialized");

    return bbox_;
}

Category Ball::getCategory() const
{
    if (category_ == 0)
        throw std::runtime_error("category is uninitialized");

    return category_;
}

cv::Rect Ball::getBbox_prec() const
{
    if (bbox_prec_.empty())
        throw std::runtime_error("bbox_prec is uninitialized");

    return bbox_prec_;
}

void Ball::setBbox(cv::Rect bbox)
{
    bbox_ = bbox;
}

void Ball::setCategory(Category category)
{
    category_ = category;
}

void Ball::setBbox_prec(cv::Rect bbox_prec)
{
    bbox_prec_ = bbox_prec;
}

Point2f Ball::getBBoxCenter() {
    Point2f center;
    float dx_half = abs(bbox_.x - bbox_.br().x)/2;
    float dy_half = abs(bbox_.y - bbox_.br().y)/2;
    if(bbox_.x > bbox_.br().x)
        dx_half *= -1;
    if(bbox_.y > bbox_.br().y)
        dy_half *= -1;
    center = Point(bbox_.x + dx_half, bbox_.br().y + dy_half);
    return center;
}
