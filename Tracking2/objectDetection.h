#pragma once
#include <opencv2\opencv.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\features2d.hpp>
#include <string>
using namespace cv;


class objectDetection {
private:
	friend class bucket;
	virtual Mat colorFilter(Mat frame);
	virtual bool contours(Mat frame);
	virtual bool boundingRec();
	virtual bool paintTarget(Scalar pos);
protected:
	Mat frame, frame_original;
	Scalar position;
public:
	virtual Scalar detect(Mat frame);
};