//motionTracking.cpp

//Written by  Kyle Hounslow, January 2014

//Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software")
//, to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
//and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

//The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
//IN THE SOFTWARE.

//#include "stdafx.h"
#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <libv4l2.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "bucket.h"
#include "UDPBroadcastReceiver.h"
#include "UDPBroadcastSender.h"
#include "UDPHostClient.h"

using namespace std;
using namespace cv;

//our sensitivity value to be used in the threshold() function
const static int SENSITIVITY_VALUE = 20;
//size of blur used to smooth the image to remove possible noise and
//increase the size of the object we are trying to track. (Much like dilate and erode)
const static int BLUR_SIZE = 10;
//we'll have just one object to search for
//and keep track of its position.
int theObject[2] = { 0,0 };
//bounding rectangle of the object, we will use the center of this as its position.
Rect objectBoundingRectangle = Rect(0, 0, 0, 0);

//Scalar low(0, 0, 170), high(180, 80, 255);
Scalar low(150), high(230);


int xError(Point p) {
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


int main(int ac, char **av) {
  ac--;
  av++;
  boost::asio::io_service io_service_broadcast;
  UDPBroadcastSender BroadcastSender(io_service_broadcast, 1425);
	
  //some boolean variables for added functionality
  bool objectDetected = false;
	
  //these two can be toggled by pressing 'd' or 't'
  bool trackingEnabled = false;
	
  //pause and resume code
  bool pause = false;
	
  //set up the matrices that we will need
  //the two frames we will be comparing
  Mat frame1;
	
  //their grayscale images (needed for absdiff() function)
  // open capture
  int descriptor = v4l2_open("/dev/video0", O_RDWR);
  v4l2_control c;
  c.id = V4L2_CID_EXPOSURE_AUTO;
  c.value = 1;	
  if(v4l2_ioctl(descriptor, VIDIOC_S_CTRL, &c) == 0)
    cout << "success";
  c.id = V4L2_CID_EXPOSURE_AUTO_PRIORITY;
  c.value = 0;
  if (v4l2_ioctl(descriptor, VIDIOC_S_CTRL, &c) == 0)
    cout << "success";

  c.id = V4L2_CID_EXPOSURE_ABSOLUTE;
  c.value = 20;
  if (v4l2_ioctl(descriptor, VIDIOC_S_CTRL, &c) == 0)
    cout << "success";
  c.id = V4L2_CID_BRIGHTNESS;
  c.value = 80;
  if (v4l2_ioctl(descriptor, VIDIOC_S_CTRL, &c) == 0)
    cout << "success";	
  //video capture object.
  VideoCapture capture(0);
  //capture.set(CAP_PROP_BRIGHTNESS,0.1);

  Rect box ;
  int count = 0 ;
  int frame_number = 0;
  bool first = true ;
  int bwidth = 180 ;
  int bheight = 120 ;
  int framecnt = 0 ;
	
  /*VideoWriter output_stream;
    string filename = "output.avi";
	
    int fcc = CV_FOURCC('D', 'I', 'V', '3');
    int fps = 20;
    Size framesize(capture.get(CV_CAP_PROP_FRAME_WIDTH),capture.get(CV_CAP_PROP_FRAME_HEIGHT));
    output_stream = VideoWriter(filename, fcc, fps, framesize);
    if (!output_stream.isOpened()) { std::cout << "Write file error"; return -1; }*/
  while (1) {
		
    //we can loop the video by re-opening the capture every time the video reaches its last frame

    if (!capture.isOpened()) {
      cout << "ERROR ACQUIRING VIDEO FEED\n";
      getchar();
      return -1;
    }
    //check if the video has reach its last frame.
    //we add '-1' because we are reading two frames from the video at a time.
    //if this is not included, we get a memory error!

    //read first frame
    capture >> frame1;
    bucket b(frame1, low, high);
    //imshow("Frame1", frame1);
    Point value ;
    string xpos = "target=" ;
    int max_area = 0;
    Rect box;
    double ratio = -10;
    if (b.detect(value, max_area, ratio)) {
      if(first) {
	box = Rect(value.x - bwidth/2, value.y - bheight, bwidth, bheight) ;
	xpos += std::to_string(xError(value));
	first = false ;
      }
      else {
	if (value.inside(box)) {
	  box = Rect(value.x - bwidth/2, value.y - bheight, bwidth, bheight) ;
	  xpos += std::to_string(xError(value));
	  count = 0 ;
	}
	else {
	  count++;
	  if (count == 3) { 
	    count = 0;
	    box = Rect(value.x - bwidth/2, value.y - bheight, bwidth, bheight) ;
	    xpos += std::to_string(xError(value));
	  }
	  else {
	    xpos.clear() ;
	  }
	}
      }
    }
    else {
      xpos += "NONE" ;
    }

    if (xpos.length() > 0) {
      // cout << "Sent '" << xpos << "'" << endl ;
      BroadcastSender.send(xpos) ;
    }

    framecnt++ ;
    if (framecnt == 20) {
      b.showFrame() ;
      framecnt = 0 ;
    }
    
    
    //output_stream.write(b.showFrame());

    //check to see if a button has been pressed.
    //this 10ms delay is necessary for proper operation of this program
    //if removed, frames will not have enough time to referesh and a blank
    //image will appear.
    waitKey(10);
    /*switch (waitKey(10)) {

      case 27: //'esc' key has been pressed, exit program.
      return 0;
      case 116: //'t' has been pressed. this will toggle tracking
      trackingEnabled = !trackingEnabled;
      if (trackingEnabled == false) cout << "Tracking disabled." << endl;
      else cout << "Tracking enabled." << endl;
      break;
      case 100: //'d' has been pressed. this will debug mode
      debugMode = !debugMode;
      if (debugMode == false) cout << "Debug mode disabled." << endl;
      else cout << "Debug mode enabled." << endl;
      break;
      case 112: //'p' has been pressed. this will pause/resume the code.
      pause = !pause;
      if (pause == true) {
      cout << "Code paused, press 'p' again to resume" << endl;
      while (pause == true) {
      //stay in this loop until
      switch (waitKey()) {
      //a switch statement inside a switch statement? Mind blown.
      case 112:
      //change pause back to false
      pause = false;
      cout << "Code resumed." << endl;
      break;
      }
      }
      }


      }
      frame_number++;*/
  }
	
  //release the capture before re-opening and looping again.


  capture.release();
  return 0;

}
