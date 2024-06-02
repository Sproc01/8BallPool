// Author: Michela Schibuola

#include "ball.h"

#include <stdexcept>
#include <utility>

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
