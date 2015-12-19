#include <opencv2\opencv.hpp>
#include <iostream>
#include "auxfunc.h"



void drawWarppedRect(cv::Mat &img, cv::Rect rect, cv::Mat W)
{
	cv::Point lb, lt, rb, rt;
	cv::Mat X(3, 1, CV_32FC1);
	cv::Mat Z(3, 1, CV_32FC1);

	// left-top point
	SET_VECTOR(X, rect.x, rect.y);
	cv::gemm(W, X, 1, 0, 0, Z);
	GET_INT_VECTOR(Z, lt.x, lt.y);

	// left-bottom point
	SET_VECTOR(X, rect.x, rect.y + rect.height);
	cv::gemm(W, X, 1, 0, 0, Z);
	GET_INT_VECTOR(Z, lb.x, lb.y);

	// right-top point
	SET_VECTOR(X, rect.x + rect.width, rect.y);
	cv::gemm(W, X, 1, 0, 0, Z);
	GET_INT_VECTOR(Z, rt.x, rt.y);

	// right-bottom point
	SET_VECTOR(X, rect.x + rect.width, rect.y + rect.height);
	cv::gemm(W, X, 1, 0, 0, Z);
	GET_INT_VECTOR(Z, rb.x, rb.y);

	// draw rectangle
	cv::line(img, lt, rt, CV_RGB(255, 0, 0), 3);
	cv::line(img, rt, rb, CV_RGB(255, 0, 0), 3);
	cv::line(img, rb, lb, CV_RGB(255, 0, 0), 3);
	cv::line(img, lb, lt, CV_RGB(255, 0, 0), 3);

	// release resources and exit
	X.release();
	Z.release();

}

void initWarp(cv::Mat& warpMat, float *warp)
{
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			warpMat.at<float>(i, j) = warp[i * warpMat.rows + j];	
}

//
//  ! 1  -wz  tx !
//  ! wz  1   ty !
//  ! 0   0   1  !
//

void initWarp2(cv::Mat& W, float wz, float tx, float ty)
{
	W.at<float>(0, 0) = 1;
	W.at<float>(1, 0) = wz;
	W.at<float>(2, 0) = 0;

	W.at<float>(0, 1) = -wz;
	W.at<float>(1, 1) = 1;
	W.at<float>(2, 1) = 0;

	W.at<float>(0, 2) = tx;
	W.at<float>(1, 2) = ty;
	W.at<float>(2, 2) = 1;
}

void overlayImage(const cv::Mat &background, const cv::Mat &foreground, cv::Mat &output, cv::Point2i location)
{
	background.copyTo(output);
	// start at the row indicated by location, or at row 0 if location.y is negative.
	for (int y = std::max(location.y, 0); y < background.rows; ++y)
	{
		int fY = y - location.y; // because of the translation

		// we are done of we have processed all rows of the foreground image.
		if (fY >= foreground.rows) break;

		// start at the column indicated by location, 
		// or at column 0 if location.x is negative.
		for (int x = std::max(location.x, 0); x < background.cols; ++x)
		{
			int fX = x - location.x; // because of the translation.

			// we are done with this row if the column is outside of the foreground image.
			if (fX >= foreground.cols)
				break;

			// determine the opacity of the foregrond pixel, using its fourth (alpha) channel.
			double opacity = 1;
			// For RGBA, ((double)foreground.data[fY * foreground.step + fX * foreground.channels() + 3]) / 255.;
			
			// and now combine the background and foreground pixel, using the opacity, 
			// but only if opacity > 0.
			for (int c = 0; opacity > 0 && c < output.channels(); ++c)
			{
				unsigned char foregroundPx = foreground.data[fY * foreground.step + fX * foreground.channels() + c];
				unsigned char backgroundPx = background.data[y * background.step + x * background.channels() + c];
				output.data[y * output.step + output.channels() * x + c] = backgroundPx * (1. - opacity) + foregroundPx * opacity;
			}
		}
	}
}