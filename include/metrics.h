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

/**
 * @brief compute the Intersection over Union between the segmented image and the ground truth mask.
 * @param segmentedImage segmented image.
 * @param folderPath path to the folder containing the ground truth masks.
 * @param frameN Set to FIRST for the first frame, LAST for the last frame.
 * @return std::vector<double> vector of IoU values for each category.
 * @throw invalid_argument if segmentedImage is empty.
 */
std::vector<double> compareMetricsIoU(cv::Mat &segmentedImage, const std::string &folderPath, FrameN frameN);

/**
 * @brief compute the Average Precision (mAP) for ball detection.
 * @param table table containing the detected balls.
 * @param folderPath path to the folder containing the ground truth bounding boxes.
 * @param frameN Set to FIRST for the first frame, LAST for the last frame.
 * @return std::vector<double> vector of AP values for each category.
 */
std::vector<double> compareMetricsAP(Table &table, const std::string &folderPath, FrameN frameN);

/**
 * @brief read the ground truth mask from a file.
 * @param filename path to the file.
 * @return std::vector<std::pair<cv::Rect, Category>> vector of pairs of rectangles and categories that represent the ground truth bounding boxes.
 * @throw invalid_argument if the file does not exist.
 */
std::vector<std::pair<cv::Rect, Category>> readGroundTruthBboxFile(const std::string &filename);

/**
 * @brief compute the Average Precision (AP) for ball detection.
 * @param detectedBalls vector of detected balls.
 * @param groundTruthBboxPath path to the file containing the ground truth bounding boxes.
 * @param iouThreshold Intersection over Union threshold.
 * @return std::vector<double> vector of AP values for each category.
 * @throw invalid_argument if the vector of detected balls is empty.
 */
std::vector<double> APDetection(const cv::Ptr<std::vector<Ball>> &detectedBalls, const std::string &groundTruthBboxPath, float iouThreshold = MAP_IOU_THRESHOLD);

/**
 * @brief compute the Intersection over Union between the segmented image and the ground truth mask.
 * @param segmentedImage segmented image.
 * @param groundTruthMaskPath path to the ground truth mask.
 * @return std::vector<double> vector of IoU values for each category.
 * @throw invalid_argument if the segmented image is empty.
 */
std::vector<double> IoUSegmentation(const cv::Mat &segmentedImage, const std::string& groundTruthMaskPath);

/**
 * @brief compute the Intersection over Union between two rectangles.
 * @param rect1 first rectangle.
 * @param rect2 second rectangle.
 * @return double IoU value.
 * @throw invalid_argument if one of the two rectangles is empty.
 */
double IoU(const cv::Rect &rect1, const cv::Rect &rect2);

/**
 * @brief compute the Intersection over Union between two masks.
 * @param mask1 first mask.
 * @param mask2 second mask.
 * @return double IoU value.
 * @throw invalid_argument if one of the two masks is empty.
 */
double IoU(const cv::Mat &mask1, const cv::Mat &mask2);

/**
 * @brief compute the Average Precision (AP) for ball detection of a specific category.
 * @param detectedBalls vector of detected balls.
 * @param groundTruthBboxes vector of pairs of rectangles and categories that represent the ground truth bounding boxes.
 * @param cat category.
 * @param iouThreshold Intersection over Union threshold.
 * @return double AP value.
 * @throw invalid_argument if the vector of detected balls is empty or if the vector of ground truth bounding boxes is empty.
 */
double APBallCategory(const cv::Ptr<std::vector<Ball>> &detectedBalls, const std::vector<std::pair<cv::Rect, Category>> &groundTruthBboxes, Category cat, float iouThreshold);

/**
 * @brief compute the Intersection over Union between the segmented image and the ground truth mask of a specific category.
 * @param segmentedImage segmented image.
 * @param groundTruthMask ground truth mask.
 * @param cat category.
 * @return double IoU value.
 * @throw invalid_argument if the segmented image is empty or if the ground truth mask is empty.
 */
double IoUCategory(const cv::Mat &segmentedImage, const cv::Mat &groundTruthMask, Category cat);

#endif //METRICS_H
