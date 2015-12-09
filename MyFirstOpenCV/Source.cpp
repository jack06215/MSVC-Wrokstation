#include <opencv2/opencv.hpp>
#include <windows.h>
#include <iostream>

using namespace cv;
using namespace std;


void initWebcam(VideoCapture &videoCapture, int cameraNumber)
{
	// Get access to the default camera.
	try 
	{   // Surround the OpenCV call by a try/catch block so we can give a useful error message!
		videoCapture.open(cameraNumber);
	}
	catch (cv::Exception &e) {}
	if (!videoCapture.isOpened()) 
	{
		cerr << "ERROR: Could not access the camera!" << endl;
		exit(1);
	}
	cout << "Loaded camera " << cameraNumber << "." << endl;
}

int main()
{
	// Create matrix to store image
	Mat image_out;
	// initialise capture
	VideoCapture cap;
	
	if (!cap.open(0)) //http://admin:@192.168.0.114:80/mjpeg.cgi?resolution=640x480&req_fps=30&.mjpg
	{
		std::cout << "Error opening video stream or file" << std::endl;
		return -1;
	}
	//initWebcam(cap, 0);
	Sleep(1000);
	while (1)
	{
		// copy stream to image
		cap.read(image_out);
		// print image to screen
		imshow("window", image_out);
		waitKey(33);
	}
	return 0;
}