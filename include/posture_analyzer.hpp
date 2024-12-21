#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <vector>

class PostureAnalyzer {
public:
    PostureAnalyzer();
    
    bool initialize();
    void processFrame(cv::Mat& frame);
    float getPostureScore() const { return postureScore; }
    void drawPoseWireframe(cv::Mat& frame);

private:
    struct KeyPoint {
        cv::Point2f point;
        float confidence;
    };

    float calculatePostureScore(const std::vector<KeyPoint>& keypoints);
    bool checkShoulderAlignment(const std::vector<KeyPoint>& keypoints);
    bool checkNeckAlignment(const std::vector<KeyPoint>& keypoints);
    bool checkSpineAlignment(const std::vector<KeyPoint>& keypoints);

    cv::dnn::Net net;
    float postureScore;
    std::vector<KeyPoint> currentPose;
    const int POSE_PAIRS[17][2] = {
        {1, 2}, {1, 5}, {2, 3}, {3, 4}, {5, 6}, {6, 7}, {1, 8}, {8, 9},
        {9, 10}, {1, 11}, {11, 12}, {12, 13}, {1, 0}, {0, 14}, {14, 16},
        {0, 15}, {15, 17}
    };
}; 