#include <opencv2/opencv.hpp>
#include <memory>
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        // Initialize OpenCV camera
        cv::VideoCapture camera(0);
        if (!camera.isOpened()) {
            std::cerr << "Error: Could not open camera" << std::endl;
            return 1;
        }

        // Main loop
        cv::Mat frame;
        while (true) {
            camera >> frame;
            if (frame.empty()) {
                std::cerr << "Error: Could not read frame" << std::endl;
                break;
            }

            // Display frame (for testing)
            cv::imshow("Posture Checker", frame);

            // Break loop on 'q' press
            if (cv::waitKey(1) == 'q') {
                break;
            }
        }

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
} 