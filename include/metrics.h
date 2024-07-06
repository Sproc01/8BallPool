// Author: Alberto Pasqualetto

#ifndef METRICS_H
#define METRICS_H

#include <opencv2/core/types.hpp>
#include <string>
#include <filesystem>
#include <utility>
#include "table.h"
#include "category.h"

enum FrameN {
  FIRST = 0,
  MIDDLE,
  LAST
};

const float MAP_IOU_THRESHOLD = 0.5;

void compareMetrics(Table &table, cv::Mat &segmentedImage, const std::string &folderPath, FrameN frameN);

std::vector<std::pair<cv::Rect, Category>> readGroundTruthBboxFile(const std::string &filename);

double mAPDetection(const cv::Ptr<std::vector<Ball>> &detectedBalls, const std::string &groundTruthBboxPath, float iouThreshold = MAP_IOU_THRESHOLD);
double mIoUSegmentation(const cv::Mat &segmentedImage, const std::string& groundTruthMaskPath);

double IoU(const cv::Rect &rect1, const cv::Rect &rect2);
double IoU(const cv::Mat &mask1, const cv::Mat &mask2);

#endif //METRICS_H
