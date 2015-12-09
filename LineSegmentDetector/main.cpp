#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <windows.h>
#include "auxfunc.h"


int main(int argc, char** argv)
{
	cv::VideoCapture cap;
	cv::Mat image;
	if (!cap.open(0))
	{
		std::cout << "cannot open camera" << std::endl;
		return 1;
	}
	Sleep(1000);
	for(;;)
	{
		cap.read(image);
		cv::cvtColor(image, image, cv::COLOR_RGB2GRAY);
#if 0
		cv::Canny(image, image, 50, 200, 3); // Apply canny edge
#endif

		// Create and LSD detector with standard refinement.
		cv::Ptr<cv::LineSegmentDetector> ls = cv::createLineSegmentDetector(cv::LSD_REFINE_STD);
		double start = double(cv::getTickCount());	//Start timer

		std::vector<cv::Vec4f> lines_std;
		// Detect the lines
		ls->detect(image, lines_std);
		double duration_ms = (double(cv::getTickCount()) - start) * 1000 / cv::getTickFrequency(); // Show execution time
		//std::cout << "It took " << duration_ms << " ms." << std::endl;
		std::cout << lines_std.size() << std::endl;
		// Show found lines
		cv::Mat drawnLines(image);
		ls->drawSegments(drawnLines, lines_std);
		cv::imshow("Standard refinement", drawnLines);
		cv::waitKey(1);
	}
	return 0;
}
