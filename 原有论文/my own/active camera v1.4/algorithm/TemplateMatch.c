 /*****************************************************************************
 * File:	TemplateMatch.c
 * Desc:	functions of Template Match Tracking Algorithm
 * Author:	Xuhui Zhou  @ Carnegie Mellon University
 * Date:	05/20/2004
 *****************************************************************************/

#include "TemplateMatch.h"

IplImage	*gTemplate;		//initialized template image (w*h)
IplImage	*gTrackChannel;	//single channel from current frame (W*H)
IplImage	*gResultImg;		//matching result (W-w+1)*(H-h+1)
IplImage	*gMatchImg;		//matching result with same size of input frame
IplImage	*gImgFgMask;
IplImage	*gImgObjMask;

///////////////////////////////////////////////////////////////////////////////
void temp_TrackCleanUp()
//release memory
{
	//projection image
	cvReleaseImage(&gTemplate);
	cvReleaseImage(&gTrackChannel);	
	cvReleaseImage(&gResultImg);
	cvReleaseImage(&gMatchImg);	
	cvReleaseImage(&gImgFgMask);
	cvReleaseImage(&gImgObjMask);
}

///////////////////////////////////////////////////////////////////////////////
void temp_TrackInit_Coeff(IplImage* inImage, IplImage* inMask, RECT inTargetBox)
//tracker init with box and mask
{	
	int		imgWidth, imgHeight;
	CvRect	templRoi;

	//check whether input is valid
	if (inImage==NULL) return;
	utl_RectCheckBound(&inTargetBox, inImage->width, inImage->height);

	imgWidth	= inImage->width;
	imgHeight	= inImage->height;

	//release memory in case re-initialization
	temp_TrackCleanUp();

	//create track template
	gTemplate	= cvCreateImage(cvSize(imgWidth, imgHeight), 8, 1);
	//create one channel image
	gTrackChannel = cvCreateImage(cvSize(imgWidth, imgHeight), 8, 1);
	//create weight image
	gMatchImg = cvCreateImage(cvSize(imgWidth, imgHeight), 8, 1);
	//create result image
	gResultImg = cvCreateImage(cvSize(imgWidth, imgHeight), 32, 1);
	gImgFgMask = cvCreateImage(cvGetSize(inImage), 8, 1);
	gImgObjMask = cvCreateImage(cvGetSize(inImage), 8, 3);

	//get green channel
	cvCvtPixToPlane(inImage, NULL, gTrackChannel, NULL, NULL);		

	//get template
	//cvSetZero(gTemplate);	
	cvCopy(gTrackChannel, gTemplate, NULL);
	templRoi.x = inTargetBox.left;
	templRoi.y = inTargetBox.top;
	templRoi.width = inTargetBox.right - inTargetBox.left+1;
	templRoi.height = inTargetBox.bottom - inTargetBox.top+1;
	cvSetImageROI(gTemplate, templRoi);		
	
	//debug display
	//cvFlip(gTrackChannel,NULL,0);
	//cvSaveImage("c:\\CTracker\\gTemplate.bmp", gTemplate);
}

//////////////////////////////////////////////////////////////////////////////
void temp_TrackNextFrame_Coeff(IplImage* inImage, RECT inStartBox, TkResult *outResult)
//track one frame
{		
	double	max;
	int		maxX, maxY;
	RECT	targetBox;
	int		boxWidth, boxHeight;
	int		boxHalfWidth, boxHalfHeight;
	int		border;
	CvRect srcRoi, rstRoi;
	CvPoint	maxPoint;
	
	//check whether input is valid
	if (inImage==NULL) return;
	utl_RectCheckBound(&inStartBox, inImage->width, inImage->height);

	boxWidth	= inStartBox.right-inStartBox.left;
	boxHeight	= inStartBox.bottom-inStartBox.top;
	boxHalfWidth	= boxWidth/2;
	boxHalfHeight	= boxHeight/2;

	//get green channel (gMatchImg as a temporary buffer.)
	cvCvtPixToPlane(inImage, NULL, gTrackChannel, NULL, NULL);		

	//get search window
	border = (int)fn_Round(0.5*max(gTemplate->roi->width,gTemplate->roi->height));
	utl_RectSizeIncrease(&inStartBox, border, inImage->width, inImage->height);

	//set search roi
	srcRoi.x = inStartBox.left;
	srcRoi.y = inStartBox.top;
	srcRoi.width = inStartBox.right - inStartBox.left;
	srcRoi.height = inStartBox.bottom - inStartBox.top;	

	cvSetZero(gResultImg);
	rstRoi.x = srcRoi.x + (gTemplate->roi->width-1)/2;
	rstRoi.y = srcRoi.y + (gTemplate->roi->height-1)/2;
	rstRoi.width = srcRoi.width - gTemplate->roi->width + 1;
	rstRoi.height = srcRoi.height - gTemplate->roi->height + 1;	
	if (rstRoi.width<=0 || rstRoi.height<=0) return;

	cvSetImageROI(gTrackChannel, srcRoi);
	cvSetImageROI(gResultImg, rstRoi);
	
	//match with template		
	cvMatchTemplate(gTrackChannel, gTemplate, gResultImg, CV_TM_CCOEFF_NORMED);
	//cvMatchTemplate(gTrackChannel, gTemplate, gResultImg, CV_TM_SQDIFF_NORMED);

	//reset roi
	gTrackChannel->roi = NULL;
	gResultImg->roi = NULL;	

	//scale 255 and convert to Byte image	
	cvConvertScale(gResultImg, gMatchImg, 255, 0);		

	//get max location
	cvMinMaxLoc(gMatchImg, NULL,  &max, NULL, &maxPoint,NULL);
	maxX = maxPoint.x;
	maxY = maxPoint.y;

	//get target rect
	targetBox.left		= (long)(maxX - boxHalfWidth);
	targetBox.right		= targetBox.left + boxWidth;
	targetBox.top		= (long)(maxY - boxHalfHeight);
	targetBox.bottom	= targetBox.top + boxHeight;	
	
	targetBox.left		= max(targetBox.left,0);
	targetBox.top		= max(targetBox.top,0);
	targetBox.right		= min(targetBox.right,inImage->width-1);
	targetBox.bottom	= min(targetBox.bottom,inImage->height-1);

	//get FG object mask image
	{
		CvRect  roi;	
		cvSetZero(gImgFgMask);	
		cvSetZero(gImgObjMask);	
		roi.x = targetBox.left;
		roi.y = targetBox.top;
		roi.width = targetBox.right-targetBox.left+1;
		roi.height = targetBox.bottom-targetBox.top+1;
		cvSetImageROI(gImgFgMask, roi);		
		cvSetImageROI(inImage, roi);
		cvSetImageROI(gImgObjMask, roi);		
		cvThreshold(gImgFgMask, gImgFgMask, 0, 255, CV_THRESH_BINARY_INV);
		cvCopy(inImage, gImgObjMask, gImgFgMask);		
		cvResetImageROI(gImgFgMask);
		cvResetImageROI(inImage);		
		cvResetImageROI(gImgObjMask);	
	}	
	outResult->FGMask	= gImgFgMask;
	outResult->ObjMask	= gImgObjMask;
	
	//return tracker result
	outResult->targetBox	= targetBox;
	outResult->FGImage		= gMatchImg;
	outResult->score		= (float)max;
	outResult->occlusion	= (max<170? TRUE:FALSE);		
}