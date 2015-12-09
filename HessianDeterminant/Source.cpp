#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/features2d/features2d.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

cv::Mat image;
int thresh = 200;
int max_thresh = 400;

int gauKsize = 11;
int maxGauKsize = 21;

char* source_window = "Source image";

void filterHessian( int, void* );

int main( int argc, char** argv )
{
  if( argc != 3 ) {
    std::cerr << "Usage: " << argv[0] << " <InputImage> <method>" << std::endl;
	  return EXIT_FAILURE;
  }

  image = cv::imread( argv[1], 1 );
  if(!image.data) {
    return EXIT_FAILURE;
  }
  cv::resize(image, image, cv::Size(0, 0), 0.75, 0.75, 1);
  //cv::cvtColor(image, image, CV_RGB2GRAY);
  cv::namedWindow( source_window, CV_WINDOW_AUTOSIZE );
  cv::createTrackbar( "Threshold: ", source_window, &thresh, max_thresh, filterHessian );
  cv::createTrackbar( "Kernel: ", source_window, &gauKsize, maxGauKsize, filterHessian );
  cv::imshow( source_window, image );
  filterHessian(0, 0);
  cv::waitKey(0);
  return EXIT_SUCCESS;
}

void filterHessian( int, void* )
{
	cv::Mat dXX, dYY, dXY;

	cv::Sobel(image, dXX, CV_32F, 2, 0);
	cv::Sobel(image, dYY, CV_32F, 0, 2);
	cv::Sobel(image, dXY, CV_32F, 1, 1);
  
	cv::Mat gau = cv::getGaussianKernel(gauKsize, -1, CV_32F);
  
	cv::sepFilter2D(dXX, dXX, CV_32F, gau.t(), gau);
	cv::sepFilter2D(dYY, dYY, CV_32F, gau.t(), gau);
	cv::sepFilter2D(dXY, dXY, CV_32F, gau.t(), gau);
  
	cv::Mat dst = image.clone();
  
	std::cout << thresh << " : " << gauKsize << std::endl;

	cv::Mat detH = dXX.mul(dYY) - (dXY.mul(dXY));

	for( int j = 0; j < detH.rows ; j++ ) { 
		for( int i = 0; i < detH.cols; i++ ) {
			//float pixelHessianResponse = detH.at<float>(j, i);
			cv::Vec3f pixelHessianResponse = detH.at<cv::Vec3f>(j, i);
			//if ((pixelHessianResponse >= (float)thresh)) {
			if( (pixelHessianResponse[0] + pixelHessianResponse[1] + pixelHessianResponse[2]) / 3 >= (float)thresh ) {
				circle( dst, cv::Point( i, j ), 1, cv::Scalar(0, 0, 255), 2, 8, 0 );
			}
		}
	}
	cv::namedWindow(source_window, CV_WINDOW_NORMAL);
	cv::imshow( source_window, dst );
}