#include <iostream>
#include "Microsoft_grabber2.h"

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <deque>

class SaveKinectData {
private:
	std::deque<cv::Mat*> imgs;
	std::deque<cv::Mat*> depths;
	HANDLE hCaptureThread, hSaveThread, hDoneSaving;
	KinectGrabber *kc;
	bool kinect_init;
	bool quit;
	int count;
	std::string direc;

public:
	SaveKinectData(std::string _directory) : quit(false), kinect_init(false), count(0), direc(_directory) { }
	SaveKinectData(KinectGrabber *_kc, std::string _directory) : quit(false), kinect_init(true), kc(_kc), count(0), direc(_directory) { };
	void StartSavingInternal();
	void StartCapturingInternal();
	void Start();
	void Stop();
};