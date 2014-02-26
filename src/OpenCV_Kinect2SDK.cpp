#include "OpenCV_KinectSDK.h"

using namespace std;
using namespace cv;

DWORD ProcessThread1(LPVOID pParam) {
	SaveKinectData *p = (SaveKinectData*) pParam;
	p->StartCapturingInternal();
	return 0;
}

DWORD ProcessThread2(LPVOID pParam) {
	SaveKinectData *p = (SaveKinectData*) pParam;
	p->StartSavingInternal();
	return 0;
}

void SaveKinectData::StartCapturingInternal() {
	if(!kinect_init) {
		kc = new KinectGrabber();
		kinect_init = true;
		kc->start();
	}
	Sleep(30);
	while(!quit) {
		Mat *img = new Mat();
		Mat *depth = new Mat();
		kc->GetDepth(*depth);
		kc->GetColor(*img);
		if(!img->empty())
			imgs.push_back(img);
		if(!depth->empty())
			depths.push_back(depth);
		SleepEx(25,false);
	}
}

void SaveKinectData::StartSavingInternal() {
	hDoneSaving = CreateEvent( NULL, FALSE, FALSE, NULL );
	while(!quit || !imgs.empty() || !depths.empty()) {
		if(!imgs.empty() && !depths.empty()) {
			Mat* img = imgs.front();
			Mat* depth = depths.front();
			imgs.pop_front();
			depths.pop_front();
			stringstream num;
			num << count;
			imwrite((direc + "color_" + num.str() + ".png").c_str(),*img);
			imwrite((direc + "depth_" + num.str() + ".png").c_str(),*depth);
			img->release();
			delete img;
			depth->release();
			delete depth;
			++count;
		}
	}
	SetEvent(hDoneSaving);
}

void SaveKinectData::Start() {
	quit = false;
	imgs.clear();
	depths.clear();
	count = 0;
	hCaptureThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)&ProcessThread1, this, 0, NULL );
	hSaveThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)&ProcessThread2, this, 0, NULL );
}

void SaveKinectData::Stop() {
	//doesn't work right now, really need to fix that
	quit = true;
	kc->stop();
	WaitForSingleObject(hDoneSaving,INFINITE);
	CloseHandle(hDoneSaving);
	hDoneSaving = NULL;
}

void DisplayImages() {
	try {
		Mat image, depth, depth_colored;
		KinectGrabber grabber;
		grabber.start();
		while(1) {
			//grabber.GetNextFrame();
			grabber.GetDepth(depth);
			grabber.GetColor(image);
			if(!image.empty())
				imshow("image",image);
			else
				cout << "empty image" << endl;
			if(!depth.empty()) {
				//applyColorMap(depth,depth_colored,COLORMAP_JET);
				imshow("depth",depth);
			} else
				cout << "empty depth" << endl;
			cvWaitKey(1);
		}
	} catch (std::exception &e) {
		cout << e.what() << endl;
	}
	cin.get();
}

void SaveImages(string out) {
	SaveKinectData save(out);
	save.Start();
	cout << "starting save" << endl;
	cin.get();
	cout << "Got end signal!" << endl;
	save.Stop();
}

int main (int argc, char** argv) {
	SaveImages(argv[1]);
	//DisplayImages();
	return 0;
}