// Author: Alberto Pasqualetto

#ifndef METRICS_H
#define METRICS_H

#include <opencv2/core/types.hpp>
#include <string>
#include <filesystem>
#include <utility>
#include "table.h"

enum FrameN {
  FIRST = 0,
  MIDDLE,
  LAST
};

const float MAP_IOU_THRESHOLD = 0.5;

void compareMetrics(Table &table, cv::Mat &segmentedImage, const std::string &folderPath, FrameN frameN);

std::vector<std::pair<cv::Rect, short>> readGroundTruthFile(std::string filename);

float mAP();
float mIoUSegmentation(const cv::Mat &segmentedImage, const std::string& groundTruthMaskPath);

float AP();
//float IoU(cv::Rect &rect1, cv::Rect &rect2);
float IoU(const cv::Mat &mask1, const cv::Mat &mask2);

#endif //METRICS_H
