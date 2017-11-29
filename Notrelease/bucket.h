//Contours are useless and CPU demanding


#pragma once
#include "objectDetection.h"
#include <vector>


class bucket :
	public objectDetection
{
private:
	Mat yuvEqualize(Mat frame);
	std::vector<KeyPoint> Keypoints;
	std::string intToString(int number);
	Scalar low_threshold, high_threshold;
	virtual bool detectContours(Mat frame, std::vector<std::vector<Point>> &contours);
	virtual Mat colorFilter(Mat frame, std::string arg = "contours");
	std::vector<Rect> poly(Mat, std::vector<std::vector<Point>>);
	bool filterContourArea(std::vector<std::vector<Point>>& contours, double limit);
	bool filterRecArea(std::vector<Rect>& rects, double limit);
	//void mergeOverlapped(
	int xError(Point p);
protected:
	std::vector<std::vector<Point>> contours;
	//Scalar hsvlow(0, 0, 170), hsvhigh(180, 80, 255);
public:
	bucket(Mat frame, Scalar low_threshold, Scalar high_threshold);
	virtual Scalar detect();
	int blobDetect();
	void showContours();
	~bucket();
};

