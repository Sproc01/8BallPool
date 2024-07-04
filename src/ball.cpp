// Author: Michela Schibuola

#include "ball.h"

#include <stdexcept>
#include <utility>
#include <opencv2/opencv.hpp>

using namespace cv;

Rect Ball::getBbox() const
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

Rect Ball::getBbox_prec() const
{
    if (bbox_prec_.empty())
        throw std::runtime_error("bbox_prec is uninitialized");

    return bbox_prec_;
}

void Ball::setBbox(Rect bbox)
{
    bbox_ = bbox;
}

void Ball::setCategory(Category category)
{
    category_ = category;
}

void Ball::setBbox_prec(Rect bbox_prec)
{
    bbox_prec_ = bbox_prec;
}

//TODO: use center in util
Point2f getBBoxCenterGeneral(Rect bbox)
{
    Point2f center;
    center = Point(bbox.x + bbox.width/2, bbox.y + bbox.height/2);
    return center;
}

Point2f Ball::getBBoxCenter() {
    return getBBoxCenterGeneral(bbox_);
}

Point2f Ball::getBboxCenter_prec()
{
    return getBBoxCenterGeneral(bbox_prec_);
}
