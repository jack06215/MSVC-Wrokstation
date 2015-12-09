#include <iostream>
#include <opencv2\opencv.hpp>
#include <opencv2\calib3d\calib3d.hpp>
#include <windows.h>
#include "auxfunc.h"
#include "lucaskanade.h"

// Name of display window.
char* sourceWindow = "img I";
char* outputWindowA = "img T";

// Our initial guess of warpping matrix.
float Tx = 5.0f;
float Ty = -3.0f;
float thetaR = 0.04f;



int main(int argc, char* argv[])
{
	// Read an input image.
	cv::Mat src = cv::imread(argv[1]);	// Original copy of input image
	if (src.empty())
	{
		std::cerr << "Could not open or find the image !" << std::endl;
		cv::waitKey();
		return -1;
	}

	// Read an image to be augmented
	cv::Mat imgA = cv::imread(argv[2]);
	if (imgA.empty())
	{
		std::cerr << "Could not open or find the image !" << std::endl;
		cv::waitKey();
		return -1;
	}
	
	// One color and grayscale copy of the input image
	cv::Mat src_copy;	// Color copy
	cv::Mat src_gray;	// Grayscale copy.

	// Create image for augmentation
	cv::Mat img_overlay = cv::imread("fig1.jpg");
	cv::Mat src_overlay = src_copy.clone();

	// Here we crete some images buffer
	cv::Mat img_t;						// The template, which is T.
	cv::Mat img_i;						// The image, which is I.
	cv::Mat warp_mat(3, 3, CV_32F);	// Warp matrix.
	cv::Mat warp_eatimated = cv::Mat::zeros(3, 3, CV_32F);
	cv::setIdentity(warp_eatimated);
	src.copyTo(src_copy);
	
	// Convert input image to grayscale because we need the intensity value.
	cv::cvtColor(src, src_gray, CV_RGB2GRAY);
	img_i = src_gray.clone();
	img_t = src_gray.clone();

	// Set ROI to become a template image.
	cv::Rect roi(50, 50, 100, 150);
	
	// Now we warp the input image
	initWarp2(warp_mat, thetaR, Tx, Ty);
	cv::warpPerspective(src_gray, img_i, warp_mat, src.size());
	
	// Let's start estimte the warp function
	alignInverseComp(img_t, roi, img_i, &warp_eatimated);

	// Print out the result and draw the region of tracking
	std::cout << warp_eatimated << std::endl;
	drawWarppedRect(src_copy, roi, warp_eatimated);
	cv::imshow("image", src_copy);

	// Shows the template image
	src.copyTo(src_copy);
	img_t = src_copy(roi);
	cv::imshow("template", img_t);

	// Let's insert and warp another image to this new region!, first we resize the object image first
	cv::resize(img_overlay, img_overlay, cv::Size(roi.width, roi.height), 0, 0, CV_INTER_CUBIC);
	
	// insert it onto our src image 
	overlayImage(src_copy, img_overlay, src_overlay, cv::Point(roi.x, roi.y));
	
	// Warp the final image with our estimated warp function we just obtained from alignedInverseComp()
	cv::warpPerspective(src_overlay, src_overlay, warp_eatimated, src_overlay.size());
	cv::imshow("imgA", src_overlay);

	// To make imshow() works properly
	cv::waitKey();

	// Release all the resources.
	cv::destroyAllWindows();
	warp_mat.release();
	img_t.release();
	img_i.release();
	src_gray.release();
	src.release();
	return 0;
}