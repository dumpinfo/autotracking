  /*****************************************************************************
 * File:	Histogram.c
 * Desc:	functions of Histogram Tracking Algorithm
 * Author:	Xuhui Zhou  @ Carnegie Mellon University
 * Date:	12/09/2003
 *****************************************************************************/

#include "Histogram.h"

//histogram bins
int  RBINS = 10;
int  GBINS = 10;
int  BBINS = 10;

//global histogram
CvHistogram  *gHistInit; //init click hist
CvHistogram  *gHistModel; //model hist without outer ring
CvHistogram	 *gHistRatio;	//ratio of foreground/(foreground+background)
CvHistogram	 *gHistDivide;	//ratio of foreground/(foreground+background)
CvHistogram	 *gHistDiff;	//Difference of foreground-background

//projection image
IplImage	*gImgBackProj;
IplImage	*gImgFgMask;
IplImage	*gImgObjMask;

//global path
char		gExePath[_MAX_PATH];

POINT		gGravityCenter;
int			gBorder = 20; //border width for outer ring box
int			gTrackMethod = 1; //SHIFT, 2:CONV

int			gMaskPixCount;
float		gMaskFgRatio;

///////////////////////////////////////////////////////////////////////////////
void his_PrintHistogram(CvHistogram *hist, char* filename)
{
	FILE	*file;
	float	data;
	int 	h, s, v;

	file = fopen(filename, "w");
	for( h = 0; h < RBINS; h++ ){			
		for( s = 0; s < GBINS; s++ ){			
			fprintf(file, "%02d_%02d_(0~%02d): ", h, s, BBINS-1);
			for(v = 0; v < BBINS; v++ ){						
				data = cvQueryHistValue_3D(hist,h,s,v);
				fprintf(file, "%3d ", (long)data );						
			}			
			fprintf(file, "\n");
		}		
	}
	fclose(file);	
}

/******************************************************************************/
CvHistogram* his_CalcHistogram(IplImage* inImg){
/******************************************************************************/
	CvHistogram *outHist;
	CvSize imgSize = cvGetSize(inImg);	
	IplImage* h_plane = cvCreateImage(imgSize, 8, 1 );
    IplImage* s_plane = cvCreateImage(imgSize, 8, 1 );
    IplImage* v_plane = cvCreateImage(imgSize, 8, 1 );        
    float h_ranges[] = { 0, 255}; /* hue varies from 0 (~0°red) to 180 (~360°red again) */
    float s_ranges[] = { 0, 255}; /* saturation varies from 0 (black-gray-white) to 255 (pure spectrum color) */
	float v_ranges[] = { 0, 255}; 
    int		hist_size[] = {RBINS, GBINS, BBINS};
	float*	ranges[] = { h_ranges, s_ranges, v_ranges};
	IplImage* planes[] = { h_plane, s_plane, v_plane};
	float max_value = 0;
    
    cvCvtPixToPlane(inImg, h_plane, s_plane, v_plane, 0 );
    outHist = cvCreateHist(3, hist_size, CV_HIST_ARRAY, ranges, 1 );
    cvCalcHist( planes, outHist, 0, 0 );

	//nornalize histogram
	cvNormalizeHist(outHist, 255);
	cvGetMinMaxHistValue( outHist, 0, &max_value, 0, 0 );

	//cvThreshHist(outHist, max_value*0.5);
	//cvGetMinMaxHistValue( outHist, 0, &max_value, 0, 0 );	

	cvReleaseImage(&h_plane);
	cvReleaseImage(&s_plane);	
	cvReleaseImage(&v_plane);	
	return outHist;
}


/******************************************************************************/
CvHistogram* his_HistogramFromArray(float* modelhist){
/******************************************************************************/		
	CvHistogram *outHist;
	int  d[1] = {256};
	int  bin;
	float *p;		
	float range[] = {0,255};
	float *ranges[] = {range};	
	float maxVal,factor; //minVal
	//int		minIdx, maxIdx;

	outHist = cvCreateHist(1,d,CV_HIST_ARRAY,ranges,1);
	
	maxVal = 0;
	for (bin=0; bin<256; bin++){		
		p = cvGetHistValue_1D(outHist, bin);
		//*p = modelhist[bin];
		*p = bin+1.0f;
		if (modelhist[bin]>maxVal){
			maxVal = modelhist[bin];
		}
	}
		
	
	if (maxVal!=0){
		factor = 255;
		cvNormalizeHist(outHist, factor);
 	}	

	return outHist;
}

/******************************************************************************/
CvHistogram* his_HistogramRatio(CvHistogram* hist1_model, CvHistogram *hist2_denom)
//ratio by probDensity
{
	CvHistogram *outHist=NULL;
	
	cvCopyHist(hist1_model, &outHist);
	cvClearHist(outHist);
	cvCalcProbDensity(hist2_denom,hist1_model,outHist,255);
		
	return outHist;
}

/******************************************************************************/
CvHistogram* his_HistogramDivide(CvHistogram* hist1, CvHistogram *hist2)
//divide and normalize
{
	CvHistogram *histRst=NULL;	
	int b1, b2, b3;
	float   *p1, *p2, *p3;
	float	rst;

	cvCopyHist(hist1, &histRst);
	cvClearHist(histRst);

	for( b1 = 0; b1 < RBINS; b1++ ){			
		for( b2 = 0; b2 < GBINS; b2++ ){
			for( b3 = 0; b3 < BBINS; b3++ ){			
				p1 = cvGetHistValue_3D(hist1,b1,b2,b3);
				p2 = cvGetHistValue_3D(hist2,b1,b2,b3);
				p3 = cvGetHistValue_3D(histRst,b1,b2,b3);
				
				if ((*p2)==0) rst = 0;
				else
					rst = (*p1)/(*p2);
				(*p3) = max(rst, 0);				
			}
		}
	}					

	his_HistogramNormalize(histRst);
		
	return histRst;
}

/******************************************************************************/
void his_HistogramDiff(CvHistogram* hist1, CvHistogram *hist2, CvHistogram *histRst)
//inner minus outer; (foreground) - (foreground+background)
{	
	int b1, b2, b3;
	float   *p1, *p2, *p3;
	float	rst;

	for( b1 = 0; b1 < RBINS; b1++ ){			
		for( b2 = 0; b2 < GBINS; b2++ ){
			for( b3 = 0; b3 < BBINS; b3++ ){			
				p1 = cvGetHistValue_3D(hist1,b1,b2,b3);
				p2 = cvGetHistValue_3D(hist2,b1,b2,b3);
				p3 = cvGetHistValue_3D(histRst,b1,b2,b3);
				
				rst = (*p1)*2 - (*p2);
				(*p3) = max(rst, 0);				
			}
		}
	}					
}

/******************************************************************************/
void his_HistogramNormalize(CvHistogram* inHist)
//make the max as 255, multiply by 255/max
{
	int b1, b2, b3;
	float	maxVal, minVal;
	//float	val; //histogram count value
	float   *pix;

	//get max value
	cvGetMinMaxHistValue( inHist, &minVal, &maxVal, 0, 0 );			

	if (maxVal==0) return;

	//get min non-zero value
	minVal = 255;
	for( b1 = 0; b1 < RBINS; b1++ ){			
		for( b2 = 0; b2 < GBINS; b2++ ){
			for( b3 = 0; b3 < BBINS; b3++ ){			
				pix = cvGetHistValue_3D(inHist,b1,b2,b3);
				if ((*pix)>0 && (*pix)<minVal){
					minVal = (*pix);
				}
			}
		}
	}					


	//normalize
	for( b1 = 0; b1 < RBINS; b1++ ){			
		for( b2 = 0; b2 < GBINS; b2++ ){
			for( b3 = 0; b3 < BBINS; b3++ ){			
				pix = cvGetHistValue_3D(inHist,b1,b2,b3);
				if (*pix>0){
					(*pix) = ((*pix)-minVal)*255/(maxVal-minVal);
				}
			}
		}
	}					
}

/******************************************************************************/
void his_HistogramThresholdUp(CvHistogram* inHist, float thresholdVal)
//make pix 255 if above threshold
{
	int b1, b2, b3;
	float	max_value = 0;
	//float	val; //histogram count value
	float   *pix;

	//get max value
	cvGetMinMaxHistValue( gHistModel, 0, &max_value, 0, 0 );			

	//normalize
	for( b1 = 0; b1 < RBINS; b1++ ){			
		for( b2 = 0; b2 < GBINS; b2++ ){
			for( b3 = 0; b3 < BBINS; b3++ ){			
				pix = cvGetHistValue_3D(inHist,b1,b2,b3);
				if (*pix>thresholdVal){
					(*pix) = 255;			
				}
			}
		}
	}					
}

/******************************************************************************/
CvHistogram* his_HistogramAdd(CvHistogram* hist1, CvHistogram *hist2, float weight){
/******************************************************************************/
	CvHistogram *outHist=NULL;	
	int	h,s,v;
	float h1, h2,*p;

	cvCopyHist(hist1, &outHist);

	for( h = 0; h < RBINS; h++ ){			
		for( s = 0; s < GBINS; s++ ){
			for( v = 0; v < BBINS; v++ ){			
				h1 = cvQueryHistValue_3D(hist1,h,s,v);
				h2 = cvQueryHistValue_3D(hist2,h,s,v);
				p = cvGetHistValue_3D(outHist,h,s,v);
				(*p) = h1*(1-weight) + h2*weight;
			}
		}
	}			
		
	return outHist;
}
/******************************************************************************/
void his_BackProject(BYTE *imgin, BYTE *imgout, CvHistogram *hist){
/******************************************************************************/
	int		imgWidth, imgHeight;	
	IplImage* imgSrc[CV_MAX_DIM];
	IplImage* imgRst = cvCreateImage( cvSize(320,243), IPL_DEPTH_8U, 1 );
	IplImage* tmp	=	cvCreateImage( cvSize(320,243), IPL_DEPTH_8U, 1 );

	imgSrc[0] = cvCreateImage( cvSize(320,243), IPL_DEPTH_8U, 1 );
	//copy input image
	imgWidth = 320;
	imgHeight = 243;	
	memcpy(imgSrc[0]->imageData, imgin, imgWidth*imgHeight);	

	//calculate backproject
	cvCalcBackProject(imgSrc, imgRst, hist);

//	//debug display		
//	cvSaveImage("result/imgsrc.bmp", imgSrc[0]);
//	cvSaveImage("result/imgRst.bmp", imgRst);
//	img_Output_C1_float(imgRst->imageData, imgWidth, imgWidth, imgHeight, "result/imgRst.txt");	

	//copy to output
	memcpy(imgout,imgRst->imageData,imgWidth*imgHeight);
	
	//clear memory
	cvReleaseImage( &imgSrc[0]);
	cvReleaseImage( &imgRst);
}

/******************************************************************************/
IplImage* his_CalcBackProject(IplImage *inImgHSV, CvHistogram *inHist){
/******************************************************************************/		
	//calculate back project		
	CvSize imgSize = cvGetSize(inImgHSV);	
	IplImage*	h_plane = cvCreateImage(imgSize, 8, 1 );
	IplImage*	s_plane = cvCreateImage(imgSize, 8, 1 );
	IplImage*	v_plane = cvCreateImage(imgSize, 8, 1 );
	IplImage*	planes[] = { h_plane, s_plane, v_plane };    	
	IplImage*	bpRst = cvCreateImage(imgSize, 8, 1 );	
		
	//seperate HSV planes
	cvCvtPixToPlane(inImgHSV, h_plane, s_plane, v_plane, 0 );
    
	//calculate backproject
	cvCalcBackProject(planes, bpRst, inHist);

	//cvThreshold(outImgBP, outImgBP, 50, 255, CV_THRESH_TOZERO );
	
	//debug display	
//	{
//		IplImage* tmp = cvCreateImage(imgSize, 8, 1 );
//		cvFlip(inImgHSV,NULL,0);
//		cvSaveImage("result/inImgHSV.bmp",inImgHSV);
//		cvFlip(outImgBP,tmp,0);
//		cvSaveImage("result/outImgBP.bmp",tmp);		
//	}
	
	//clear memory
	cvReleaseImage( &h_plane);
	cvReleaseImage( &s_plane);
	cvReleaseImage( &v_plane);
	return bpRst;
}

/******************************************************************************/
void his_CalcForeground2(IplImage *inImgHSV, CvHistogram *inHist, CvRect inRect, IplImage *outImgFg){
/******************************************************************************/		
	int			border;
	CvRect		rectSearch;	 //search window in tracking
	IplImage	*imgsrc = cvCloneImage(inImgHSV);		
	IplImage	*imgrst;		
	double		dRstMax;
	int			row, col;
	double		pixval;
	
	//extent neighboring search window
	border = gBorder;
	rectSearch.x = inRect.x - border;
	rectSearch.y = inRect.y - border;
	rectSearch.width = inRect.width+2*border;
	rectSearch.height = inRect.height+2*border;
	his_CheckBoundRect(&rectSearch, inImgHSV->width, inImgHSV->height);		
	
	//get back project		
	imgrst = his_CalcBackProject(imgsrc, inHist);

	//scale to 0~255
	cvMinMaxLoc(imgrst,NULL, &dRstMax, NULL,NULL,NULL);	
	if (dRstMax!=0){		
		cvConvertScale(imgrst, imgrst, 255/dRstMax,0);
	}
	
	//copy imgrst to outImgFg
	cvSetZero(outImgFg);
	for (col=rectSearch.x; col<rectSearch.x+rectSearch.width;col++){
		for (row=rectSearch.y;row<rectSearch.y+rectSearch.height;row++){
			pixval = cvGetReal2D(imgrst, row, col);
			cvSetReal2D(outImgFg, row, col, pixval);
		}
	}
	
	//release memory
	cvReleaseImage( &imgsrc);
	cvReleaseImage( &imgrst);
}

/******************************************************************************/
void his_CheckBoundRect(CvRect *rect, int imgWidth, int imgHeight){
/******************************************************************************/
	if (rect->x <0) rect->x=0;
	if (rect->x >imgWidth) rect->x = imgWidth;
	if (rect->y <0) rect->y=0;
	if (rect->y >imgHeight) rect->y = imgHeight;

	if (rect->width >=imgWidth - rect->x)
		rect->width = imgWidth -1 - rect->x;
	if (rect->height>=imgHeight - rect->y)
		rect->height = imgHeight -1 - rect->y;
}

///////////////////////////////////////////////////////////////////////////////
IplImage* his_GetBackProjImage()
{
	return gImgBackProj;
}

///////////////////////////////////////////////////////////////////////////////
void his_ReadHistBinsFromFile(){
	FILE *filein;
	char *binfile = "histogram.txt";
	char prompt[10];
	//int i;
	//char buffer[_MAX_PATH];
	
	//_getcwd(buffer, _MAX_PATH);

	filein = fopen(binfile, "r");
	if (filein==NULL){
		return;
	}
		
	fscanf(filein, "%s", &prompt);
	fscanf(filein, "%d", &RBINS);
	fscanf(filein, "%s", &prompt);
	fscanf(filein, "%d", &GBINS);
	fscanf(filein, "%s", &prompt);
	fscanf(filein, "%d", &BBINS);
	
	fclose(filein);

	//in case of wrong setting
	if(RBINS<=0) 
		RBINS = 10;
	if(GBINS<=0) 
		GBINS = 10;
	if(BBINS<=0) 
		BBINS = 10;

//	//debug 
//	RBINS = 50;
//	GBINS = 50;
//	BBINS = 50;
}

/////////////////////////////////////////////////////////////////////////////////////////
void his_CheckBoxBound(RECT *inRect, int imgWidth, int imgHeight)
//check whether the box (left, right, top, bottm) is out of bound
{
	inRect->left	= max(inRect->left, 0);
	inRect->right	= min(inRect->right, imgWidth-1);
	inRect->top		= max(inRect->top, 0);
	inRect->bottom	= min(inRect->bottom, imgHeight-1);	

	inRect->right = (inRect->right>inRect->left? inRect->right: inRect->left);
	inRect->bottom = (inRect->bottom>inRect->top? inRect->bottom: inRect->top);
}

///////////////////////////////////////////////////////////////////////////////
void his_CountBoxHistogram(IplImage* inImg, RECT inRect, CvHistogram *outHist)
//count histogram within a rect box
{
	int		row, col;
	BYTE	pixH, pixS, pixV;
	int		binH, binS, binV;
	float	*pix;
	
	IplImage* h_plane = cvCreateImage(cvGetSize(inImg), 8, 1 );
    IplImage* s_plane = cvCreateImage(cvGetSize(inImg), 8, 1 );
    IplImage* v_plane = cvCreateImage(cvGetSize(inImg), 8, 1 );

	//Divide image into planes
	cvCvtPixToPlane(inImg, h_plane, s_plane, v_plane, 0 );

	//count histogram
	cvClearHist(outHist);		
	for (row=inRect.top;row<=inRect.bottom;row++){	
		for (col=inRect.left; col<=inRect.right; col++){								
			//calculate histogram
			pixH = (BYTE)cvGetReal2D(h_plane,row,col);
			pixS = (BYTE)cvGetReal2D(s_plane,row,col);
			pixV = (BYTE)cvGetReal2D(v_plane,row,col);
			binH = (int)(RBINS*pixH/256);
			binS = (int)(GBINS*pixS/256);
			binV = (int)(BBINS*pixV/256);

			pix = cvGetHistValue_3D(outHist,binH,binS,binV);
			(*pix)++;
		}
	}
	cvReleaseImage( &h_plane);
	cvReleaseImage( &s_plane);
	cvReleaseImage( &v_plane);
}

/******************************************************************************/
void his_CalcGravityCenter2(IplImage* inImg, RECT blobBox, POINT *outCenter)
//get blob weight center 
{		
	int		row, col;
	int		border = 10;
	BYTE	pixel;
	float	sumRow, sumCol, sumPix;
	POINT	center;
	int		dx, dy;

	//get outer ring box
	center.x = (blobBox.left+blobBox.right)/2;
	center.y = (blobBox.top+blobBox.bottom)/2;

	sumRow = 0;
	sumCol = 0;
	sumPix = 0;
	for (col=blobBox.left; col<blobBox.right; col++){
		for (row=blobBox.top; row<blobBox.bottom; row++){
			pixel = (BYTE)cvGetReal2D(inImg,row,col);
			sumCol = sumCol + pixel*(col-center.x);
			sumRow = sumRow + pixel*(row-center.y);
			sumPix = sumPix + pixel;
		}
	}
	dx = (int)fn_Round(sumCol/sumPix);
	dy = (int)fn_Round(sumRow/sumPix);		

	//get weight center
	outCenter->x = center.x + dx;
	outCenter->y = center.y + dy;
}

/******************************************************************************/
void his_CalcGravityCenter(IplImage* inImg, POINT *outCenter)
//get blob weight center by moments
{	
	CvMoments moments;	

	cvMoments(inImg, &moments, 0);
	outCenter->x = (int)fn_Round(moments.m10/moments.m00);
	outCenter->y = (int)fn_Round(moments.m01/moments.m00);		
}

///////////////////////////////////////////////////////////////////////////////
void his_BlobCenterShift(IplImage* inImg, RECT *inRect, int range)
//shift to track blob center movement
{	
	RECT	shiftBox; 
	int		row, col;
	BYTE	pixel;
	float	sumRow, sumCol, sumPix;
	POINT	center, inCenter;
	//int		halfWidth, halfHeight;
	int		dx, dy;
	int		loopCount;	

	//get outer ring box
	shiftBox.left = inRect->left;
	shiftBox.right = inRect->right;
	shiftBox.top = inRect->top;
	shiftBox.bottom = inRect->bottom;
	his_CheckBoxBound(&shiftBox, inImg->width, inImg->height);
	inCenter.x = (shiftBox.left+shiftBox.right)/2; //input Rect center
	inCenter.y = (shiftBox.top+shiftBox.bottom)/2;
	center.x = inCenter.x; //shift center
	center.y = inCenter.y; 

	loopCount=0;
	dx=1;
	dy=1;
	while (abs(dx)+abs(dy)>=1 && loopCount<10){		
		sumRow = 0;
		sumCol = 0;
		sumPix = 0;
		for (col=shiftBox.left; col<shiftBox.right; col++){
			for (row=shiftBox.top; row<shiftBox.bottom; row++){
				pixel = (BYTE)cvGetReal2D(gImgBackProj,row,col);
				sumCol = sumCol + pixel*(col-center.x);
				sumRow = sumRow + pixel*(row-center.y);
				sumPix = sumPix + pixel;
			}
		}
		dx = (int)fn_Round(sumCol/sumPix);
		dy = (int)fn_Round(sumRow/sumPix);		

		//move track box
		shiftBox.left = shiftBox.left + dx;
		shiftBox.right = shiftBox.right + dx;
		shiftBox.top = shiftBox.top + dy;
		shiftBox.bottom = shiftBox.bottom + dy;
		his_CheckBoxBound(&shiftBox, inImg->width, inImg->height);
		center.x = (shiftBox.left+shiftBox.right)/2;
		center.y = (shiftBox.top+shiftBox.bottom)/2;

		loopCount++;
	}
	//output shift result
	dx = center.x - inCenter.x;
	dy = center.y - inCenter.y;
	if (dx<-range)	dx = -range;
	if (dx>range)	dx = range;
	if (dy<-range)	dy = -range;
	if (dy>range)	dy = range;

	inRect->left	= inRect->left + dx;
	inRect->right	= inRect->right + dx;
	inRect->top		= inRect->top + dy;
	inRect->bottom	= inRect->bottom + dy;
}

///////////////////////////////////////////////////////////////////////////////
void his_TrackInit_Bins(IplImage *inImg, IplImage *inMask, RECT inRect, int nbins1, int nbins2, int nbins3)
//tracker init with RGB bins parameters
{
	//set histogram bins
	RBINS = nbins1;
	GBINS = nbins2;
	BBINS = nbins3;

	//init tracker
	his_TrackInit(inImg, inMask, inRect);
}


///////////////////////////////////////////////////////////////////////////////
void his_TrackInit(IplImage *inImg, IplImage *inMask, RECT inRect)
//tracker init with mask image
{
	//histogram variable
	float	h_ranges[] = { 0, 255}; /* hue varies from 0 (~0°red) to 180 (~360°red again) */
    float	s_ranges[] = { 0, 255}; /* saturation varies from 0 (black-gray-white) to 255 (pure spectrum color) */
	float	v_ranges[] = { 0, 255}; 
    int		hist_size[] = {RBINS, GBINS, BBINS};
	float	max_value = 0;
	float*	ranges[] = { h_ranges, s_ranges, v_ranges};	    	
	int		row, col;
	BYTE	pixR, pixG, pixB;
	float	*pix;
	int		binR, binG, binB;
	BOOL	bResult =0;
	BYTE    maskPix;
	CvScalar	pixel;

	//check whether input is valid
	if (inImg==NULL) return;
	utl_RectCheckBound(&inRect, inImg->width, inImg->height);

	//release memeory in case re-initialize
	his_TrackCleanUp();

	gHistInit = cvCreateHist(3, hist_size, CV_HIST_ARRAY, ranges, 1 );
	gHistModel = cvCreateHist(3, hist_size, CV_HIST_ARRAY, ranges, 1 );			
	//create histogram		
	gImgBackProj = cvCreateImage(cvGetSize(inImg), 8, 1);
	gImgFgMask = cvCreateImage(cvGetSize(inImg), 8, 1);
	gImgObjMask = cvCreateImage(cvGetSize(inImg), 8, 3);

	//clear histogram array
	cvClearHist(gHistInit);
	//count index image
	gMaskPixCount = 0;
	for (row=inRect.top;row<=inRect.bottom;row++){
		for (col=inRect.left; col<=inRect.right; col++){		
			//bResult = PtInRegion(hRegion, col, row);
			maskPix = (BYTE)cvGetReal2D(inMask, row, col);
			if (maskPix>0){			
				gMaskPixCount++;
				//calculate histogram
				pixel = cvGet2D(inImg, row, col);
				pixR = (BYTE)pixel.val[0];
				pixG = (BYTE)pixel.val[1];
				pixB = (BYTE)pixel.val[2];
				binR = (int)(RBINS*pixR/256);
				binG = (int)(GBINS*pixG/256);
				binB = (int)(BBINS*pixB/256);
				pix = cvGetHistValue_3D(gHistInit,binR,binG,binB);
				(*pix)++;
			}			
		}
	}
		
	gMaskFgRatio = (float)gMaskPixCount/((inRect.right-inRect.left)*(inRect.bottom-inRect.top));
		
	//get outer box histogram
	cvClearHist(gHistModel);	
	{
		//outer ring Box
		RECT outerBox;

		//get outer ring box
		outerBox.left	= inRect.left-gBorder;
		outerBox.right	= inRect.right+gBorder;
		outerBox.top	= inRect.top-gBorder;
		outerBox.bottom	= inRect.bottom +gBorder;
		his_CheckBoxBound(&outerBox, inImg->width, inImg->height);

		//count index image
		for (row=outerBox.top;row<=outerBox.bottom;row++){	
			for (col=outerBox.left; col<=outerBox.right; col++){								
				//calculate histogram
				pixel = cvGet2D(inImg, row, col);
				pixR = (BYTE)pixel.val[0];
				pixG = (BYTE)pixel.val[1];
				pixB = (BYTE)pixel.val[2];
				binR = (int)(RBINS*pixR/256);
				binG = (int)(GBINS*pixG/256);
				binB = (int)(BBINS*pixB/256);

				pix = cvGetHistValue_3D(gHistModel,binR,binG,binB);
				(*pix)++;
			}
		}			
	}
		
	//get back project image
	{			
		CvRect	trackRect;			

		//ratio histogram			
		//gHistRatio = his_HistogramRatio(gHistInit, gHistModel);
		gHistRatio = his_HistogramDivide(gHistInit, gHistModel);

		//debug - print out rst
//		{
//			_chdir(gExePath);		
//			his_PrintHistogram(gHistInit, "result/gHistInit.txt");
//			his_PrintHistogram(gHistModel, "result/gHistModel.txt");
//			his_PrintHistogram(gHistRatio, "result/gHistRatio.txt");
//		}
			
		//get foreground
		trackRect.x			= inRect.left;
		trackRect.y			= inRect.top;
		trackRect.width		= inRect.right - inRect.left;
		trackRect.height	= inRect.bottom - inRect.top;
		his_CalcForeground2(inImg, gHistRatio, trackRect, gImgBackProj);

		//get the foreground gravity center
		//his_CalcGravityCenter(gImgBackProj, &gGravityCenter);		

		//release memory
		cvReleaseHist(&gHistRatio);
	}
}

///////////////////////////////////////////////////////////////////////////////
void his_TrackNextFrame(IplImage* inImg, RECT inRect, TkResult *outResult)
//move gravity center by histogram ratio
{
	CvRect	trackRect;
	RECT	outerBox;
	float	score=1;
	//int		countPix;

	//check whether input is valid
	if (inImg==NULL) return;
	utl_RectCheckBound(&inRect, inImg->width, inImg->height);

	trackRect.x			= inRect.left;
	trackRect.y			= inRect.top;
	trackRect.width		= inRect.right - inRect.left;
	trackRect.height	= inRect.bottom - inRect.top;

	//get outer ring box
	outerBox.left	= inRect.left-gBorder;
	outerBox.right	= inRect.right+gBorder;
	outerBox.top	= inRect.top-gBorder;
	outerBox.bottom = inRect.bottom +gBorder;
	his_CheckBoxBound(&outerBox, inImg->width, inImg->height);

	//get histogram Model of outerBox
	his_CountBoxHistogram(inImg, outerBox, gHistModel);

	//get Histogram Ratio
	gHistRatio = his_HistogramRatio(gHistInit, gHistModel);	
	gHistDivide = his_HistogramDivide(gHistInit, gHistModel);	
	
	//normalize histogram
	//cvThreshHist(gHistRatio, 60);	
	//his_HistogramNormalize(gHistRatio);
	
	//get back projection image based on ratio histogram	
	his_CalcForeground2(inImg, gHistDivide, trackRect, gImgBackProj);

	//find blobCenter				
	his_BlobCenterShift(gImgBackProj, &inRect, 15);

	//calculate tracking score
	score = his_CountFGPixel(gImgBackProj, inRect);

	//update foreground image for display purpose
	his_CalcForeground2(inImg, gHistRatio, trackRect, gImgBackProj);
	cvThreshold(gImgBackProj, gImgBackProj, 5, 0, CV_THRESH_TOZERO);
	
	//get FG object mask image
	{
		CvRect  roi;	
		cvSetZero(gImgFgMask);	
		cvSetZero(gImgObjMask);	
		roi.x = inRect.left;
		roi.y = inRect.top;
		roi.width = inRect.right-inRect.left;
		roi.height = inRect.bottom-inRect.top;
		cvSetImageROI(gImgBackProj, roi);
		cvSetImageROI(gImgFgMask, roi);		
		cvSetImageROI(inImg, roi);
		cvSetImageROI(gImgObjMask, roi);		
		cvThreshold(gImgBackProj, gImgFgMask, 50, 255, CV_THRESH_BINARY);
		cvCopy(inImg, gImgObjMask, gImgFgMask);
		cvResetImageROI(gImgBackProj);
		cvResetImageROI(gImgFgMask);
		cvResetImageROI(inImg);		
		cvResetImageROI(gImgObjMask);	
	}

	//assign returning results
	outResult->targetBox = inRect;
	outResult->FGImage	= gImgBackProj;
	outResult->FGMask	= gImgFgMask;
	outResult->ObjMask	= gImgObjMask;
	outResult->score	= score;		
	outResult->occlusion = (score<0.5? TRUE:FALSE);	
	
	//clear histogram
	cvReleaseHist(&gHistRatio);
	cvReleaseHist(&gHistDivide);
}

///////////////////////////////////////////////////////////////////////////////
float his_CountFGPixel(IplImage *inImg, RECT inRect)
//cout foreground pixel number to find score
{
	int pixCount;
	float score;
	CvRect crectRoi;	

	//count foreground points
	crectRoi.x = inRect.left;
	crectRoi.y = inRect.top;
	crectRoi.width = inRect.right - inRect.left;
	crectRoi.height = inRect.bottom - inRect.top;
	cvSetImageROI(inImg, crectRoi);
	pixCount = cvCountNonZero(inImg);
	inImg->roi = NULL;
	
	score = (float)pixCount/(crectRoi.width*crectRoi.height);		
	//score = (float)pixCount/gMaskPixCount;
	score = score/gMaskFgRatio;
	score = min(1, score);

	return score;
}

///////////////////////////////////////////////////////////////////////////////
void his_TrackCleanUp()
//release memory
{
	//clear histogram
	cvReleaseHist(&gHistInit);
	cvReleaseHist(&gHistModel);
	cvReleaseHist(&gHistRatio);
	cvReleaseHist(&gHistDivide);

	//projection image
	cvReleaseImage(&gImgBackProj);
	cvReleaseImage(&gImgFgMask);
	cvReleaseImage(&gImgObjMask);
}

