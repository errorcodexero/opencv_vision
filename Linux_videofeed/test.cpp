#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <libv4l2.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <fcntl.h>
 
using namespace cv;
using namespace std;
 
int main(int argc,char ** argv)
{
	int descriptor = v4l2_open("/dev/video0", O_RDWR);
	v4l2_control c;
c.id = V4L2_CID_EXPOSURE_AUTO_PRIORITY;
c.value = 3;
if(v4l2_ioctl(descriptor, VIDIOC_S_CTRL, &c) == 0)
    cout << "success";
c.id = V4L2_CID_EXPOSURE_AUTO;
c.value = V4L2_EXPOSURE_MANUAL;
if(v4l2_ioctl(descriptor, VIDIOC_S_CTRL, &c) == 0)
    cout << "success";

// manual brigctness control
c.id = V4L2_CID_BRIGHTNESS;
c.value = 100;
if(v4l2_ioctl(descriptor, VIDIOC_S_CTRL, &c) == 0)
{
    cout << c.value << " success";
}
c.id = V4L2_CID_EXPOSURE_ABSOLUTE;
c.value = 10;
if(v4l2_ioctl(descriptor, VIDIOC_S_CTRL, &c) == 0)
    cout << "success";
 //auto priority control

  VideoCapture cap(0);
  if (!cap.isOpened()) {
    cerr << "ERROR: Unable to open the camera" << endl;
    return 0;
  }
 
  Mat frame;
  cout << "Start grabbing, press a key on Live window to terminate" << endl;
  while(1) {
    cap >> frame;
    if (frame.empty()) {
        cerr << "ERROR: Unable to grab from the camera" << endl;
        break;
    }
    imshow("Live",frame);
    int key = cv::waitKey(5);
    key = (key==255) ? -1 : key; //#Solve bug in 3.2.0
    if (key>=0)
      break;
  }
 
  cout << "Closing the camera" << endl;
  cap.release();
  destroyAllWindows();
  cout << "bye!" <<endl;
  return 0;
}
