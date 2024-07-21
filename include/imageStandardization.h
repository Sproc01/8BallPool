// Author: Alberto Pasqualetto

#ifndef IMAGESTANDARDIZATION_H
#define IMAGESTANDARDIZATION_H

#include <opencv2/core/mat.hpp>

#include "util.h"
#include "table.h"

void calculateStandardizationParameters(const cv::Mat &img, int targetWidth, int targetHeight, bool &toRotate, bool &toResize, short &leftBorderLength, short &rightBorderLength);

void doInscriptImage(cv::Mat &img, int targetWidth, int targetHeight, const bool &toRotate, const bool &toResize, const short &leftBorderLength, const short &rightBorderLength);
void undoInscriptImage(cv::Mat &img, int originalWidth, int originalHeight, const bool &toRotate, const bool &toResize, const short &leftBorderLength, const short &rightBorderLength);

void doInscriptTableObject(Table &table, int targetWidth, int targetHeight, int originalWidth, int originalHeight, const bool &toRotate, const bool &toResize, const short &leftBorderLength, const short &rightBorderLength, bool changeBboxPrec  = false);
void undoInscriptTableObject(Table &table, int targetWidth, int targetHeight, int originalWidth, int originalHeight, const bool &toRotate, const bool &toResize, const short &leftBorderLength, const short &rightBorderLength, bool changeBboxPrec  = false);

#endif //IMAGESTANDARDIZATION_H
