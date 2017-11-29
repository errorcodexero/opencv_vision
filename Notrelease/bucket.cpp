//#include "stdafx.h"
#include "bucket.h"
#include <vector>

Scalar hsvlow(0, 0, 150), hsvhigh(180, 30, 230);

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


/*Mat bucket::yuvEqualize(Mat frame) {
	cvtColor(frame,frame,CV_BGR2YUV);
	std::vector<Mat> channels;
	split(frame,channels);
	equalizeHist(channels[0],channels[0]);
	merge(channels, frame);
	cvtColor(frame,frame,CV_YUV2BGR);
	imshow("YUV", frame);
	return frame;
}*/

Mat bucket::colorFilter(Mat frame, std::string arg ) {
	if (arg == "contours") {
		Mat hsv_frame;
		cvtColor(frame, hsv_frame, COLOR_RGB2HSV);
		Mat mask, filtered;
		inRange(hsv_frame, hsvlow, hsvhigh, mask);
		bitwise_and(frame, frame, filtered, mask);
		//cvtColor(filtered, filtered, COLOR_BGR2GRAY);
		imshow("hsv", filtered);
		return filtered;
	}
	else if (arg == "blobs") {
		Mat hsv_frame;
		cvtColor(frame, hsv_frame, COLOR_RGB2HSV);
		Mat mask, filtered;
		inRange(hsv_frame, hsvlow, hsvhigh, mask);
		bitwise_and(frame, frame, filtered, mask);
		//std::cout << filtered.type() << std::endl;
		return filtered;
	}
	else if (arg=="gray") {
		Mat gray, thresholded;
		cvtColor(frame,gray,COLOR_BGR2GRAY);
		imshow("gray", gray);
		Mat mask;
		Mat filtered(frame.rows, frame.cols, frame.type());
		inRange(gray, low_threshold, high_threshold, mask);
		bitwise_and(gray, gray, filtered, mask);
		//cvtColor(frame,frame,COLOR_BGR2GRAY);
		imshow("f",filtered);
		return filtered;

	}
	else return Mat();
}



bool bucket::detectContours(Mat frame, std::vector<std::vector<Point>> &contours)
{
	frame = colorFilter(frame_original,"gray");
	threshold(frame,frame,200,255,3);
	imshow("Thresholded", frame);
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

int bucket::xError(Point p) {
	int r;
	if (p.x<200) r = p.x-200;
	else if (p.x>280) r = p.x-280;
	else r =0;
	return r;
}
std::vector<Rect> bucket::poly(Mat frame, std::vector<std::vector<Point>> contours)
{
	//std::vector<std::vector<Point>>::iterator i = contours.begin();
	//std::vector<std::vector<Point>> derivedPolygon;
	auto derivedPolygon = contours;
	std::vector<Rect> rectangles;
	Mat rects = Mat::zeros(Size(frame.cols, frame.rows), CV_8U);
	for (size_t i = 0; i < contours.size(); i++) {
		double epsilon = 0.1 * arcLength(contours[i], true);
		approxPolyDP(contours[i], derivedPolygon[i], epsilon, true);
		rectangles.push_back(boundingRect(derivedPolygon[i]));

	}
	Mat poly = Mat::zeros(Size(frame.cols,frame.rows),frame.type());
	//drawContours(poly, derivedPolygon, -1,255);

	//std::cout << rectangles.size() << std::endl;
	//Merge overlapped
	bool changed = true;
	while(changed) {
		std::vector<Rect> orig = rectangles;
		rectangles.clear();
		changed = false;
		for (size_t i = 0; i< orig.size(); i++) {
			Rect r = orig[i];
			size_t j = i+1;
			while (j<orig.size()) {
				Rect intersect = r & orig[j];
				if(intersect.width>0 && intersect.height>0) {
					r = r | orig[j];
					orig.erase(orig.begin()+j);
					changed = true;

				}
				else j++;
			}
			if ((r.area()>400) && (r.width/r.height < 0.8)) rectangles.push_back(r);
		}

	}
	//std::cout << rectangles.size() << std::endl;
	/*for (unsigned int i =0; i< rectangles.size(); i++) {
		rectangle(rects, rectangles[i], Scalar(255, 255, 255));
	}*/
	//imshow("rects", rects);
	//imshow("poly", poly);
	return rectangles;
}

bool bucket::filterContourArea(std::vector<std::vector<Point>>& contours, double limit)			//Not Working
{
	for (unsigned int i = 0; i < contours.size(); i++) {
		if (contourArea(contours[i]) < limit) contours.erase(contours.begin() + i);
		//if (contours[i].size() < limit) contours.erase(contours.begin() + i);
	};
	Mat drawing = Mat::zeros(frame_original.size(), frame_original.type());
	drawContours(drawing, contours, -1, Scalar(255, 255, 255));
	//imshow("filtered contours", drawing);
	return true;
}

bool bucket::filterRecArea(std::vector<Rect>& rects, double limit)
{
	for (size_t i = 0; i < rects.size(); i++) {
		if (rects[i].area() < limit) rects.erase(rects.begin() + i);
	};
	return true;
}

int bucket::blobDetect()
{
	//Mat blob=colorFilter(frame,"gray");
	//Mat blob = frame;
	//imshow("blobs", blob);
	//Mat hsv_frame = colorFilter(frame_original, "contours");
	Mat blob = colorFilter(frame_original, "gray");
	//Mat blob = hsv_frame;
	SimpleBlobDetector::Params params;


	//Color
	params.blobColor = 255;

	// Change thresholds
	params.minThreshold = 130;
	params.maxThreshold = 200;
	params.thresholdStep = 10;

	// Filter by Area.
	params.filterByArea = true;
	params.minArea = 1500;
	params.maxArea = 90000;

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
	imshow("keypoints", im_with_keypoints);
	std::vector<Vec4i> hierachy;
	Mat canny_output;
	std::vector<Point> Targets;
	Canny(blob, canny_output, 25, 75);
	findContours(canny_output, contours, hierachy, CV_RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	//filterContourArea(contours, 0);
	//poly(contours);
	//Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
	//drawContours(drawing, contours, -1, Scalar(255,255,255),1,8,hierachy,1);
	//imshow("contours", drawing);
	//filterContourArea(contours, 200);
	std::vector<Rect> rectangles = poly(frame, contours);
	Point pmin(0,0);
	for (unsigned int i = 0; i < Keypoints.size(); i++) {
		Point p = Keypoints[i].pt;
		for(unsigned int j = 0;j<rectangles.size();j++) {
			Rect r = rectangles[j];
			if(p.inside(r))
			if (p.y>pmin.y) pmin = p;
			}
		}
	circle(frame, pmin, 20, Scalar(0, 255, 0), 2);
	line(frame, pmin, Point(pmin.x, pmin.y + 25), Scalar(0, 255, 0), 2);
	line(frame, pmin, Point(pmin.x, pmin.y - 25), Scalar(0, 255, 0), 2);
	line(frame, pmin, Point(pmin.x - 25, pmin.y), Scalar(0, 255, 0), 2);
	line(frame, pmin, Point(pmin.x + 25, pmin.y), Scalar(0, 255, 0), 2);
	putText(frame, "Tracking object at (" + intToString(pmin.x) + "," + intToString(pmin.y) + ")", pmin, 1, 1, Scalar(255, 0, 0), 2);
	//imshow("keypoints", frame);
	//return Scalar();
	return xError(pmin);
}




Scalar bucket::detect()
{
	colorFilter(frame);
	imshow("filtered", frame);
	detectContours(frame,contours);
	filterContourArea(contours, 500);
	//poly(contours);
	return Scalar(0,0);
}

/*void bucket::showContours() {
	/*Mat gray_frame,frame;
	gray_frame = colorFilter(frame_original,"gray");
	threshold(gray_frame,frame,200,255,3);
	imshow("Thresholded",frame);
	//Mat frame_gray = colorFilter(frame_original,"gray");

	//Mat frame = colorFilter(frame_gray, "contours");
	Mat yuv_eq = yuvEqualize(frame_original);
	//Mat hsv_frame = colorFilter(frame_original, "contours");
	//Mat frame = colorFilter(hsv_frame, "gray");
	Mat hsv_frame = colorFilter(yuv_eq, "contours");
	Mat frame = colorFilter(hsv_frame, "gray");
	std::vector<Vec4i> hierachy;
	Mat canny_output;
	//blur(frame, frame, Size(5,5));
	Canny(frame, canny_output, 25, 75);
	findContours(canny_output, contours, hierachy, CV_RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	filterContourArea(contours, 0);
	//poly(contours);
	Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
	drawContours(drawing, contours, -1, Scalar(255,255,255),1,8,hierachy,1);
	//imshow("contours", drawing);
	//filterContourArea(contours, 200);
	poly(frame, contours);
}*/

bucket::~bucket()
{
}
