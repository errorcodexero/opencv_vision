//#include "stdafx.h"
#include "bucket.h"
#include <vector>

Scalar hsvlow(30, 20, 40), hsvhigh(85, 255, 255);
Scalar grnlow(0, 0, 0), grnhigh(255, 255, 255) ;
//Scalar hsvlow(0, 0, 0), hsvhigh(180, 255, 255);

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



Mat bucket::colorFilter(Mat frame, std::string arg) {
  if (arg == "contours") {
#ifdef COLORFILTER
    Mat bgr_frame ;
#endif
    Mat mask, filtered ;
    Mat hsv_frame;

#ifdef COLORFILTER
    inRange(frame, grnlow, grnhigh, mask);
    bitwise_and(frame, frame, bgr_frame, mask);
#endif		

#ifdef COLORFILTER
    cvtColor(bgr_frame, hsv_frame, COLOR_RGB2HSV);
#else
    cvtColor(frame, hsv_frame, COLOR_RGB2HSV) ;
#endif
    
    inRange(hsv_frame, hsvlow, hsvhigh, mask);
    bitwise_and(frame, frame, filtered, mask);
    
    return filtered;
  }
  else if (arg == "blobs") {
    Mat hsv_frame;
    cvtColor(frame, hsv_frame, COLOR_RGB2HSV);
    imshow("hsv", hsv_frame);
    Mat mask, filtered;
    inRange(hsv_frame, hsvlow, hsvhigh, mask);
    bitwise_and(frame, frame, filtered, mask);
    //std::cout << filtered.type() << std::endl;
    return filtered;
  }
  else if (arg == "gray") {
    Mat gray, thresholded;
    cvtColor(frame, gray, COLOR_BGR2GRAY);
    imshow("gray", gray);
    Mat mask;
    Mat filtered(frame.rows, frame.cols, frame.type());
    inRange(gray, low_threshold, high_threshold, mask);
    bitwise_and(gray, gray, filtered, mask);
    //cvtColor(frame,frame,COLOR_BGR2GRAY);
    imshow("f", filtered);
    return filtered;

  }
  else return Mat();
}



bool bucket::detectContours(Mat frame, std::vector<std::vector<Point>> &contours)
{
  /*frame = colorFilter(frame_original, "gray");
    threshold(frame, frame, 200, 255, 3);
    imshow("Thresholded", frame);*/
  std::vector<Vec4i> hierachy;
  Mat canny_output;
  blur(frame, frame, Size(5, 5));
  Canny(frame, canny_output, 25, 75);
  findContours(canny_output, contours, hierachy, CV_RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
  //filterContourArea(contours, 200);
  Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
  drawContours(drawing, contours, -1, Scalar(255, 255, 255), 1, 8, hierachy, 1);
  //imshow("contours", drawing);
  return true;
}


std::vector<Rect> bucket::poly(Mat frame, std::vector<std::vector<Point>> contours)
{
  //std::vector<std::vector<Point>>::iterator i = contours.begin();
  //std::vector<std::vector<Point>> derivedPolygon;
  auto derivedPolygon = contours;
  std::vector<Rect> rectangles;
  Mat rects = Mat::zeros(Size(frame.cols, frame.rows), CV_8U);
  for (size_t i = 0; i < contours.size(); i++) {
    /*double epsilon = 0.1 * arcLength(contours[i], true);
      approxPolyDP(contours[i], derivedPolygon[i], epsilon, true);*/
    Rect contour_bound = boundingRect((contours[i]));
    if ((((contour_bound.width / contour_bound.height) > 2.3) || ((contour_bound.width / contour_bound.height) < 0.5)) && (contour_bound.area() > 100) && ((contour_bound.width / contour_bound.height) > 0))
      rectangles.push_back(contour_bound);

  }
  /*Mat poly = Mat::zeros(Size(frame.cols, frame.rows), frame.type());
  //drawContours(poly, derivedPolygon, -1,255);

  //std::cout << rectangles.size() << std::endl;
  //Merge overlapped
  bool changed = true;
  while (changed) {
  std::vector<Rect> orig = rectangles;
  rectangles.clear();
  changed = false;
  for (size_t i = 0; i< orig.size(); i++) {
  Rect r = orig[i];
  size_t j = i + 1;
  while (j<orig.size()) {
  Rect intersect = r & orig[j];
  if (intersect.width>0 && intersect.height>0) {
  r = r | orig[j];
  orig.erase(orig.begin() + j);
  changed = true;

  }
  else j++;
  }
  if ((r.area()>200) && (r.width / r.height < 0.5)) rectangles.push_back(r);
  }

  }
  //std::cout << rectangles.size() << std::endl;
  /*for (unsigned int i =0; i< rectangles.size(); i++) {
  rectangle(rects, rectangles[i], Scalar(255, 255, 255));
  }*/
  //imshow("rects", rects);
  //imshow("poly", poly);*/
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
  Point pmin(0, 0);
  for (unsigned int i = 0; i < Keypoints.size(); i++) {
    Point p = Keypoints[i].pt;
    for (unsigned int j = 0; j<rectangles.size(); j++) {
      Rect r = rectangles[j];
      if (p.inside(r))
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


int bucket::xError(Point p) {
  double r;
  int percent;
  if ((p.x==0) && (p.y==0)) return -1000;
  else {
    if (p.x<300) {
      r = p.x - 300;
      percent = r/300*100;
    }
    else if (p.x>340) {
      r = p.x - 340;
      percent = r/300*100;
    }
    else return 0;
  }
	
	
  return percent;
}

bool bucket::detect(Point &value, int &max_area, double &ratio)
{
  frame =	colorFilter(frame_original,"contours");
	
  //imshow("filtered", frame);
  detectContours(frame, contours);
  std::vector<Rect> rectangles = poly(frame, contours);
  /*Mat recs=Mat::zeros(Size(frame.cols, frame.rows), frame.type());
    for (int i = 0; i < rectangles.size(); i++) {
    rectangle(recs, rectangles[i],Scalar(255,255,255));
    }
    imshow("recs", recs);*/
  //int max_area = 0;
  Point flag(0,0);
  max_area = 0;
  ratio = 0;
  for (int i = 0; i < rectangles.size(); i++) {
		
    Rect r = rectangles[i];
    if (rectangles[i].area() > max_area) {
      flag.x = r.x + r.width / 2; flag.y = r.y + r.height / 2;
      max_area = rectangles[i].area();
      ratio = rectangles[i].width / rectangles[i].height;
    }
  }
  circle(frame, flag, 20, Scalar(0, 255, 0), 2);
  line(frame, flag, Point(flag.x, flag.y + 25), Scalar(0, 255, 0), 2);
  line(frame, flag, Point(flag.x, flag.y - 25), Scalar(0, 255, 0), 2);
  line(frame, flag, Point(flag.x - 25, flag.y), Scalar(0, 255, 0), 2);
  line(frame, flag, Point(flag.x + 25, flag.y), Scalar(0, 255, 0), 2);
  putText(frame, "Tracking object at (" + intToString(flag.x) + "," + intToString(flag.y) + ")", flag, 1, 1, Scalar(255, 0, 0), 2);
  //imshow("Targets", frame);
  //filterContourArea(contours, 500);
  //poly(contours);

  bool ret = false ;
  if (flag.x != 0 && flag.y != 0) {
    ret = true ;
    value = flag ;
  }
	
  return ret ;
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

Mat bucket::showFrame() {
  imshow("output", frame);
  Mat return_frame = frame;
  return return_frame;
}

bucket::~bucket()
{
}
