// ForTest.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include "stdio.h"
#include "math.h"
#include "string.h"
#include "pxc.h"
#include "highgui.h"
#include "new.h"
#include "Utility.h"
#include "KltWrap.h"
#include "Histogram.h"

#define	POSNUM 40

typedef struct _FPOINT{
    float  x;
    float  y;
} 
FPOINT;
int main(int argc, char* argv[])
{
	IplImage* pInput = NULL;
	IplImage* pOutput = NULL;
	char ori_w[200];

    int m = 0;
	int n = 0;

	strcpy(ori_w,"original_window");

	cvNamedWindow( ori_w, CV_WINDOW_AUTOSIZE );

	CvRect  roi;
	roi.x = 135;
	roi.y = 124;
	roi.height = 76;
	roi.width  = 62;

	RECT Rect;

	int gRbins=10;
	int	gGbins=10;
	int	gBbins=10;
	
	TkResult gTkResult;
	gTkResult.FGImage = NULL;
	gTkResult.FGMask = NULL;
	gTkResult.ObjMask = NULL;	

	IplImage	*gImgInitMaskHalf=NULL;

	bool bKltFlag = true;

	float	gAffineMatrix[6];
	float	gZoomAccum = 1; //accumulative scale factor
	float	zoom;
	int	gBlobWidthInit, gBlobHeightInit; //initial dimension of tracking blob
	int	gBlobWidth, gBlobHeight; //blob dimension of previous frame

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
			m = pInput->width;
			n = pInput->height;

			gImgInitMaskHalf = cvCreateImage(cvSize(m, n), 8, 1);
			
			Rect.bottom = 196;
			Rect.top    = 121;
			Rect.left   = 136;
			Rect.right  = 197;
			his_TrackInit_Bins(pInput, gImgInitMaskHalf, Rect, gRbins, gGbins, gBbins);
			gTkResult.FGImage = his_GetBackProjImage();
			for (int i=0;i<6;i++)
			{		
				gAffineMatrix[i] = 0;
			}
			gBlobWidth = gBlobWidthInit = Rect.right-Rect.left;
			gBlobHeight = gBlobHeightInit = Rect.bottom-Rect.top;
			klt_TrackInit(pInput);
		}
		else
		{
			if (bKltFlag)
			{
				klt_TrackNextFrame(pInput, gAffineMatrix);

				//get zoom scale factor from affine matrix		
				zoom = (float)sqrt(fn_Abs(gAffineMatrix[0]*gAffineMatrix[4] - gAffineMatrix[1]*gAffineMatrix[3]));
				
				//if zoom scale is not 1 (different above threshold), accumulate zoom change.
				if (fabs(zoom-1)>0.002){		
					gZoomAccum = gZoomAccum*zoom;
				}

				//change size if not in predict mood		
				//change rect size based on scale factor accumulation
				gBlobWidth = (int)fn_Round(gBlobWidthInit * gZoomAccum);
				gBlobHeight = (int)fn_Round(gBlobHeightInit * gZoomAccum);

				//adjust the current Blob size
				utl_RectSizeAdjust(&Rect, gBlobWidth, gBlobHeight, m, n);
			}
			else
			{
				for (int i=0;i<6;i++)
				{		
					gAffineMatrix[i] = 0;
				}
			}
			his_TrackNextFrame(pInput, Rect, &gTkResult);
			Rect = gTkResult.targetBox;
		}
		
		cvRectangle(pInput, cvPoint(Rect.left,Rect.top),cvPoint(Rect.right,Rect.bottom),CV_RGB(255,0,0));

		cvShowImage(ori_w,pInput);
		cvWaitKey(1);
		cvReleaseImage(&pInput);
	}
	his_TrackCleanUp();
	cvReleaseImage(&gImgInitMaskHalf);
	return 0;
}

