
/*
Copyright (C) 2014 Steven Hickson

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/
// TestVideoSegmentation.cpp : Defines the entry point for the console application.
//

#include "Microsoft_grabber2.h"

using namespace std;
using namespace cv;

// Safe release for interfaces
template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease)
{
	if (pInterfaceToRelease != NULL)
	{
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
	}
}

DWORD ProcessThread(LPVOID pParam) {
	KinectGrabber *p = (KinectGrabber*) pParam;
	p->ProcessThreadInternal();
	return 0;
}

template <typename T> inline T Clamp(T a, T minn, T maxx)
{ return (a < minn) ? minn : ( (a > maxx) ? maxx : a ); }

KinectGrabber::KinectGrabber(const int instance) {
	HRESULT hr;
	int num = 0;
	m_person = m_depthStarted = m_videoStarted = m_audioStarted = m_infraredStarted = false;
	hStopEvent = NULL;
	hKinectThread = NULL;

	hr = GetDefaultKinectSensor(&m_pKinectSensor);
	if (FAILED(hr)) {
		throw exception("Error could not get default kinect sensor");
	}

	if (m_pKinectSensor) {
		hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
		hr = m_pKinectSensor->Open();
		if (SUCCEEDED(hr)) {
			hr = m_pKinectSensor->OpenMultiSourceFrameReader(
				FrameSourceTypes::FrameSourceTypes_Depth | FrameSourceTypes::FrameSourceTypes_Color | FrameSourceTypes::FrameSourceTypes_BodyIndex,
				&m_pMultiSourceFrameReader);
			if (SUCCEEDED(hr))
			{
				m_videoStarted = m_depthStarted = true;
			} else
				throw exception("Failed to Open Kinect Multisource Stream");
		}
	}

	if (!m_pKinectSensor || FAILED(hr)) {
		throw exception("No ready Kinect found");
	}
	m_colorSize = Size(cColorWidth, cColorHeight);
	m_pColorRGBX = new RGBQUAD[cColorWidth * cColorHeight];
}

void KinectGrabber::start() {
	hStopEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	hKinectThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)&ProcessThread, this, 0, NULL );
}

void KinectGrabber::stop() {
	//stop the ProcessThread
	if(hStopEvent != NULL) {
		//signal the process to stop
		SetEvent(hStopEvent);
		if(hKinectThread != NULL) {
			WaitForSingleObject(hKinectThread,INFINITE);
			CloseHandle(hKinectThread);
			hKinectThread = NULL;
		}
		CloseHandle(hStopEvent);
		hStopEvent = NULL;
	}
	if (m_pColorRGBX) {
		delete [] m_pColorRGBX;
		m_pColorRGBX = NULL;
	}
}

bool KinectGrabber::isRunning () const {
	KinectStatus status;
	m_pKinectSensor->get_Status(&status);
	return (m_pKinectSensor && (status == KinectStatus_Connected || status == KinectStatus_InUseAsShared || status == KinectStatus_InUseAsExclusive));
}

KinectGrabber::~KinectGrabber() {
	Release();
}

bool KinectGrabber::GetCameraSettings() {
	return false;
}

void KinectGrabber::ProcessThreadInternal() {
	bool quit = false;
	int nEventIdx;
	while(!quit) {
		// Wait for any of the events to be signalled
		if(WaitForSingleObject(hStopEvent,1) == WAIT_OBJECT_0)
			quit = true;
		else {
			//Get the newest frame info
			GetNextFrame();
		}
	}
}

/*void KinectGrabber::StartVideoCapture() {
if (!m_videoStarted && m_pKinectSensor) {
HRESULT hr;
// Initialize the Kinect and get the color reader
IColorFrameSource* pColorFrameSource = NULL;
hr = m_pKinectSensor->get_ColorFrameSource(&pColorFrameSource);
if (SUCCEEDED(hr))
{
hr = pColorFrameSource->OpenReader(&m_pColorFrameReader);
m_videoStarted = true;
} else
throw exception("Failed to Open Kinect Image Stream");

SafeRelease(pColorFrameSource);
}
}

void KinectGrabber::StartDepthCapture() {
}

void KinectGrabber::StartInfraredCapture() {
}*/

void KinectGrabber::Release() {
	try {
		//clean up stuff here
		stop();
		if(m_pKinectSensor) {
			//Shutdown NUI and Close handles
			if (m_pMultiSourceFrameReader)
				SafeRelease(m_pMultiSourceFrameReader);
			if(m_pCoordinateMapper)
				SafeRelease(m_pCoordinateMapper);
			// close the Kinect Sensor
			if (m_pKinectSensor)
				m_pKinectSensor->Close();

			SafeRelease(m_pKinectSensor);
		}
	} catch(...) {
		//destructor never throws
	}
}

string KinectGrabber::getName () const {
	return std::string ("Kinect2Grabber");
}

float KinectGrabber::getFramesPerSecond () const {
	return 30.0f;
}


#pragma endregion

//Camera Functions
#pragma region Camera

/*void KinectGrabber::GetColor(Mat &color_image) {
	if (!m_pColorFrameReader)
	{
		throw exception("Color Frame reader not open!");
	}
	IColorFrame* pColorFrame = NULL;

	HRESULT hr = m_pColorFrameReader->AcquireLatestFrame(&pColorFrame);

	if (SUCCEEDED(hr))
	{
		INT64 nTime = 0;
		IFrameDescription* pFrameDescription = NULL;
		int nWidth = 0;
		int nHeight = 0;
		ColorImageFormat imageFormat = ColorImageFormat_None;
		UINT nBufferSize = 0;
		RGBQUAD *pBuffer = NULL;

		hr = pColorFrame->get_RelativeTime(&nTime);
		if (SUCCEEDED(hr)) {
			hr = pColorFrame->get_FrameDescription(&pFrameDescription);
		}
		if (SUCCEEDED(hr)) {
			hr = pFrameDescription->get_Width(&nWidth);
		}
		if (SUCCEEDED(hr)) {
			hr = pFrameDescription->get_Height(&nHeight);
		}
		if (SUCCEEDED(hr)) {
			hr = pColorFrame->get_RawColorImageFormat(&imageFormat);
		}
		if (SUCCEEDED(hr)) {
			if (imageFormat == ColorImageFormat_Bgra) {
				hr = pColorFrame->AccessRawUnderlyingBuffer(&nBufferSize, reinterpret_cast<BYTE**>(&pBuffer));
			}
			else if (m_pColorRGBX) {
				pBuffer = m_pColorRGBX;
				nBufferSize = cColorWidth * cColorHeight * sizeof(RGBQUAD);
				hr = pColorFrame->CopyConvertedFrameDataToArray(nBufferSize, reinterpret_cast<BYTE*>(pBuffer), ColorImageFormat_Bgra);            
			} else {
				hr = E_FAIL;
			}
		}

		if (SUCCEEDED(hr)) {
			color_image.release();
			color_image = Mat(m_colorSize, COLOR_PIXEL_TYPE, pBuffer, Mat::AUTO_STEP);
		}

		SafeRelease(pFrameDescription);
	}

	SafeRelease(pColorFrame);
}*/



#pragma endregion

//Depth Functions
#pragma region Depth

void KinectGrabber::GetDepth(Mat &depth_image) {
}

#pragma endregion

#pragma region Cloud

#pragma endregion
