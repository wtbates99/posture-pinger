#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <memory>
#include <vector>

class PoseDetector {
public:
    PoseDetector();
    ~PoseDetector();

    bool Initialize();
    bool CheckPosture();

private:
    cv::VideoCapture m_camera;
    cv::dnn::Net m_net;
    
    // Camera settings
    const int FRAME_WIDTH = 640;
    const int FRAME_HEIGHT = 480;
    const int PROCESS_WIDTH = 256;  // Smaller size for processing
    const int PROCESS_HEIGHT = 256;
    
    // Pose detection parameters
    const float CONFIDENCE_THRESHOLD = 0.5f;
    const float POSTURE_ANGLE_THRESHOLD = 15.0f;
    
    // Key point indices for posture analysis
    const int NECK_INDEX = 1;
    const int LEFT_SHOULDER_INDEX = 5;
    const int RIGHT_SHOULDER_INDEX = 2;
    
    bool InitializeCamera();
    bool LoadModel();
    std::vector<cv::Point2f> DetectKeypoints(const cv::Mat& frame);
    float CalculatePostureAngle(const std::vector<cv::Point2f>& keypoints);
    cv::Mat PreprocessFrame(const cv::Mat& frame);
}; 