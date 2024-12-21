import cv2
import pose_detector
import score_history
from threading import Thread, Lock
import time
import queue
import select
import sys


class PoseTracker:
    def __init__(self):
        self.cap = cv2.VideoCapture(0)
        self.detector = pose_detector.pose_detector()
        self.s_history = score_history.score_history()
        self.is_tracking = False
        self.show_video = False
        self.running = True
        self.frame_lock = Lock()
        self.current_frame = None
        self.frame_queue = queue.Queue(maxsize=1)

    def toggle_tracking(self):
        self.is_tracking = not self.is_tracking
        return self.is_tracking

    def toggle_video(self):
        was_showing = self.show_video
        self.show_video = not self.show_video
        if was_showing:
            cv2.destroyAllWindows()
            cv2.waitKey(1)  # Force window destruction
        return self.show_video

    def stop(self):
        self.running = False
        self.cap.release()
        cv2.destroyAllWindows()
        cv2.waitKey(1)  # Force window destruction

    def run(self):
        while self.running:
            ret, frame = self.cap.read()
            if not ret:
                continue

            if self.is_tracking:
                try:
                    frame, score = self.detector.process_frame(frame)
                    self.s_history.add_score(score)
                    avg_score = self.s_history.get_average_score()
                    print(
                        f"Average score over last {self.s_history.WINDOW_SIZE} seconds: {avg_score:.2f}"
                    )
                except Exception as e:
                    print(f"Error processing frame: {e}")
                    continue

            if self.show_video:
                try:
                    while not self.frame_queue.empty():
                        self.frame_queue.get_nowait()
                    self.frame_queue.put(frame)
                except queue.Full:
                    pass

            time.sleep(0.01)  # Small delay to prevent high CPU usage


def main():
    tracker = PoseTracker()
    tracking_thread = Thread(target=tracker.run)
    tracking_thread.start()

    try:
        while True:
            # Non-blocking input check
            if select.select([sys.stdin], [], [], 0.01)[0]:  # 10ms timeout
                cmd = input().strip()
                if cmd == "t":
                    print("Tracking:", tracker.toggle_tracking())
                elif cmd == "v":
                    print("Video:", tracker.toggle_video())
                elif cmd == "q":
                    break

            # Handle video display continuously
            if tracker.show_video:
                try:
                    frame = tracker.frame_queue.get_nowait()
                    cv2.imshow("Pose Tracking", frame)
                    if cv2.waitKey(1) & 0xFF == ord("q"):
                        break
                except queue.Empty:
                    pass
                except Exception as e:
                    print(f"Error displaying frame: {e}")
                    tracker.show_video = False
                    cv2.destroyAllWindows()
                    cv2.waitKey(1)  # Force window destruction

    finally:
        tracker.stop()
        tracking_thread.join()


if __name__ == "__main__":
    main()
