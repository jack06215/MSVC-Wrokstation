#ifndef __AUXFUNC_H__
#define __AUXFUNC_H__

#define SET_VECTOR(X, u, v)\
	X.at<float>(0, 0) = (float)(u);\
	X.at<float>(1, 0) = (float)(v);\
	X.at<float>(2, 0) = 1.0f;

#define GET_VECTOR(X, u, v)\
	(u) = X.at<float>(0, 0);\
	(v) = X.at<float>(1, 0);

#define GET_INT_VECTOR(X, u, v)\
	(u) = (int)X.at<float>(0, 0);\
	(v) = (int)X.at<float>(1, 0);

template <class T>
float interpolate(cv::Mat* pImage, float x, float y)
{
	// Get the nearest integer pixel coords (xi;yi).
	int xi = cvFloor(x);
	int yi = cvFloor(y);

	float k1 = x - xi; // Coefficients for interpolation formula.
	float k2 = y - yi;

	int f1 = xi < pImage->cols - 1;  // Check that pixels to the right  
	int f2 = yi < pImage->rows - 1; // and to down direction exist.

	T* row1 = &pImage->at<T>(cv::Point(xi, yi));
	T* row2 = &pImage->at<T>(cv::Point(xi, yi + 1));

	// Interpolate pixel intensity.
	float interpolated_value = (1.0f - k1)*(1.0f - k2)*(float)row1[0] +
		(f1 ? (k1 * (1.0f - k2) * (float)row1[1]) : 0) +
		(f2 ? ((1.0f - k1) * k2 * (float)row2[0]) : 0) +
		((f1 && f2) ? (k1 * k2 * (float)row2[1]) : 0);

	return interpolated_value;
}

void drawWarppedRect(cv::Mat &img, cv::Rect rect, cv::Mat W);
void initWarp(cv::Mat& warpMat, float *warp);
void initWarp2(cv::Mat& W, float wz, float tx, float ty);
void overlayImage(const cv::Mat &background, const cv::Mat &foreground, cv::Mat &output, cv::Point2i location);


#endif