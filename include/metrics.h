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
 * @brief Compute the Intersection over Union between the segmented image and the ground truth mask.
 * @param segmentedImage segmented image.
 * @param folderPath path to the folder containing the ground truth masks.
 * @param frameN Set to FIRST for the first frame, LAST for the last frame.
 * @return std::vector<double> vector of IoU values for each category.
 * @throw invalid_argument if segmentedImage is empty.
 */
std::vector<double> compareMetricsIoU(const cv::Mat &segmentedImage, const std::string &folderPath, const FrameN &frameN);

/**
 * @brief Compute the Average Precision (mAP) for ball detection.
 * @param table Table object containing the detected balls.
 * @param folderPath path to the folder containing the ground truth bounding boxes.
 * @param frameN Set to FIRST for the first frame, LAST for the last frame.
 * @return std::vector<double> vector of AP values for each category.
 */
std::vector<double> compareMetricsAP(Table &table, const std::string &folderPath, const FrameN &frameN);


/**
 * @brief Read the ground truth bounding boxes from a file with format "x y w h category".
 * @param filename path to the file.
 * @return std::vector<std::pair<cv::Rect, Category>> vector of pairs of rectangles and categories that represent the ground truth bounding boxes.
 * @throw invalid_argument if the file does not exist.
 */
std::vector<std::pair<cv::Rect, Category>> readGroundTruthBboxFile(const std::string &filename);


/**
 * @brief Compute the Average Precision (AP) for balls detection.
 * @param detectedBalls vector of detected balls.
 * @param groundTruthBboxPath path to the file containing the ground truth bounding boxes.
 * @param iouThreshold Intersection over Union threshold.
 * @return std::vector<double> vector of AP values for each category.
 * @throw invalid_argument if the vector of detected balls is empty.
 */
std::vector<double> APDetection(const cv::Ptr<std::vector<Ball>> &detectedBalls, const std::string &groundTruthBboxPath, float iouThreshold = MAP_IOU_THRESHOLD);

/**
 * @brief Compute the Intersection over Union between the segmented image and the ground truth mask.
 * @param segmentedImage segmented image.
 * @param groundTruthMaskPath path to the ground truth mask.
 * @return std::vector<double> vector of IoU values for each category.
 * @throw invalid_argument if the segmented image is empty.
 */
std::vector<double> IoUSegmentation(const cv::Mat &segmentedImage, const std::string& groundTruthMaskPath);


/**
 * @brief Compute the Average Precision (AP) for ball detection of a specific category.
 * @param detectedBalls vector of detected balls.
 * @param groundTruthBboxes vector of pairs of (Rect, Category) that represent the ground truth bounding boxes.
 * @param cat Category.
 * @param iouThreshold Intersection over Union threshold.
 * @return double AP value.
 * @throw invalid_argument if the vector of detected balls is empty or if the vector of ground truth bounding boxes is empty.
 */
double APBallCategory(const cv::Ptr<std::vector<Ball>> &detectedBalls, const std::vector<std::pair<cv::Rect, Category>> &groundTruthBboxes, Category cat, float iouThreshold);

/**
 * @brief Compute the Intersection over Union between the segmented image and the ground truth mask of a specific category.
 * @param segmentedImage segmented image.
 * @param groundTruthMask ground truth mask.
 * @param cat Category.
 * @return double IoU value.
 * @throw invalid_argument if the segmented image is empty or if the ground truth mask is empty.
 */
double IoUCategory(const cv::Mat &segmentedImage, const cv::Mat &groundTruthMask, const Category &cat);


/**
 * @brief Compute the Intersection over Union between two Rect.
 * @param rect1 first Rect.
 * @param rect2 second Rect.
 * @return double IoU value.
 * @throw invalid_argument if one of the two Rect is empty.
 */
double IoU(const cv::Rect &rect1, const cv::Rect &rect2);

/**
 * @brief Compute the Intersection over Union between two binary masks.
 * @param mask1 first binary mask.
 * @param mask2 second binary mask.
 * @return double IoU value.
 * @throw invalid_argument if one of the two masks is empty.
 */
double IoU(const cv::Mat &mask1, const cv::Mat &mask2);

#endif //METRICS_H
