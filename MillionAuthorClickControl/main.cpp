#include "AutoItX3_DLL.h"
#include <opencv\highgui.h>
#include <opencv2\imgproc\imgproc.hpp>
#include <iostream>
#include <windows.h>
#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>
#include <string>
#include <sstream>
#include <locale>
#include <stdio.h>
#include <fstream>


using namespace std;
using namespace cv;
using namespace tesseract;

string path = "D:\\";

void MA_move_stage_scrollbar_down_by(RECT windowsMA, int num);
void find_threshold_value(const char* filename);
void MA_ocrJapanese(const char *filename);
void MA_imageFilter(const char* filename, double contrast, int brightness);
void MA_cropCrystalPoint(string filename, Mat &src, Rect ROI);
void MA_cropAndSave(string filename, Mat &src, Rect ROI);
int  MA_ocrProcess(const char *filename);
Mat cvCaptureActiveWindow();
void MA_touchScreen_click(int x, int y, string message, int delay = 1000);
void MA_battle_singlePlayer();
wchar_t *char2LPCWSTR(const char* charArray);

int main(void)
{
	
	AU3_Init();
	int result = AU3_WinExists(char2LPCWSTR("BlueStacks App Player"), char2LPCWSTR(""));
	AU3_WinActivate(char2LPCWSTR("BlueStacks App Player"), char2LPCWSTR(""));
	
	//for (;;)
	//{ 
		
		Mat screen = cvCaptureActiveWindow();
		RECT windowsMA;
		int array = AU3_WinGetPos(char2LPCWSTR("BlueStacks App Player"), char2LPCWSTR(""), &windowsMA); // Win frame parameter: x = +3 y = +26

		/* Scroll down to the next stage */
		//array = AU3_MouseClickDrag(char2LPCWSTR("left"), windowsMA.left + 800, windowsMA.top + 150, windowsMA.left + 800, windowsMA.top + 180, 10);
		MA_move_stage_scrollbar_down_by(windowsMA, 5);

		/* Determine stage type 
		Rect stageType_ROI(405, 136, 41, 30);
		MA_cropAndSave("StageType.jpg", screen, stageType_ROI);
		find_threshold_value("StageType.jpg");*/

		


		/* Update current Crystal Point */
		////MA_cropCrystalPoint(screen);
		//MA_imageFilter("CrystalPoint.jpg", 0.5, -60);
		//int crystal = MA_ocrProcess("CrystalPoint_OCR.jpg");
		
		//SetConsoleOutputCP(CP_UTF8); // output is in UTF8
		//MA_imageFilter("QuestName.jpg", 2.0, -200);
		//MA_ocrJapanese("QuestName_OCR.jpg");
		//cout << "PressKey" << endl;
		//getchar();
		//waitKey(3000);
		//getchar();
	//}
	return 0;
}

void MA_move_stage_scrollbar_down_by(RECT windowsMA, int num)
{
	int ret = AU3_MouseClickDrag(char2LPCWSTR("left"), windowsMA.left + 800, windowsMA.top + 150, windowsMA.left + 800, windowsMA.top + (num * 25 + 150), 10);
}

void MA_cropCrystalPoint(string filename, Mat &src, Rect ROI)
{
	Mat croppedImage;
	//string filename = "CrystalPoint.jpg";
	Rect myROI(532, 6, 45, 17);
	Mat(src, myROI).copyTo(croppedImage);
	//return croppedImage;
	imwrite(path + filename, croppedImage);
}

void MA_cropAndSave(string filename, Mat &src, Rect ROI)
{
	Mat croppedImage;
	//string filename = "StageType.jpg";
	//Rect myROI(405, 136, 41, 30);
	Mat(src, ROI).copyTo(croppedImage);
	//return croppedImage;
	imwrite(path + filename, croppedImage);
}

void find_threshold_value(const char* filename)
{
	Mat image = imread(path + filename);
	int threshold(0);
	int image_size = image.cols * image.rows;
	for (int y = 0; y < image.rows; y++)
	{
		for (int x = 0; x < image.cols; x++)
		{
			for (int c = 0; c < 3; c++)
			{
				threshold += image.at<Vec3b>(y, x)[c];
			}
		}
	}
	threshold /= image_size;
	cout << threshold << endl;
}

void MA_imageFilter(const char* filename, double contrast, int brightness)
{
	/* Step 1: Adjust Brightness and Contrast */
	Mat image = imread(filename);
	Mat new_image = Mat::zeros(image.size(), image.type());
	// Do the operation image(i,j) = alpha*image(i,j) + beta
	for (int y = 0; y < image.rows; y++)
	{
		for (int x = 0; x < image.cols; x++)
		{
			for (int c = 0; c < 3; c++)
			{
				new_image.at<Vec3b>(y, x)[c] =
					saturate_cast<uchar>(contrast*(image.at<Vec3b>(y, x)[c]) + brightness);
			}
		}
	}
	cvtColor(new_image, new_image, CV_RGB2GRAY);
	//bitwise_not(new_image, new_image);
	resize(new_image, new_image, cvSize(0, 0), 2.0, 2.0);
	/*
	namedWindow("Original Image", 1);
	namedWindow("New Image", 1);
	imshow("Original Image", image);
	imshow("New Image", new_image);
	*/
	//imwrite("CrystalPoint_OCR.jpg", new_image);
	imwrite("QuestName_OCR.jpg", new_image);
}

void MA_ocrJapanese(const char *filename)
{
	// Initialise and configure Tesseract OCR
	TessBaseAPI ocrEngine;
	ocrEngine.SetPageSegMode(PSM_SINGLE_BLOCK);
	ocrEngine.Init("", "jpn", OEM_DEFAULT);
	PIX *tessImg = pixRead(filename);
	STRING text_out;
	ocrEngine.ProcessPages(filename, NULL, 0, &text_out);
	//stringstream ss(text_out.string());
	string result = text_out.string();
	string filename_out = "out.txt";
	//result.erase(remove_if(result.begin(), result.end(), 20), result.end());
	ofstream ofs(path + filename_out);
	ofs << text_out.string() << std::endl;
}

int MA_ocrProcess(const char *filename)
{
	// Initialise and configure Tesseract OCR
	TessBaseAPI ocrEngine;
	ocrEngine.SetPageSegMode(PSM_SINGLE_BLOCK);
	ocrEngine.Init("./tessdata", "eng", OEM_DEFAULT);
	PIX *tessImg = pixRead(filename);
	STRING text_out;
	ocrEngine.ProcessPages(filename, NULL, 0, &text_out);
	stringstream ss(text_out.string());
	int crystal;
	ss >> crystal;
	return crystal;
}

Mat cvCaptureActiveWindow()
{
	RECT rc;
	HWND hwnd = FindWindow(NULL, TEXT("BlueStacks App Player"));
	if (hwnd == NULL)
	{
		cout << "it can't find any 'note' window" << endl;
		exit;
	}

	HDC hwindowDC, hwindowCompatibleDC;

	int height, width, srcheight, srcwidth;
	HBITMAP hbwindow;
	Mat src;
	BITMAPINFOHEADER  bi;

	hwindowDC = GetDC(hwnd);
	hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
	SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

	RECT windowsize;    // get the height and width of the screen
	GetClientRect(hwnd, &windowsize);

	srcheight = windowsize.bottom;
	srcwidth = windowsize.right;
	height = windowsize.bottom;  //change this to whatever size you want to resize to
	width = windowsize.right;

	src.create(height, width, CV_8UC4);

	// create a bitmap
	hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
	bi.biSize = sizeof(BITMAPINFOHEADER); 
	bi.biWidth = width;
	bi.biHeight = -height;  //this is the line that makes it draw upside down or not
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	// use the previously created device context with the bitmap
	SelectObject(hwindowCompatibleDC, hbwindow);
	// copy from the window device context to the bitmap device context
	StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, 0, 0, srcwidth, srcheight, SRCCOPY); //change SRCCOPY to NOTSRCCOPY for wacky colors !
	GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, (BITMAPINFO *)&bi, DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow

	// avoid memory leak
	DeleteObject(hbwindow); 
	DeleteDC(hwindowCompatibleDC); 
	ReleaseDC(hwnd, hwindowDC);

	return src;

	//imwrite("screen.jpg", src);
}

void MA_touchScreen_click(int x, int y, string message, int delay)
{
	AU3_ControlClick(char2LPCWSTR("BlueStacks App Player"), char2LPCWSTR(""), char2LPCWSTR(""), char2LPCWSTR("LEFT"), 1, x, y);
	AU3_Sleep(delay);
}

void MA_battle_singlePlayer()
{
	MA_touchScreen_click(208, 512, "Buttom_Battle");
	MA_touchScreen_click(665, 204, "Buttom_Request");
	MA_touchScreen_click(665, 204, "Buttom_SinglePlayer");
}


wchar_t *char2LPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}