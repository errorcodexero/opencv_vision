//Contours are useless and CPU demanding


#pragma once
#include "objectDetection.h"


class bucket :
	public objectDetection
{
private:
	std::vector<KeyPoint> Keypoints;
	std::string intToString(int number);
	Scalar low_threshold, high_threshold;
	virtual bool detectContours(Mat frame, std::vector<std::vector<Point>> &contours);
	virtual Mat colorFilter(Mat frame, std::string arg = "contours");
	bool poly(std::vector<std::vector<Point>>);
	bool filterContourArea(std::vector<std::vector<Point>>& contours, double limit);
	bool filterRecArea(std::vector<Rect>& rects, double limit);
protected:
	std::vector<std::vector<Point>> contours;
public:
	bucket(Mat frame, Scalar low_threshold, Scalar high_threshold);
	virtual Scalar detect();
	void blobDetect();
	void showContours();
	~bucket();
};

