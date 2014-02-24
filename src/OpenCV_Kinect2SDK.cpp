#include <iostream>
#include "Microsoft_grabber2.h"

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

int
	main (int argc, char** argv)
{
	try {
		Mat image;
		KinectGrabber grabber;
		grabber.start();
		while(1) {
			grabber.GetColor(image);
			if(!image.empty())
				imshow("image",image);
			cvWaitKey(1);
		}
	} catch (std::exception &e) {
		cout << e.what() << endl;
	}
	cin.get();
	return (0);
}