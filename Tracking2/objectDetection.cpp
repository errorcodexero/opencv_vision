#include "stdafx.h"
#include "objectDetection.h"



Mat objectDetection::colorFilter(Mat frame)
{
	return Mat();
}

bool objectDetection::contours(Mat frame)
{
	return true;
}

bool objectDetection::boundingRec()
{
	return true;
}

bool objectDetection::paintTarget(Scalar pos)
{
	return true;
}

Scalar objectDetection::detect(Mat frame)
{
	return Scalar(0,0);
}
