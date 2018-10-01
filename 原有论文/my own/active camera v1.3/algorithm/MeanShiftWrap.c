 /*****************************************************************************
 * File:	MeanShiftWrap.c
 * Desc:	Wrapper function of (R-G):(G-B):(R+G+B) MeanShift Tracker
 * Author:	Xuhui Zhou  @ Carnegie Mellon University
 * Date:	11/18/2003
 *****************************************************************************/

//#include "stdafx.h"
#include "MeanShiftWrap.h"
#include "MeanShift/meanshift_tracker.h"
#include "MeanShift/imageprocessing.h"

#define		MAXROWS 600
#define		MAXCOLS 800
#define		WID_TRESHOLD 400
#define		HISTLEN 8*8*4

float		gModelhist[HISTLEN];
int			gDatahist[HISTLEN];
IplImage*	gcWeightImg=NULL;
IplImage*	gcTrackImg=NULL;
IplImage	*gImgFgMask;
IplImage	*gImgObjMask;

float		gBhattScore;

///////////////////////////////////////////////////////////////////////////////
IplImage* msw_GetWeightImg()
{
	return gcWeightImg;
}

///////////////////////////////////////////////////////////////////////////////
IplImage* msw_GetTrackImg()
{
	return gcTrackImg;
}
///////////////////////////////////////////////////////////////////////////////
void msw_ClearWeightImg(){
	cvSetZero(gcWeightImg);
}

///////////////////////////////////////////////////////////////////////////////
void msw_PrintHistogram(char* filename)
//print histogram for debug purpose
{
	FILE *file;
	int i;

	file = fopen(filename, "w");
	
	for (i=0;i<HISTLEN;i++){
		fprintf(file, "%f \n", gModelhist[i]);
	}	
	
	fclose(file);	
}

///////////////////////////////////////////////////////////////////////////////
void msw_TrackInit(IplImage* inImage, IplImage* inMask, RECT inTargetBox)
//tracker init with box and mask
{
	int		NUMROWS, NUMCOLS;
	IplImage *planeImg, *indexImg;
	BYTE	pixel;
	int		row, col;
	int		i;	
	BOOL	bResult =0;
	float	accum;
	BYTE    maskPix;

	//check whether input is valid
	if (inImage==NULL) return;
	utl_RectCheckBound(&inTargetBox, inImage->width, inImage->height);

	NUMROWS = inImage->height;
	NUMCOLS = inImage->width;

	//init image size
	setMeanshiftSize(NUMROWS,NUMCOLS);

	//clean memory in re-initialization
	msw_TrackCleanUp();

	//create track image
	gcTrackImg = cvCreateImage(cvSize(NUMCOLS, NUMROWS), 8, 3);
	//create weight image
	gcWeightImg = cvCreateImage(cvSize(NUMCOLS, NUMROWS), 8, 1);
	//calculate index image
	planeImg = cvCreateImage(cvSize(NUMCOLS, NUMROWS), 8, 3);
	indexImg = cvCreateImage(cvSize(NUMCOLS, NUMROWS), 8, 1);    
	gImgFgMask = cvCreateImage(cvGetSize(inImage), 8, 1);
	gImgObjMask = cvCreateImage(cvGetSize(inImage), 8, 3);

	separatePlanes(inImage->imageData, planeImg->imageData, NUMROWS, NUMCOLS);
    color_index_image(planeImg->imageData, NUMROWS, NUMCOLS, indexImg->imageData, 1);	

	//GetRgnBox(hRegion, &rgnBox);

	//clear histogram array
	for(i=0;i<HISTLEN;i++)
		gModelhist[i] = 0;

	//count index image
	for (row=inTargetBox.top;row<=inTargetBox.bottom;row++){	
		for (col=inTargetBox.left; col<=inTargetBox.right; col++){		
			maskPix = (BYTE)cvGetReal2D(inMask, row, col);
			if (maskPix>0){			
				//add into histogram model
				pixel = (BYTE)cvGetReal2D(indexImg,row,col);
				gModelhist[pixel]++;
			}
		}
	}
	
	//debug display
	//cvFlip(indexImg,NULL,0);
	//cvSaveImage("result/indexImg_rgn.bmp", indexImg);

	// make norm(histogram) = 1 //
	accum=0.0;
	for  (i=0; i<HISTLEN; i++){
		accum += (gModelhist[i] * gModelhist[i]);
		//accum += gModelhist[i];
	}
	accum = (float)sqrt(accum);

	for  (i=0; i < HISTLEN; i++){	
		gModelhist[i] /= accum;
	}

	cvReleaseImage(&planeImg);
	cvReleaseImage(&indexImg);
}


//////////////////////////////////////////////////////////////////////////////
void msw_TrackNextFrame(IplImage* inImage, RECT inStartBox, TkResult *outResult)
//track one frame
{	
	double blobIn[5], blobOut[6];
	RECT	targetBox;

	//check whether input is valid
	if (inImage==NULL) return;
	utl_RectCheckBound(&inStartBox, inImage->width, inImage->height);

	//blob track
	blobIn[0] = (inStartBox.top + inStartBox.bottom)/2; //center row
	blobIn[1] = (inStartBox.left + inStartBox.right)/2; //center col
	blobIn[2] = fn_Abs(inStartBox.top - inStartBox.bottom)/2;		//half height (y)
	blobIn[3] = fn_Abs(inStartBox.right - inStartBox.left)/2;		//half width (x)
	blobIn[4] = 1;		
	meantrack_run_scale2D(blobIn, blobOut, (BYTE*)inImage->imageData, gModelhist, gDatahist, &gBhattScore, (BYTE*)gcWeightImg->imageData);	
	
	//get target rect
	targetBox.left		= (long)(blobOut[1]-blobOut[3]);
	targetBox.right		= (long)(blobOut[1]+blobOut[3]);
	targetBox.top		= (long)(blobOut[0]-blobOut[2]);
	targetBox.bottom	= (long)(blobOut[0]+blobOut[2]);	
	
	targetBox.left		= max(targetBox.left,0);
	targetBox.top		= max(targetBox.top,0);
	targetBox.right		= min(targetBox.right,inImage->width);
	targetBox.bottom	= min(targetBox.bottom,inImage->height);

	//get FG object mask image
	{
		CvRect  roi;	
		cvSetZero(gImgFgMask);	
		cvSetZero(gImgObjMask);	
		roi.x = targetBox.left;
		roi.y = targetBox.top;
		roi.width = targetBox.right-targetBox.left;
		roi.height = targetBox.bottom-targetBox.top;
		cvSetImageROI(gcWeightImg, roi);
		cvSetImageROI(gImgFgMask, roi);		
		cvSetImageROI(inImage, roi);
		cvSetImageROI(gImgObjMask, roi);		
		cvThreshold(gcWeightImg, gImgFgMask, 10, 255, CV_THRESH_BINARY);
		cvCopy(inImage, gImgObjMask, gImgFgMask);
		cvResetImageROI(gcWeightImg);
		cvResetImageROI(gImgFgMask);
		cvResetImageROI(inImage);		
		cvResetImageROI(gImgObjMask);	
	}	
	outResult->FGMask	= gImgFgMask;
	outResult->ObjMask	= gImgObjMask;
	
	//return tracker result
	outResult->targetBox	= targetBox;
	outResult->FGImage		= gcWeightImg;
	outResult->score		= gBhattScore;
	outResult->occlusion	= (gBhattScore<0.4? TRUE:FALSE);	
}

///////////////////////////////////////////////////////////////////////////////
void msw_TrackCleanUp()
//release memory
{
	//projection image
	cvReleaseImage(&gcTrackImg);
	cvReleaseImage(&gcWeightImg);
	cvReleaseImage(&gImgFgMask);
	cvReleaseImage(&gImgObjMask);
}
