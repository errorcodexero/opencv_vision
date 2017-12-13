//Contours are useless and CPU demanding


#pragma once
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <string>
#include <opencv2/opencv.hpp>
using namespace cv;

class bucket {
private:
	std::vector<KeyPoint> Keypoints;
	std::string intToString(int number);
	Scalar low_threshold, high_threshold;
	virtual bool detectContours(Mat frame, std::vector<std::vector<Point>> &contours);
	virtual Mat colorFilter(Mat frame, std::string arg = "contours");
	std::vector<Rect> poly(Mat, std::vector<std::vector<Point>>);
	bool filterContourArea(std::vector<std::vector<Point>>& contours, double limit);
	bool filterRecArea(std::vector<Rect>& rects, double limit);
	int xError(Point p);
	//void mergeOverlapped(
protected:
	Mat frame, frame_original;
		Scalar position;
	std::vector<std::vector<Point>> contours;
	//Scalar hsvlow(0, 0, 170), hsvhigh(180, 80, 255);
public:
	bucket(Mat frame, Scalar low_threshold, Scalar high_threshold);
	virtual int detect();
	int blobDetect();
	void showContours();
	Mat showFrame();
	~bucket();
};

