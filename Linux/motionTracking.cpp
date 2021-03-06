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
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <libv4l2.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "bucket.h"

using namespace std;
using namespace cv;

//our sensitivity value to be used in the threshold() function
const static int SENSITIVITY_VALUE = 20;
//size of blur used to smooth the image to remove possible noise and
//increase the size of the object we are trying to track. (Much like dilate and erode)
const static int BLUR_SIZE = 10;
//we'll have just one object to search for
//and keep track of its position.
int theObject[2] = {0,0};
//bounding rectangle of the object, we will use the center of this as its position.
Rect objectBoundingRectangle = Rect(0,0,0,0);

//Scalar low(0, 0, 170), high(180, 80, 255);
Scalar low(150), high(230);


//int to string helper function
string intToString(int number){

	//this function has a number input and string output
	std::stringstream ss;
	ss << number;
	return ss.str();
}

/*void searchForMovement(Mat thresholdImage, Mat &cameraFeed){
	//notice how we use the '&' operator for the cameraFeed. This is because we wish
	//to take the values passed into the function and manipulate them, rather than just working with a copy.
	//eg. we draw to the cameraFeed in this function which is then displayed in the main() function.
	bool objectDetected=false;
	Mat temp;
	thresholdImage.copyTo(temp);
	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	//findContours(temp,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );// retrieves all contours
	findContours(temp,contours,hierarchy,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_SIMPLE );// retrieves external contours

	//if contours vector is not empty, we have found some objects
	if(contours.size()>0)objectDetected=true;
	else objectDetected = false;

	if(objectDetected){
		//the largest contour is found at the end of the contours vector
		//we will simply assume that the biggest contour is the object we are looking for.
		vector< vector<Point> > largestContourVec;
		largestContourVec.push_back(contours.at(contours.size()-1));
		//make a bounding rectangle around the largest contour then find its centroid
		//this will be the object's final estimated position.
		objectBoundingRectangle = boundingRect(largestContourVec.at(0));
		int xpos = objectBoundingRectangle.x+objectBoundingRectangle.width/2;
		int ypos = objectBoundingRectangle.y+objectBoundingRectangle.height/2;

		//update the objects positions by changing the 'theObject' array values
		theObject[0] = xpos , theObject[1] = ypos;
	}
	//make some temp x and y variables so we dont have to type out so much
	int x = theObject[0];
	int y = theObject[1];
	//draw some crosshairs on the object
	circle(cameraFeed,Point(x,y),20,Scalar(0,255,0),2);
	line(cameraFeed,Point(x,y),Point(x,y-25),Scalar(0,255,0),2);
	line(cameraFeed,Point(x,y),Point(x,y+25),Scalar(0,255,0),2);
	line(cameraFeed,Point(x,y),Point(x-25,y),Scalar(0,255,0),2);
	line(cameraFeed,Point(x,y),Point(x+25,y),Scalar(0,255,0),2);
	putText(cameraFeed,"Tracking object at (" + intToString(x)+","+intToString(y)+")",Point(x,y),1,1,Scalar(255,0,0),2);



}*/
int main(int ac, char **av){
	//ac--;
	//av++;

	//some boolean variables for added functionality
	bool objectDetected = false;
	//these two can be toggled by pressing 'd' or 't'
	bool debugMode = false;
	bool trackingEnabled = false;
	//pause and resume code
	bool pause = false;
	//set up the matrices that we will need
	//the two frames we will be comparing
	Mat frame1,frame2;
	//their grayscale images (needed for absdiff() function)
	// open capture
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
c.value = 30;
if(v4l2_ioctl(descriptor, VIDIOC_S_CTRL, &c) == 0)
    cout << "success";
	//video capture object.
	VideoCapture capture(0);
	//capture.set(CAP_PROP_BRIGHTNESS,0.1);
		if(!capture.isOpened()){
			cout<<"ERROR ACQUIRING VIDEO FEED\n";
			getchar();
			return -1;
		}

	

	while(1){

		//we can loop the video by re-opening the capture every time the video reaches its last frame
		//capture.open(*av);

		//check if the video has reach its last frame.
		//we add '-1' because we are reading two frames from the video at a time.
		//if this is not included, we get a memory error!
		

			//read first frame
			//capture.read(frame1); 
			capture >> frame1;
			/*cvSetImageROI(frame1, cvRect(0, 240, 640, 240));
			Mat frame = cvCreateImage(cvGetSize(frame1), frame1.depth, frame1.channels);
			cvCopy(frame1, frame, NULL);
			cvResetImageROI(frame1);*/
			Rect cropWindow(0, 240, 640, 240);
			Mat frame;
			frame1(cropWindow).copyTo(frame);
			imshow("frame", frame);
			bucket b(frame,low,high);
			//b.showContours();
			b.blobDetect();
			if(debugMode==true){
				//show the difference image and threshold image
				//imshow("Dif", differenceImage);
				//imshow("Thresh", thresholdImage);
				//b.blobDetect();
				
			}else{
				//if not in debug mode, destroy the windows so we don't see them anymore
				//destroyWindow("Dif");
				//destroyWindow("Thresh");
			}
			//use blur() to smooth the image, remove possible noise and
			//increase the size of the object we are trying to track. (Much like dilate and erode)

			//threshold again to obtain binary image from blur output

			if(debugMode==true){
				//show the threshold image after it's been "blurred"

			}
			else {
				//if not in debug mode, destroy the windows so we don't see them anymore

			}

			//if tracking enabled, search for contours in our thresholded image

			//show our captured frame
			imshow("Frame1",frame);
			//check to see if a button has been pressed.
			//this 10ms delay is necessary for proper operation of this program
			//if removed, frames will not have enough time to referesh and a blank 
			//image will appear.
			switch(waitKey(10)){

			case 27: //'esc' key has been pressed, exit program.
				return 0;
			case 116: //'t' has been pressed. this will toggle tracking
				trackingEnabled = !trackingEnabled;
				if(trackingEnabled == false) cout<<"Tracking disabled."<<endl;
				else cout<<"Tracking enabled."<<endl;
				break;
			case 100: //'d' has been pressed. this will debug mode
				debugMode = !debugMode;
				if(debugMode == false) cout<<"Debug mode disabled."<<endl;
				else cout<<"Debug mode enabled."<<endl;
				break;
			case 112: //'p' has been pressed. this will pause/resume the code.
				pause = !pause;
				if(pause == true){ cout<<"Code paused, press 'p' again to resume"<<endl;
				while (pause == true){
					//stay in this loop until 
					switch (waitKey()){
						//a switch statement inside a switch statement? Mind blown.
					case 112: 
						//change pause back to false
						pause = false;
						cout<<"Code resumed."<<endl;
						break;
					}
				}
				}


			}


		
		//release the capture before re-opening and looping again.
		
	}
	capture.release();
	return 0;

}
