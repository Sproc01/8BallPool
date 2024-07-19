// Author: Alberto Pasqualetto

#ifndef METRICS_H
#define METRICS_H

#include <opencv2/core/types.hpp>
#include <string>
#include <filesystem>
#include <utility>
#include "table.h"
#include "category.h"
#include "ball.h"

enum FrameN {
  FIRST = 0,
  MIDDLE,
  LAST
};

const float MAP_IOU_THRESHOLD = 0.5;

std::vector<double> compareMetricsIoU(cv::Mat &segmentedImage, const std::string &folderPath, FrameN frameN);
std::vector<double> compareMetricsAP(Table &table, const std::string &folderPath, FrameN frameN);

std::vector<std::pair<cv::Rect, Category>> readGroundTruthBboxFile(const std::string &filename);

std::vector<double> mAPDetection(const cv::Ptr<std::vector<Ball>> &detectedBalls, const std::string &groundTruthBboxPath, float iouThreshold = MAP_IOU_THRESHOLD);
std::vector<double> mIoUSegmentation(const cv::Mat &segmentedImage, const std::string& groundTruthMaskPath);

double IoU(const cv::Rect &rect1, const cv::Rect &rect2);
double IoU(const cv::Mat &mask1, const cv::Mat &mask2);

double APBallCategory(const cv::Ptr<std::vector<Ball>> &detectedBalls, const std::vector<std::pair<cv::Rect, Category>> &groundTruthBboxes, Category cat, float iouThreshold);
double mIoUCategory(const cv::Mat &segmentedImage, const cv::Mat &groundTruthMask, Category cat);

#endif //METRICS_H
