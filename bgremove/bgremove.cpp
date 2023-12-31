// bgremove.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "bgremove.h"
#include <stdlib.h>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <vector>



#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/opencv.hpp>
#include<opencv2/core.hpp>



//C
#include <stdio.h>
//C++
//#include <iostream>
#include <sstream>

#include "base64.h"


using namespace std;
using namespace cv; 


Mat bgRemove(Mat imgSource,  int iters, int sample);

Mat bgRemove(Mat imgSource,  int iters, int sample)
{

	if (imgSource.type() != CV_8UC3)
	{
		cvtColor(imgSource, imgSource, COLOR_BGRA2BGR);
	}
	
	Mat downsampled;

	if (sample > 0)
	{
		pyrDown(imgSource, downsampled, cv::Size(imgSource.cols / 2, imgSource.rows / 2));
	}
	else
		downsampled = imgSource; 
	
	
	

	//Point p1 = Point(top, left);
	//Point p2 = Point(height, width);
	//Rect rect = Rect(p1, p2);

		int r = downsampled.rows;
		int c = downsampled.cols;
		Point p1 = Point(c / 100, r / 100);
		Point p2 = Point(c - c / 100, r - r / 100);
		Rect rect = Rect(p1, p2);

	Mat mask = Mat();
	Mat fgdModel = Mat();
	Mat bgdModel = Mat();	  

	

	grabCut(downsampled, mask, rect, bgdModel, fgdModel, iters, GC_INIT_WITH_RECT);


	Mat resultUp;

	if (sample > 0)
	{
		pyrUp(mask, resultUp, cv::Size(mask.cols * 2, mask.rows * 2));
	}
	else
		resultUp = mask;
	
	
	


	Mat source = Mat(1, 1, CV_8U, Scalar(3.0));
	compare(resultUp, source/* GC_PR_FGD */, resultUp, CMP_EQ);

	//This is important. You must use Scalar(255,255, 255,255), not Scalar(255,255,255)
	Mat foreground = Mat(resultUp.size(), CV_8UC3, Scalar(255,255, 255, 255));

	imgSource.copyTo(foreground, resultUp);

	return foreground;
}



//Mat bgRemove(Mat imgSource, int top, int left, int width, int height, int iters, int downsample);
//
//Mat bgRemove(Mat imgSource, int top, int left, int width, int height, int iters, int downsample)
//{
//		
//
//	
//	Mat mask = Mat();
//	Mat fgdModel = Mat();
//	Mat bgdModel = Mat();
//
//
//
//	if (imgSource.type() != CV_8UC3)
//	{
//		cvtColor(imgSource, imgSource, COLOR_BGRA2BGR);
//	}
//
//
//	Mat downsampled;
//	if (downsample == 2)
//	{
//		pyrDown(imgSource, downsampled, cv::Size(imgSource.cols / downsample, imgSource.rows / downsample));
//	}
//	else
//		downsampled = imgSource; 
//
//	//Point p1 = Point(top/ downsample, left/ downsample);
//	//Point p2 = Point(height/ downsample, width/ downsample);
//	//Rect rect = Rect(p1, p2);
//
//	Rect rect = Rect(1, 1, downsampled.rows - 2, downsampled.cols - 2);
//
//	int x = downsampled.rows;
//	int y = downsampled.cols;
//
//	/*int r = downsampled.rows;
//	int c = downsampled.cols;
//	Point p1 = Point(c / 100, r / 100);
//	Point p2 = Point(c - c / 100, r - r / 100);
//	Rect rect = Rect(p1, p2);*/
//	
//
//	//mask.create(imgSource.size(), CV_8UC3);
//
//	
//
//	grabCut(downsampled, mask, rect, bgdModel, fgdModel, iters, GC_INIT_WITH_RECT);
//
//
//	
//	//compare(mask, GC_PR_FGD, mask, CMP_EQ);
//	// upsample the resulting mask
//
//	cv::Mat resultUp;
//	if (downsample == 2)
//	{
//
//		cv::pyrUp(mask, resultUp, cv::Size(mask.cols * 2, mask.rows * 2));
//	}
//	else
//		resultUp = mask;
//
//	Mat source = Mat(1, 1, CV_8U, Scalar(3.0));
//	compare(resultUp, source/* GC_PR_FGD */, resultUp, CMP_EQ);
//
//	// Generate output image
//	cv::Mat foreground(resultUp.size(), CV_8UC3, cv::Scalar(255, 255, 255, 255));
//	
//	imgSource.copyTo(foreground, resultUp); // bg pixels not copied
//
//	
//	//Mat resultUp;
//	//pyrUp(result, resultUp, Size(result.cols * 2, result.rows * 2));
//
//	////CONVERT TO PNG 
//	//Mat source = Mat(1, 1, CV_8U, Scalar(3.0));
//	////compare(mask, source/* GC_PR_FGD */, mask, CMP_EQ);	
//	//Mat foreground = Mat(imgSource.size(), CV_8UC3, Scalar(255,	255, 255, 255));
//
//	//
//	//imgSource.copyTo(foreground, resultUp);
//
//	return foreground;
//}



extern "C" BGREMOVE_API char*  __stdcall  fnbgRemove(char* base64img, int iterations, int sample)
{
	string encoded_string = base64img; 

	string decoded_string = base64_decode(encoded_string);
	vector<uchar> data(decoded_string.begin(), decoded_string.end());
	Mat img = imdecode(data, IMREAD_UNCHANGED);

	Mat dest;

	//imshow("Image", img);
	

	try {
		dest = bgRemove(img, iterations, sample);
		
		
	}
	catch (Exception ex) 
	{
		return NULL;
	}
	
	

	int params[3] = { 0 };
	params[0] = IMWRITE_PNG_COMPRESSION; // IMWRITE_PNG_BILEVEL;//IMWRITE_JPEG_QUALITY;
	params[1] = 0;

	
	vector<uchar> buf;
	bool code = cv::imencode(".png", dest, buf, std::vector<int>(params, params + 2));
	uchar* result = reinterpret_cast<uchar*> (&buf[0]);

	string strretbase64 =base64_encode(result, buf.size());

	//waitKey();

	char* pcharRet = new char[strretbase64.length()];
	strcpy(pcharRet, strretbase64.c_str());
	return pcharRet;

	
}







//
//
//static const std::string base64_chars =
//"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
//"abcdefghijklmnopqrstuvwxyz"
//"0123456789+/";
//
//
//static inline bool is_base64(unsigned char c) {
//	return (isalnum(c) || (c == '+') || (c == '/'));
//}
//
//std::string base64_decode(std::string const& encoded_string) {
//	int in_len = encoded_string.size();
//	int i = 0;
//	int j = 0;
//	int in_ = 0;
//	unsigned char char_array_4[4], char_array_3[3];
//	std::string ret;
//
//	while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
//		char_array_4[i++] = encoded_string[in_]; in_++;
//		if (i == 4) {
//			for (i = 0; i < 4; i++)
//				char_array_4[i] = base64_chars.find(char_array_4[i]);
//
//			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
//			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
//			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
//
//			for (i = 0; (i < 3); i++)
//				ret += char_array_3[i];
//			i = 0;
//		}
//	}
//
//	if (i) {
//		for (j = i; j < 4; j++)
//			char_array_4[j] = 0;
//
//		for (j = 0; j < 4; j++)
//			char_array_4[j] = base64_chars.find(char_array_4[j]);
//
//		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
//		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
//		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
//
//		for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
//	}
//
//	return ret;
//}
