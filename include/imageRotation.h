// Author: Alberto Pasqualetto

#ifndef IMAGEROTATION_H
#define IMAGEROTATION_H

#include <opencv2/core/mat.hpp>

#include "util.h"
#include "table.h"

void calculateInscriptionParameters(const cv::Mat &img, int targetWidth, int targetHeight, bool &toRotate, bool &toResize, short &leftBorderLength, short &rightBorderLength);

void doInscript(cv::Mat &img, int targetWidth, int targetHeight, const bool &toRotate, const bool &toResize, const short &leftBorderLength, const short &rightBorderLength);
void undoInscript(cv::Mat &img, int originalWidth, int originalHeight, const bool &toRotate, const bool &toResize, const short &leftBorderLength, const short &rightBorderLength);

void rotateTable(Table &table, const cv::Mat &destImg, bool changeBboxPrec = false);
void unrotateTable(Table &table, const cv::Mat &destImg, bool changeBboxPrec = false);

#endif //IMAGEROTATION_H
