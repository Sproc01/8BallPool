# 8BallPool
In this repository there are different executables:
- `8BallPool`: the main executable that, given a video file path from command line input, processes it and creates the output video with the superimposed minimap.
- `TestAllClip`: it is the executable used to test the detection and segmentation in the first and last frame of all videos through AP and IoU by comparing them with the ground truth.
- `ShowSegmentationColored`: is a helper executable that has been used to show the ground truth of the segmentation of a particular frame using human-readable colors and it was also used as a test for the code that computes the metrics because it computes the performance of the ground truth on itself.
- `ComputePerformance`: is used to compute the performance across the dataset so the mAP and the mIoU.

For more information read the [report](Report/main.pdf).
