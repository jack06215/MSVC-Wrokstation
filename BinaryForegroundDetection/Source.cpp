#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <windows.h>

#include <stdio.h>
#include <vector>

#define FRAME_WIDTH 640
#define FRAME_HEIGHT 480
#define THRESHOLD_VALUE 50
#define SCANNING_WINDOW_HEIGHT 10
#define SCANNING_WINDOW_WIDTH 10
#define CAMERA_SELECTED 0
#define BLOCK_WIDTH  (FRAME_WIDTH / SCANNING_WINDOW_WIDTH)
#define BLOCK_HEIGHT (FRAME_HEIGHT / SCANNING_WINDOW_HEIGHT)
#define FRAME_SIZE ((FRAME_WIDTH) * (FRAME_HEIGHT))

using namespace cv;
using namespace std;

typedef struct background
{
	int x0;
	int y0;
	int centerX;
	int centerY;
	int width;
	int height;
}background_t;

// Function prototpying
unsigned int fastAbs(int value);
float fastInvSqrt(float x);
void detecting_motion_blocks(char *srcImage, int width, int height, int boxHeight, int boxWidth, int threshold, vector<background_t> &result);
void grouping_motion_blocks(vector<background_t> &src, int displacement_threshold);
unsigned int calculate_linear_distance(background_t recA, background_t recB);
IplImage* get_video_each_N_frame(CvCapture* capture, int n);
void motion_detection_via_webcam(int selected_camera);
void motion_detection_via_video(char *filename);
void remove_dummy_blocks(vector<background_t> &src, int area_threshold);


// Global variables used in this motion dection example, but it is not good to use global variables in general practice!
IplImage *currentImg = cvCreateImage(cvSize(FRAME_WIDTH, FRAME_HEIGHT), IPL_DEPTH_8U, 1);
IplImage *prevImg = cvCreateImage(cvSize(FRAME_WIDTH, FRAME_HEIGHT), IPL_DEPTH_8U, 1);
IplImage *refImg = cvCreateImage(cvSize(FRAME_WIDTH, FRAME_HEIGHT), IPL_DEPTH_8U, 3);
IplImage *cropImg = NULL;
vector<background_t> movingObject;
char *currentImgData = (char*)currentImg->imageData;
char *prevImgData = (char*)prevImg->imageData;
char *binaryDifference = (char*)malloc(FRAME_SIZE * sizeof(char));
char *motionWindow = (char*)malloc(BLOCK_WIDTH * BLOCK_HEIGHT * sizeof(char));
char *cropImage;
void crop_and_save_image(char *src, char *dest, background_t rect);

int main()
{
	cvZero(currentImg);
	cvZero(prevImg);
	cvZero(refImg);
	//motion_detection_via_webcam(CAMERA_SELECTED);
	motion_detection_via_video("333.mpg");
	free(binaryDifference);
	return 0;
}

void crop_and_save_image(char *src, char *dest, background_t rect)
{
	int width = rect.width;
	int height = rect.height;
	int x0 = rect.x0;
	int y0 = rect.y0;
	int counter = 0;
	
	for (int i = y0; i < y0 + height; i++)
	{
		for (int j = x0; j < x0 + width; j++)
		{
			dest[counter++] = src[i * width + j];
			//counter++;
		}
	}
	cout <<  width << "\t" << height << "\t" << counter << endl;
}

void motion_detection_via_video(char *filename)
{
	CvCapture *video = cvCaptureFromFile(filename);
	IplImage *refImg2 = NULL;
	do
	{
		refImg2 = get_video_each_N_frame(video, 0);
		cvResize(refImg2, refImg);
		cvCvtColor(refImg, currentImg, CV_RGB2GRAY);
		for (int i = FRAME_SIZE; i--;)
			fastAbs(currentImgData[i] - prevImgData[i]) > 10 ? binaryDifference[i] = 1 : binaryDifference[i] = 0;

		detecting_motion_blocks(binaryDifference, FRAME_WIDTH, FRAME_HEIGHT, SCANNING_WINDOW_HEIGHT, SCANNING_WINDOW_WIDTH, THRESHOLD_VALUE, movingObject);
		if (movingObject.size() < 3)
			continue;

		grouping_motion_blocks(movingObject, 100);
		remove_dummy_blocks(movingObject, 120);
		cvCopyImage(currentImg, prevImg);
		
		for (int resultIndex = 0; resultIndex < movingObject.size(); resultIndex++)
		{
			cvRectangle(currentImg,
				cvPoint(movingObject.at(resultIndex).x0, movingObject.at(resultIndex).y0),
				cvPoint(movingObject.at(resultIndex).x0 + movingObject.at(resultIndex).width, movingObject.at(resultIndex).y0 + movingObject.at(resultIndex).height),
				CV_RGB(255, 0, 0),
				2,
				8);
		}

		for (int i = 0; i < movingObject.size(); i++)
		{
			
			int width = movingObject.at(i).width;
			int height = movingObject.at(i).height;
			cropImage = (char*)malloc(width * height * sizeof(char));
			//cropImg = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
			//char *cropImgData = cropImg->imageData;
			crop_and_save_image(currentImgData, cropImage, movingObject.at(i));
			
			//strncpy(cropImgData, cropImage, (width * height));
			//cvSaveImage("hi.jpg", cropImg);
			free(cropImage);
			//cvReleaseImage(&cropImg);
		}
		
		
		cvShowImage("Camera Image", currentImg);
		// Delay for 33ms
		waitKey(100);
		movingObject.clear();
		

	} while (cvQueryFrame(video) != NULL);
	cout << "End of video play" << endl;
	cvReleaseCapture(&video);
}

void motion_detection_via_webcam(int selected_camera)
{
	// Initialise capture
	CvCapture *webcam = cvCaptureFromCAM(selected_camera);
	cvSetCaptureProperty(webcam, CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	cvSetCaptureProperty(webcam, CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);
	IplImage *refImg2 = NULL;
	Sleep(1000);
	for (;;)
	{
		refImg2 = cvQueryFrame(webcam);
		cvResize(refImg2, refImg);
		cvCvtColor(refImg, currentImg, CV_RGB2GRAY);
		for (int i = FRAME_SIZE; i--;)
			fastAbs(currentImgData[i] - prevImgData[i]) > 25 ? binaryDifference[i] = 1 : binaryDifference[i] = 0;

		detecting_motion_blocks(binaryDifference, FRAME_WIDTH, FRAME_HEIGHT, SCANNING_WINDOW_HEIGHT, SCANNING_WINDOW_WIDTH, THRESHOLD_VALUE, movingObject);
		if (movingObject.size() < 2)
			continue;

		grouping_motion_blocks(movingObject, 100);
		remove_dummy_blocks(movingObject, 120);
		cvCopyImage(currentImg, prevImg);
		for (int resultIndex = 0; resultIndex < movingObject.size(); resultIndex++)
		{
			cvRectangle(refImg,
				cvPoint(movingObject.at(resultIndex).x0, movingObject.at(resultIndex).y0),
				cvPoint(movingObject.at(resultIndex).x0 + movingObject.at(resultIndex).width, movingObject.at(resultIndex).y0 + movingObject.at(resultIndex).height),
				CV_RGB(255, 0, 0),
				2,
				8);
		}
		cvShowImage("Camera Image", refImg);
		// Delay for 33ms
		waitKey(1);
		movingObject.clear();
	}
	cvReleaseCapture(&webcam);
}

template<typename Cont, typename It>
auto byIndices(Cont &cont, It beg, It end) -> decltype(std::end(cont))
{
	int helpIndx(0);
	return std::stable_partition(std::begin(cont), std::end(cont),
		[&](decltype(*std::begin(cont)) const& val) -> bool {
		return std::find(beg, end, helpIndx++) == end;
	});
}

void remove_dummy_blocks(vector<background_t> &src, int area_threshold)
{
	vector<int> position;
	for (int i = 0; i < src.size(); i++)
	{
		if (src.at(i).y0 < 130)
			position.push_back(i);
	}

	src.erase(byIndices(src, begin(position), end(position)), src.end());
	position.clear();
}

void grouping_motion_blocks(vector<background_t> &src, int displacement_threshold)
{
	background_t *max_width_local;
	background_t *max_height_local;
	vector<int> position;
	unsigned int distance;
	for (int i = 0; i < src.size() - 1; i++)
	{
		unsigned int max_width = 0;
		unsigned int max_height = 0;
		max_width_local = nullptr;
		max_height_local = nullptr;
		for (int j = i + 1; j < src.size(); j++)
		{
			distance = calculate_linear_distance(src.at(i), src.at(j));
			if (distance <= displacement_threshold)
				position.push_back(j);
		}
		for (int k = 0; k < position.size(); k++)
		{
			int maxHeight = fastAbs(src.at(i).centerX - src.at(position.at(k)).centerX);
			int maxWidth = fastAbs(src.at(i).centerY - src.at(position.at(k)).centerY);

			if (maxWidth > max_width)
			{
				max_width = maxWidth;
				max_width_local = &src.at(position.at(k));
			}
			if (maxHeight > max_height)
			{
				max_height = maxHeight;
				max_height_local = &src.at(position.at(k));
			}
		}
		if (max_width_local != nullptr && max_height_local != nullptr)
		{
			if (src.at(i).x0 > max_width_local->x0)
			{
				src.at(i).width = src.at(i).x0 - max_width_local->x0 + max_width_local->width;
				src.at(i).x0 = max_width_local->x0;
			}
			else
			{
				src.at(i).width = max_width_local->x0 - src.at(i).x0 + src.at(i).width;
			}
			src.at(i).height = max_width_local->y0 - src.at(i).y0 + max_width_local->height;
		}
		src.erase(byIndices(src, begin(position), end(position)), src.end());
		position.clear();
	}
}

unsigned int calculate_linear_distance(background_t recA, background_t recB)
{
	unsigned int distanceX, distanceY, norm, distance;
	distanceX = fastAbs(recB.centerX - recA.centerX);
	distanceY = fastAbs(recB.centerY - recA.centerY);
	norm = distanceX * distanceX + distanceY * distanceY;
	distance = (unsigned int)(norm * fastInvSqrt(norm));
	return distance;
}

void detecting_motion_blocks(char *srcImage, int width, int height, int boxHeight, int boxWidth, int threshold, vector<background_t> &result)
{
	int index = 0;
	int counter = 0;
	background_t rectangle;
	for (int i = 0; i < height; i += boxHeight)
	{
		for (int j = 0; j < width; j += boxWidth)
		{
			for (int x = 0; x < boxHeight; x++)
			{
				for (int y = 0; y < boxWidth; y++)
					counter += srcImage[(i + x) * width + (j + y)];	//counter += srcData[i + x][j + y];
			}
			if (counter >= threshold)
			{
				rectangle.y0 = i;
				rectangle.x0 = j;
				rectangle.centerX = (j + boxWidth + 1) >> 1;
				rectangle.centerY = (i + boxHeight + 1) >> 1;
				rectangle.width = boxWidth;
				rectangle.height = boxHeight;
				result.push_back(rectangle);
			}
			counter = 0;
		}
	}
}

IplImage* get_video_each_N_frame(CvCapture* capture, int n)
{
	for (int i = 0; i < n; i++)
	{
		if (cvQueryFrame(capture) == NULL)
			return NULL;
	}

	return cvQueryFrame(capture);
}

unsigned int fastAbs(int value)
{
	unsigned int result;
	int const mask = value >> sizeof(int)* CHAR_BIT - 1;
	result = (value + mask) ^ mask;
	return result;
}

float fastInvSqrt(float x)
{
	float xhalf = 0.5f * x;
	int i = *(int*)&x;            // store floating-point bits in integer
	i = 0x5f3759df - (i >> 1);    // initial guess for Newton's method
	x = *(float*)&i;              // convert new bits into float
	x = x*(1.5f - xhalf*x*x);     // One round of Newton's method
	return x;
}
