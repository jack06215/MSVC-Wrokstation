#include <opencv2\opencv.hpp>
#include <time.h>
#include <iostream>
#include <cmath>
#include "lucaskanade.h"
#include "auxfunc.h"

void alignInverseComp(cv::Mat imgT, cv::Rect roi, cv::Mat imgI, cv::Mat *W)
{
	// Termination constraint 
	const float EPSILON = 10.0f;// 0.01;		// Difference error
	const int MAX_ITER = 100;	// Maximum iteration constraint

	// Here we create some intermediate images
	cv::Mat gradientX(imgT.size(), CV_32FC(1));	// Gradient of I in X-direction
	cv::Mat gradientY(imgT.size(), CV_32FC(1));	// Gradient of I in Y-direction
	cv::Mat stDesc(imgT.size(), CV_32FC(3));	// Steepest descent images of T
	// Create some matrices
	//cv::Mat *W = new cv::Mat(cv::Mat::zeros(3, 3, CV_32F));		// Current warp function W(x; p)
	cv::Mat dW(3, 3, CV_32F);			// Incremental warp update
	cv::Mat idW(3, 3, CV_32F);			// Inverse incremental warp update
	cv::Mat X(3, 1, CV_32F);			// Point in coordinate of T
	cv::Mat Z(3, 1, CV_32F);			// Point in coordinate of I

	cv::Mat H(3, 3, CV_32F);			// Hessian matrix
	cv::Mat iH(3, 3, CV_32F);			// Inverse Hessian		
	cv::Mat delta_p(3, 1, CV_32F);		// Parameter update
	cv::Mat b(3, 1, CV_32F);	
	/* Pre-computation stage of Template (T) */

	time_t start_time = clock();

	// Compute gradient of T
	cv::Sobel(imgT, gradientX, gradientX.type(), 1, 0);
	gradientX.convertTo(gradientX, gradientX.type(), 0.125);
	cv::Sobel(imgT, gradientY, gradientY.type(), 0, 1);
	gradientY.convertTo(gradientY, gradientY.type(), 0.125);

	// Compute steepest descent images and Hessain
	H = cv::Scalar(0);

	int u, v; // (u, v) - pixel coordinates in the coordinate of T
	int u2, v2; // (u2, v2) - pixel coordinate in the coordinate frame of I
	int i, j;
	for (i = 0; i < roi.width; i++)
	{
		u = i + roi.x;
		for (j = 0; j < roi.height; j++)
		{
			v = j + roi.y;
			// Evaluate gradient of T
			short Tx = (short)gradientX.at<float>(cv::Point(u, v));
			short Ty = (short)gradientY.at<float>(cv::Point(u, v));
			
			// Calcuate steepest descent image's element
			cv::Vec3f *stdesc = &stDesc.at<cv::Vec3f>(cv::Point(u, v));
			stdesc->val[0] = (-v*Tx + u*Ty);
			stdesc->val[1] = Tx;
			stdesc->val[2] = Ty;

			// Add a term to Hessian
			for (int m = 0; m < 3; m++)
			{
				for (int n = 0; n < 3; n++)
				{
					H.at<float>(m, n) += stdesc->val[m] * stdesc->val[n];
				}
			}
		}
	}
	
	// Invert Hessian.
	double inv_res = cv::invert(H, iH);
	cv::waitKey();
	if (inv_res == 0)
	{
		std::cout << "Error: Hessian is singular." << std::endl;
		return;
	}

	// We assume no warping is applied at initial guess, then eventually update its parameters depends on error
	cv::setIdentity(*W);

	float rms_error = 0;
	int iteration = 0;
	

	
	/* Iteration stage */
	
	while (iteration < MAX_ITER)
	{
		iteration++;
		rms_error = 0;
		int pixel_count = 0;
		b = cv::Scalar(0);

		for (int i = 0; i < roi.width; i++)
		{
			int u = i + roi.x;
			for (int j = 0; j < roi.height; j++)
			{
				int v = j + roi.y;

				// Set vector X with pixel coordinates (u,v,1)
				SET_VECTOR(X, u, v);

				// Warp Z=W*X
				cv::gemm(*W, X, 1, 0, 0, Z);

				// Get coordinates of warped pixel in coordinate frame of I.
				GET_VECTOR(Z, u2, v2);

				// Get the nearest integer pixel coords (u2i;v2i).
				int u2i = cvFloor(u2);
				int v2i = cvFloor(v2);

				if (u2i >= 0 && u2i < imgI.cols && // check if pixel is inside I.
					v2i >= 0 && v2i < imgI.rows)
				{
					pixel_count++;

					// Calculate intensity of a transformed pixel with sub-pixel accuracy
					// using bilinear interpolation.
					float I2 = interpolate<uchar>(&imgI, u2, v2);

					float D = I2 - imgT.at<uchar>(cv::Point(u, v));

					rms_error += fabs(D);
					//std::cout << rms_error << std::endl;
					cv::Vec3f* stdesc = &stDesc.at<cv::Vec3f>(cv::Point(u, v));
					cv::Vec3f* pb = &b.at<cv::Vec3f>(cv::Point(0, 0));
					
					pb->val[0] += stdesc->val[0] * D;		/*	b.at<float>(cv::Point(0, 0)) += stdesc->val[0] * D;
																b.at<float>(cv::Point(0, 1)) += stdesc->val[1] * D;
																b.at<float>(cv::Point(0, 2)) += stdesc->val[2] * D;	*/
					pb->val[1] += stdesc->val[1] * D;
					pb->val[2] += stdesc->val[2] * D;
				}
			}
		}
		
		if (pixel_count != 0)
			rms_error /= pixel_count;

		cv::gemm(iH, b, 1, 0, 0, delta_p);
		float delta_wz = delta_p.at<float>(cv::Point(0, 0));
		float delta_tx = delta_p.at<float>(cv::Point(0, 1));
		float delta_ty = delta_p.at<float>(cv::Point(0, 2));
	
		initWarp2(dW, delta_wz, delta_tx, delta_ty);
		
		inv_res = cv::invert(dW, idW);
		if (inv_res == 0)
		{
			printf("Error: Warp matrix is singular.\n");
			return;
		}
		
		cv::gemm(idW, *W, 1, 0, 0, dW);
		
		dW.copyTo(*W);
		//std::cout << rms_error << std::endl;
		//std::cout << delta_wz << " " << delta_tx << " " << delta_ty << std::endl;
		if (rms_error <= EPSILON) break;
		//if (fabs(delta_wz) <= EPSILON && fabs(delta_tx) <= EPSILON && fabs(delta_ty) <= EPSILON) break;
	}
	
	// pause the stopwatch
	time_t finish_time = clock();
	double total_time = (double)(finish_time - start_time) / CLOCKS_PER_SEC;

	std::cout << "alignInverseComp() exe time: " << total_time << " seconds\n" << std::endl;

	// Release all useed recources
	gradientX.release();
	gradientY.release();
	stDesc.release();
	dW.release();
	idW.release();
	X.release();
	Z.release();
	H.release();
	iH.release();
	H.release();
	iH.release();
	delta_p.release();
}