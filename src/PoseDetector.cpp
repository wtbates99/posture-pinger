#include "PoseDetector.hpp"
#include <filesystem>

PoseDetector::PoseDetector() {
}

PoseDetector::~PoseDetector() {
    if (m_camera.isOpened()) {
        m_camera.release();
    }
}

bool PoseDetector::Initialize() {
    if (!InitializeCamera()) {
        return false;
    }
    
    if (!LoadModel()) {
        return false;
    }
    
    return true;
}

bool PoseDetector::InitializeCamera() {
    m_camera.open(0);  // Open default camera
    if (!m_camera.isOpened()) {
        return false;
    }
    
    m_camera.set(cv::CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
    m_camera.set(cv::CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);
    m_camera.set(cv::CAP_PROP_FPS, 30);
    
    return true;
}

bool PoseDetector::LoadModel() {
    try {
        // Load OpenPose or MediaPipe model
        std::filesystem::path modelPath = std::filesystem::current_path() / "models";
        m_net = cv::dnn::readNetFromTensorflow(
            (modelPath / "pose_model.pb").string(),
            (modelPath / "pose_model.pbtxt").string()
        );
        
        m_net.setPreferableBackend(cv::dnn::DNN_BACKEND_DEFAULT);
        m_net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
        
        return true;
    }
    catch (const cv::Exception& e) {
        // Log error
        return false;
    }
}

bool PoseDetector::CheckPosture() {
    cv::Mat frame;
    m_camera >> frame;
    
    if (frame.empty()) {
        return false;
    }
    
    // Process frame and detect pose
    cv::Mat processedFrame = PreprocessFrame(frame);
    std::vector<cv::Point2f> keypoints = DetectKeypoints(processedFrame);
    
    if (keypoints.empty()) {
        return true;  // Assume good posture if detection fails
    }
    
    // Calculate angle between neck and shoulders
    float angle = CalculatePostureAngle(keypoints);
    
    // Return true if posture is good (angle within threshold)
    return std::abs(angle) < POSTURE_ANGLE_THRESHOLD;
}

cv::Mat PoseDetector::PreprocessFrame(const cv::Mat& frame) {
    cv::Mat processed;
    cv::resize(frame, processed, cv::Size(PROCESS_WIDTH, PROCESS_HEIGHT));
    
    // Convert to blob for neural network
    cv::Mat inputBlob = cv::dnn::blobFromImage(processed, 
        1.0 / 255.0,                    // scale
        cv::Size(PROCESS_WIDTH, PROCESS_HEIGHT),
        cv::Scalar(0, 0, 0),           // mean
        true,                          // swapRB
        false                          // crop
    );
    
    return inputBlob;
}

std::vector<cv::Point2f> PoseDetector::DetectKeypoints(const cv::Mat& frame) {
    std::vector<cv::Point2f> keypoints;
    
    try {
        m_net.setInput(frame);
        cv::Mat output = m_net.forward();
        
        // Process the output to get keypoint coordinates
        // This implementation will depend on the specific model used
        // Here's a simplified version:
        for (int i = 0; i < output.size[1]; i++) {
            float confidence = output.at<float>(0, i, 2);
            if (confidence > CONFIDENCE_THRESHOLD) {
                float x = output.at<float>(0, i, 0) * PROCESS_WIDTH;
                float y = output.at<float>(0, i, 1) * PROCESS_HEIGHT;
                keypoints.emplace_back(x, y);
            }
        }
    }
    catch (const cv::Exception& e) {
        // Log error
        return std::vector<cv::Point2f>();
    }
    
    return keypoints;
}

float PoseDetector::CalculatePostureAngle(const std::vector<cv::Point2f>& keypoints) {
    if (keypoints.size() <= std::max({NECK_INDEX, LEFT_SHOULDER_INDEX, RIGHT_SHOULDER_INDEX})) {
        return 0.0f;
    }
    
    // Calculate angle between neck and shoulders
    cv::Point2f neck = keypoints[NECK_INDEX];
    cv::Point2f leftShoulder = keypoints[LEFT_SHOULDER_INDEX];
    cv::Point2f rightShoulder = keypoints[RIGHT_SHOULDER_INDEX];
    
    // Calculate midpoint of shoulders
    cv::Point2f shoulderMidpoint(
        (leftShoulder.x + rightShoulder.x) / 2.0f,
        (leftShoulder.y + rightShoulder.y) / 2.0f
    );
    
    // Calculate angle between vertical line and neck-shoulder line
    float dx = shoulderMidpoint.x - neck.x;
    float dy = shoulderMidpoint.y - neck.y;
    float angle = std::atan2(dx, dy) * 180.0f / CV_PI;
    
    return angle;
} 