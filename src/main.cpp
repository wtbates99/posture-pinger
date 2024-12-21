#include <opencv2/opencv.hpp>
#include "posture_analyzer.hpp"

int main() {
    cv::VideoCapture cap(0); // Open default camera
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open camera." << std::endl;
        return -1;
    }

    PostureAnalyzer analyzer;
    if (!analyzer.initialize()) {
        std::cerr << "Error: Could not initialize pose analyzer." << std::endl;
        return -1;
    }

    cv::Mat frame;
    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        analyzer.processFrame(frame);
        analyzer.drawPoseWireframe(frame);

        // Display the frame
        cv::imshow("Posture Analyzer", frame);

        // Break loop on 'q' key
        if (cv::waitKey(1) == 'q') break;
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
} 