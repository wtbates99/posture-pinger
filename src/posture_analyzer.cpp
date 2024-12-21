#include "posture_analyzer.hpp"

PostureAnalyzer::PostureAnalyzer() : postureScore(0.0f) {}

bool PostureAnalyzer::initialize() {
    try {
        // Load OpenCV's DNN model for pose estimation
        net = cv::dnn::readNetFromTensorflow("pose_estimation_model.pb");
        if (net.empty()) {
            std::cerr << "Error: Could not load pose estimation model" << std::endl;
            return false;
        }
        return true;
    }
    catch (const cv::Exception& e) {
        std::cerr << "Error initializing network: " << e.what() << std::endl;
        return false;
    }
}

void PostureAnalyzer::processFrame(cv::Mat& frame) {
    cv::Mat inputBlob = cv::dnn::blobFromImage(frame, 1.0/255.0, 
                                              cv::Size(256, 256), 
                                              cv::Scalar(0,0,0), 
                                              false, false);
    
    net.setInput(inputBlob);
    cv::Mat output = net.forward();
    
    // Process the output to get keypoints
    currentPose.clear();
    const int nPoints = output.size[1];
    for (int i = 0; i < nPoints; i++) {
        KeyPoint kp;
        // Get x,y coordinates
        float* data = output.ptr<float>(0, i);
        kp.point = cv::Point2f(data[0] * frame.cols, data[1] * frame.rows);
        kp.confidence = data[2];
        
        if (kp.confidence > 0.5) { // Only keep points with good confidence
            currentPose.push_back(kp);
        }
    }
    
    postureScore = calculatePostureScore(currentPose);
}

float PostureAnalyzer::calculatePostureScore(const std::vector<KeyPoint>& keypoints) {
    float score = 1.0f;
    
    if (!checkShoulderAlignment(keypoints)) score -= 0.3f;
    if (!checkNeckAlignment(keypoints)) score -= 0.3f;
    if (!checkSpineAlignment(keypoints)) score -= 0.4f;
    
    return std::max(0.0f, score);
}

bool PostureAnalyzer::checkShoulderAlignment(const std::vector<KeyPoint>& keypoints) {
    // Check if shoulders are level (points 5 and 2 in the model)
    if (keypoints.size() > 5) {
        float shoulderDiff = std::abs(keypoints[5].point.y - keypoints[2].point.y);
        return shoulderDiff < 20.0f; // Threshold for shoulder alignment
    }
    return false;
}

bool PostureAnalyzer::checkNeckAlignment(const std::vector<KeyPoint>& keypoints) {
    // Check neck alignment (points 0 and 1)
    if (keypoints.size() > 1) {
        float neckAngle = std::abs(std::atan2(
            keypoints[1].point.y - keypoints[0].point.y,
            keypoints[1].point.x - keypoints[0].point.x
        )) * 180.0f / CV_PI;
        return neckAngle > 80.0f && neckAngle < 100.0f;
    }
    return false;
}

bool PostureAnalyzer::checkSpineAlignment(const std::vector<KeyPoint>& keypoints) {
    // Check spine alignment (points 1, 8, and 9)
    if (keypoints.size() > 9) {
        cv::Point2f spine_vector = keypoints[9].point - keypoints[1].point;
        float spineAngle = std::abs(std::atan2(spine_vector.y, spine_vector.x)) * 180.0f / CV_PI;
        return spineAngle > 80.0f && spineAngle < 100.0f;
    }
    return false;
}

void PostureAnalyzer::drawPoseWireframe(cv::Mat& frame) {
    if (currentPose.empty()) return;
    
    // Draw keypoints
    for (const auto& kp : currentPose) {
        cv::circle(frame, kp.point, 5, cv::Scalar(0, 255, 0), -1);
    }
    
    // Draw connections
    for (const auto& pair : POSE_PAIRS) {
        if (pair[0] < currentPose.size() && pair[1] < currentPose.size()) {
            cv::line(frame, currentPose[pair[0]].point, 
                    currentPose[pair[1]].point, 
                    cv::Scalar(0, 255, 0), 2);
        }
    }
    
    // Draw posture score
    cv::putText(frame, 
                "Posture Score: " + std::to_string(postureScore),
                cv::Point(30, 30),
                cv::FONT_HERSHEY_SIMPLEX,
                1.0,
                cv::Scalar(0, 255, 0),
                2);
} 