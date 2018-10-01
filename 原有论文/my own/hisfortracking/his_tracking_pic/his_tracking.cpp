// his_tracking.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include "math.h"
#include "string.h"
#include "highgui.h"
//#include "Hisfortrack.h"
//#include "MeanShift.h"
#include <time.h>
//#include "FeatMS.h"
#include "cv.h"
#include "cxcore.h"
#include "Msfeature.h"


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

	RECT Rect;

	int gRbins=10;
	int	gGbins=10;
	int	gBbins=10;	

	
	TkResult Result;

	clock_t tstart, tend;
	double tsum = 0;
//	tstart = clock();
	for(int pic_num = 0; pic_num<=2159; pic_num++)
	{

		char filename[100];
		char append[50];

		sprintf(filename,"E:\\algorithm software\\tts\\tts\\");

		if (pic_num<10)
		{
			sprintf(append,"tts000%d.jpg", pic_num );
		}
		else if (pic_num<100)
		{
			sprintf(append,"tts00%d.jpg", pic_num );
		}
		else if (pic_num<1000)
		{
			sprintf(append,"tts0%d.jpg", pic_num );
		}
		else
		{
			sprintf(append,"tts%d.jpg", pic_num );
		}
		strcat(filename,append);
		
		pInput = cvLoadImage(filename,-1);

		//³õÊ¼»¯
		if (pic_num == 0)
		{
			m = pInput->width/2;
			n = pInput->height/2;
			pInputHalf = cvCreateImage(cvSize(m, n), 8, 3);

			Rect.bottom = (int)(196/2);
			Rect.top    = (int)(121/2);
			Rect.left   = (int)(136/2);
			Rect.right  = (int)(197/2);

			pOutput = cvCreateImage(cvSize(m, n), 8, 1);

			cvResize(pInput, pInputHalf, CV_INTER_LINEAR);

			feat_TrackInit(pInputHalf, NULL, Rect);
		}
		else
		{
			cvResize(pInput, pInputHalf, CV_INTER_LINEAR);
			tstart = clock();
			feat_TrackNextFrame_Adapt(pInputHalf, Rect, &Result);
			tend = clock();
			tsum += (double)tend-(double)tstart;
			Rect = Result.targetBox;
			memcpy(pOutput->imageData, Result.FGImage->imageData, sizeof(unsigned char)*pOutput->widthStep*n);
		}
		
		cvRectangle(pInput, cvPoint(Rect.left*2,Rect.top*2),cvPoint(Rect.right*2,Rect.bottom*2),CV_RGB(255,0,0));

		cvShowImage(ori_w,pInput);
		cvShowImage(ori_m,pOutput);
		cvWaitKey(1);
		cvReleaseImage(&pInput);
	}
//	tend   = clock();
	double averagetime = double(tsum)/2158;
	
	printf("%f", averagetime);
	cvReleaseImage(&pOutput);
	return 0;
}

