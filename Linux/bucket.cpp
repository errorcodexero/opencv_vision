//#include "stdafx.h"
#include "bucket.h"
#include <vector>


std::string bucket::intToString(int number) {

	//this function has a number input and string output
	std::stringstream ss;
	ss << number;
	return ss.str();
}

bucket::bucket(Mat frame, Scalar low_threshold, Scalar high_threshold)
{
	this->frame = frame;
	this->frame_original = frame;
	this->low_threshold = low_threshold;
	this->high_threshold = high_threshold;
}

Mat bucket::colorFilter(Mat frame, std::string arg ) {
	if (arg == "contours") {
		Mat hsv_frame;
		cvtColor(frame_original, hsv_frame, COLOR_RGB2HSV);
		Mat mask, filtered;
		inRange(hsv_frame, low_threshold, high_threshold, mask);
		bitwise_and(frame, frame, filtered, mask);
		cvtColor(filtered, filtered, COLOR_BGR2GRAY);
		return filtered;
	}
	else if (arg == "blobs") {
		Mat hsv_frame;
		cvtColor(frame_original, hsv_frame, COLOR_RGB2HSV);
		Mat mask, filtered;
		inRange(hsv_frame, low_threshold, high_threshold, mask);
		bitwise_and(frame, frame, filtered, mask);
		//std::cout << filtered.type() << std::endl;
		return filtered;
	}
	else return Mat();
}



bool bucket::detectContours(Mat frame, std::vector<std::vector<Point>> &contours)
{
	int k_size;
	std::vector<Vec4i> hierachy;
	Mat canny_output;
	blur(frame, frame, Size(5,5));
	Canny(frame, canny_output, 25, 75);
	findContours(canny_output, contours, hierachy, CV_RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	//filterContourArea(contours, 200);
	Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
	drawContours(drawing, contours, -1, Scalar(255,255,255),1,8,hierachy,1);
	imshow("contours", drawing);
	return true;
}


bool bucket::poly(std::vector<std::vector<Point>> contours)
{
	//std::vector<std::vector<Point>>::iterator i = contours.begin();
	//std::vector<std::vector<Point>> derivedPolygon;
	auto derivedPolygon = contours;
	std::vector<Rect> rectangles;
	Mat rects = Mat::zeros(Size(frame_original.cols, frame_original.rows), frame_original.type());
	for (size_t i = 0; i < contours.size(); i++) {
		double epsilon = 0.1 * arcLength(contours[i], true);
		approxPolyDP(contours[i], derivedPolygon[i], epsilon, true);
		rectangles.push_back(boundingRect(derivedPolygon[i]));
		rectangle(rects, rectangles[i], Scalar(255, 255, 255));
	}
	Mat poly = Mat::zeros(Size(frame_original.cols,frame_original.rows),frame_original.type());
	drawContours(poly, derivedPolygon, -1, Scalar(255, 255, 255));
	imshow("rects", rects);
	
	imshow("poly", poly);
	return true;
}

bool bucket::filterContourArea(std::vector<std::vector<Point>>& contours, double limit)			//Not Working
{
	for (size_t i = 0; i < contours.size(); i++) {
		//if (contourArea(contours[i]) < limit) contours.erase(contours.begin() + i);
		if (contours[i].size() < limit) contours.erase(contours.begin() + i);
	};
	Mat drawing = Mat::zeros(frame_original.size(), frame_original.type());
	drawContours(drawing, contours, -1, Scalar(255, 255, 255));
	imshow("filtered contours", drawing);
	return true;
}

bool bucket::filterRecArea(std::vector<Rect>& rects, double limit)
{
	for (size_t i = 0; i < rects.size(); i++) {
		if (rects[i].area() < limit) rects.erase(rects.begin() + i);
	};
	return true;
}

void bucket::blobDetect()
{
	Mat blob=colorFilter(frame,"blobs");
	//Mat blob = frame;
	//imshow("blobs", blob);
	SimpleBlobDetector::Params params;


	//Color
	params.blobColor = 255;

	// Change thresholds
	params.minThreshold = 200;
	params.maxThreshold = 250;
	params.thresholdStep = 25;

	// Filter by Area.
	params.filterByArea = true;
	params.minArea = 1000;
	params.maxArea = 80000;

	params.filterByCircularity = false;
	params.filterByConvexity = false;
	params.filterByInertia = false;


	// Detect blobs.
	//std::vector<KeyPoint> keypoints;
	Ptr<SimpleBlobDetector> detector = SimpleBlobDetector::create(params);
	detector->detect(blob, Keypoints);



	// Draw detected blobs as red circles.
	// DrawMatchesFlags::DRAW_RICH_KEYPOINTS flag ensures the size of the circle corresponds to the size of blob
	Mat im_with_keypoints;
	drawKeypoints(blob, Keypoints, im_with_keypoints, Scalar(0, 0, 255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

	// Show blobs
	
	
	for (int i = 0; i < Keypoints.size(); i++) {
		Point p = Keypoints[i].pt;
		circle(frame, p, 20, Scalar(0, 255, 0), 2);
		line(frame, p, Point(p.x, p.y - 25), Scalar(0, 255, 0), 2);
		line(frame, p, Point(p.x, p.y + 25), Scalar(0, 255, 0), 2);
		line(frame, p, Point(p.x - 25, p.y), Scalar(0, 255, 0), 2);
		line(frame, p, Point(p.x + 25, p.y), Scalar(0, 255, 0), 2);
		putText(frame, "Tracking object at (" + intToString(p.x) + "," + intToString(p.y) + ")", p, 1, 1, Scalar(255, 0, 0), 2);
	}
	//imshow("keypoints", frame);
	//return Scalar();
}



Scalar bucket::detect()
{
	colorFilter(frame);
	imshow("filtered", frame);
	detectContours(frame,contours);
	filterContourArea(contours, 500);
	poly(contours);
	return Scalar(0,0);
}

bucket::~bucket()
{
}
