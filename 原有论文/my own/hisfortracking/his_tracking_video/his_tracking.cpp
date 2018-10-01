// his_tracking.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include "math.h"
#include "string.h"
#include "highgui.h"
#include "Hisfortrack.h"
//#include "MeanShift.h"
#include <time.h>
//#include "FeatMS.h"
#include "cv.h"
#include "cxcore.h"
//#include "Msfeature.h"


int main(int argc, char* argv[])
{
	IplImage* pInput = NULL;
	IplImage* pInputHalf = NULL;
	IplImage* pOutput = NULL;
	char ori_w[200];
	char ori_m[200];

    int m = 0;
	int n = 0;

	strcpy(ori_w,"original_window");
	strcpy(ori_m,"median_data");

	cvNamedWindow( ori_w, CV_WINDOW_AUTOSIZE );
	cvNamedWindow( ori_m, CV_WINDOW_AUTOSIZE );

	CRECT Rect;

	int gRbins=10;
	int	gGbins=10;
	int	gBbins=10;	

	CvCapture* pVideo = 0;
	pVideo = cvCaptureFromFile("e:\\5.avi");
	if(pVideo == 0)
	{
		printf("Fail to open file");
		return -1;
	}
	

	TkResult Result;

	CHisfortrack tracker;
	clock_t tstart, tend;
	double tsum = 0;
//	tstart = clock();
	long pic_num = 0;
	while(pInput = cvQueryFrame(pVideo))
	{

		//³õÊ¼»¯
		if (pic_num == 0)
		{
			m = pInput->width/2;
			n = pInput->height/2;
			pInputHalf = cvCreateImage(cvSize(m, n), 8, 3);

			Rect.bottom = (int)(179/2);//196,113,93,162
			Rect.top    = (int)(138/2);
			Rect.left   = (int)(185/2);
			Rect.right  = (int)(219/2);

			pOutput = cvCreateImage(cvSize(m, n), 8, 1);

			cvResize(pInput, pInputHalf, CV_INTER_LINEAR);
			cvFlip(pInputHalf);
//			feat_TrackInit(pInputHalf, NULL, Rect);
			tracker.Initial((unsigned char *)pInputHalf->imageData, NULL, Rect, pInputHalf->width, pInputHalf->height,10,10,10);
			cvRectangle(pInput, cvPoint(Rect.left*2,2*n-Rect.top*2),cvPoint(Rect.right*2,2*n-Rect.bottom*2),CV_RGB(255,0,0));
		}
		else
		{
			if (pic_num == 114)
			{
				int zz = 0;
			}
			cvResize(pInput, pInputHalf, CV_INTER_LINEAR);
			cvFlip(pInputHalf);
			tstart = clock();
//			feat_TrackNextFrame_Adapt(pInputHalf, Rect, &Result);

			tracker.TrackNextFrame((unsigned char *)pInputHalf->imageData, Rect, &Result);
			tend = clock();
			tsum += (double)tend-(double)tstart;
			if (!Result.occlusion)
			{
				Rect = Result.targetBox;
			}
			
			memcpy(pOutput->imageData, Result.FGImage, sizeof(unsigned char)*pOutput->widthStep*n);
//			memcpy(pOutput->imageData, Result.FGImage, sizeof(unsigned char)*pOutput->widthStep*n);

		}
		
		cvRectangle(pInput, cvPoint(Rect.left*2,2*n-Rect.top*2),cvPoint(Rect.right*2,2*n-Rect.bottom*2),CV_RGB(255,0,0));

		cvShowImage(ori_w,pInput);
		cvShowImage(ori_m,pOutput);
		cvWaitKey(1);
//		cvReleaseImage(&pInput);
		pic_num++;
	}
//	tend   = clock();
	double averagetime = double(tsum)/2158;
	
	printf("%f", averagetime);
	cvReleaseImage(&pOutput);
	return 0;
}

