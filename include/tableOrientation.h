//Author: Michela Schibuola

#ifndef TABLEORIENTATION_H
#define TABLEORIENTATION_H

#include <opencv2/opencv.hpp>

/**
 * @brief Check if the table image (transformed and cropped) is horizontal
 * @param table_img image of the table transformed and cropped to the minimap dimension
 * @param corners corners of the table in the table_img
 * @return true if the image is horizontal, false otherwise
 * @throw invalid_argument if the image in input is empty
 */
bool checkHorizontalTable(const cv::Mat &table_img,cv::Vec<cv::Point2f, 4> corners);

#endif //TABLEORIENTATION_H
