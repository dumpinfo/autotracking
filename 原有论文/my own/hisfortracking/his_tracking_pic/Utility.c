 /*****************************************************************************
 * File:	utiltiy.c
 * Desc:	useful utility fuctions
 * Author:	Xuhui Zhou  @ Carnegie Mellon University
 * Date:	Sep 22, 2003
 *****************************************************************************/
#include "utility.h"

/*****************************************************************************
 * Name: fn_Round
 *****************************************************************************/
double fn_Round(double input){
	double output, remain;

	output = floor(input);
	remain = input - output;
	if (remain>=0.5) output++;
	
	return((int)output);
}

/* find kth smallest of n double array.
 * N.B. array is rearranged IN PLACE.
 */
double fn_kthSmallest(double a[], int n, int k)
{
    int i,j,l,m ;
    double t, x ;

    l=0 ; m=n-1 ;
    while (l<m) {
        x=a[k] ;
        i=l ;
        j=m ;
        do {
            while (a[i]<x) i++ ;
            while (x<a[j]) j-- ;
            if (i<=j) {
		t=a[i]; a[i]=a[j]; a[j]=t;
                i++ ; j-- ;
            }
        } while (i<=j) ;
        if (j<k) l=i ;
        if (k<i) m=j ;
    }
    return a[k] ;
}


/* find median of n double array.
 * N.B. array is rearranged IN PLACE.
 */
double fn_median(double a[], int n)
{
	return (fn_kthSmallest (a, n, n/2));
}


/*****************************************************************************
 * Name: fn_Abs                                                           *
 *****************************************************************************/
double fn_Abs(double input){
	if (input<0)
		input = - input;
	
	return(input);
}

/*****************************************************************************
 * Name: fn_DoubleToStr                                                           *
 *****************************************************************************/
char* fn_DoubleToStr(double input, int precision){
	int length;
	char *strTemp; 
	//char strOutput[100];
	int decimal, sign, i;
	
	strTemp = fcvt(input, precision, &decimal, &sign);
	length = (int)strlen(strTemp);
	if (decimal == 0){
		for(i=length;i>=0;i--){
			strTemp[i+2] = strTemp[i];
		}
		strTemp[1] = '.';
		strTemp[0] = '0';
	}
	if (decimal == -1){
		for(i=length;i>=0;i--){
			strTemp[i+3] = strTemp[i];
		}
		strTemp[2] = '0';
		strTemp[1] = '.';
		strTemp[0] = '0';
	}
	if (decimal >0)
	{
		for(i=length;i>=decimal;i--){
			strTemp[i+1] = strTemp[i];
		}
		strTemp[decimal] = '.';
	}

	length = (int)strlen(strTemp);
	if (sign==1){ //negative
		for(i=length; i>=0; i--){
			strTemp[i+1] = strTemp[i];
		}
		strTemp[0] = '-';
	}	

//	strcpy(strOutput, strTemp);
//	return (strOutput);

	return(strTemp);
}

/*****************************************************************************
 * Name: fn_DoubleToStr2                                                           *
 *****************************************************************************/
void fn_DoubleToStr2(double input, int precision, char* strOutput)
{
	int length;
	char *strTemp; 
	int decimal, sign, i;	
		
	strTemp = fcvt(input, precision, &decimal, &sign);
	length = (int)strlen(strTemp);
	if (decimal == 0){
		for(i=length;i>=0;i--){
			strTemp[i+2] = strTemp[i];
		}
		strTemp[1] = '.';
		strTemp[0] = '0';
	}
	else if (decimal <0){
		decimal = -decimal;
		//shift string for 2+decimal times. 2 stands for '0.' in '0.xxx'
		for(i=length;i>=0;i--){
			strTemp[i+2+decimal] = strTemp[i];
		}
		//pad 0 for |decimal| times
		for(i=2;i<2+decimal;i++){		
			strTemp[i] = '0';
		}		
		strTemp[1] = '.';
		strTemp[0] = '0';
	}
	else //(decimal >0)
	{
		//shift one position for '.' in 'xxx.xxx'
		for(i=length;i>=decimal;i--){
			strTemp[i+1] = strTemp[i];
		}
		strTemp[decimal] = '.';
	}

	length = (int)strlen(strTemp);
	if (sign==1){ //negative
		for(i=length; i>=0; i--){
			strTemp[i+1] = strTemp[i];
		}
		strTemp[0] = '-';
	}	

	strcpy(strOutput, strTemp);
//	return (strOutput);

//	return(strTemp);
}

///////////////////////////////////////////////////////////////////////////////
void utl_RectZoom(RECT *ioRect, float inZoomFactor, int imgWidth, int imgHeight)
//scale a rect by a zoom factor
{
	int width;
	int height;
	int halfDx;
	int halfDy;

	if (inZoomFactor==1) return;

	//get rect dimension
	width = ioRect->right - ioRect->left;
	height = ioRect->bottom - ioRect->top;

	//get zoom difference
	halfDx = (int)fn_Round(width*(inZoomFactor-1)/2);
	halfDy = (int)fn_Round(height*(inZoomFactor-1)/2);

	//get out Rect
	ioRect->left = ioRect->left - halfDx;
	ioRect->right = ioRect->right + halfDx;
	ioRect->top = ioRect->top - halfDy;
	ioRect->bottom = ioRect->bottom + halfDy;

	//check boundary
	if (inZoomFactor>1)
	{
		utl_RectCheckBound(ioRect, imgWidth, imgHeight);
	}
}

///////////////////////////////////////////////////////////////////////////////
void utl_RectSizeAdjust(RECT *ioRect, int rectWidth, int rectHeight, int imgWidth, int imgHeight)
//rect at the same center with different size
{
	int center_x, center_y;
	int halfWid;
	int halfHgt;

	center_x = (ioRect->left+ioRect->right)/2;
	center_y = (ioRect->top+ioRect->bottom)/2;
	halfWid = rectWidth/2;
	halfHgt = rectHeight/2;

	//get out Rect
	ioRect->left	= center_x - halfWid;
	ioRect->right	= ioRect->left + rectWidth;
	ioRect->top		= center_y - halfHgt;
	ioRect->bottom	= ioRect->top + rectHeight;

	//check boundary
	utl_RectCheckBound(ioRect, imgWidth, imgHeight);
}

///////////////////////////////////////////////////////////////////////////////
void utl_RectSizeIncrease(RECT *ioRect, int border, int imgWidth, int imgHeight)
//rect at the same center with different size
{
	int center_x, center_y;
	int halfWid;
	int halfHgt;

	center_x = (ioRect->left+ioRect->right)/2;
	center_y = (ioRect->top+ioRect->bottom)/2;
	halfWid = (ioRect->right-ioRect->left)/2 + border;
	halfHgt = (ioRect->bottom-ioRect->top)/2 + border;

	//get out Rect
	ioRect->left	= center_x - halfWid;
	ioRect->right	= center_x + halfWid;
	ioRect->top		= center_y - halfHgt;
	ioRect->bottom	= center_y + halfHgt;

	//check boundary
	utl_RectCheckBound(ioRect, imgWidth, imgHeight);
}

/******************************************************************************/
void utl_RectCheckBound(RECT *ioRect, int imgWidth, int imgHeight)
//check the boundary limit of RECT
{
	if (ioRect->left <0)			ioRect->left=0;
	if (ioRect->left >imgWidth-1)	ioRect->left = imgWidth-1;
	if (ioRect->right <0)			ioRect->right=0;
	if (ioRect->right >imgWidth-1)	ioRect->right = imgWidth-1;

	if (ioRect->top <0)				ioRect->top=0;
	if (ioRect->top >imgHeight-1)	ioRect->top = imgHeight-1;
	if (ioRect->bottom <0)			ioRect->bottom=0;
	if (ioRect->bottom >imgHeight-1) ioRect->bottom = imgHeight-1;

	if (ioRect->right<=ioRect->left) ioRect->right = ioRect->left+1;
	if (ioRect->bottom<=ioRect->top) ioRect->bottom = ioRect->top+1;
}

///////////////////////////////////////////////////////////////////////////////
void utl_PixelCheckBound(int *pixelX, int *pixelY, int imgWidth, int imgHeight)
//check the boundary limit of pixel
{
	//check x bound
	*pixelX = max(*pixelX, 0);
	*pixelX = min(*pixelX, imgWidth-1);

	//check y bound
	*pixelY = max(*pixelY, 0);
	*pixelY = min(*pixelY, imgHeight-1);
}

///////////////////////////////////////////////////////////////////////////////
void utl_PixelCheckBound_Float(float *pixelX, float *pixelY, int imgWidth, int imgHeight)
//check the boundary limit of pixel
{
	//check x bound
	*pixelX = max(*pixelX, 0);
	*pixelX = min(*pixelX, imgWidth-1);

	//check y bound
	*pixelY = max(*pixelY, 0);
	*pixelY = min(*pixelY, imgHeight-1);
}

///////////////////////////////////////////////////////////////////////////////
void utl_IndexNewDir(char *ioNewDirPath)
//increase index for new dir in case conflict with existing. If dir exists, increase index by 1
{
	char dirCandidate[MAX_PATH];
	char sBufferPath[MAX_PATH];
	int i;
	char strIndex[10];
	int existFlag;

	strcpy(dirCandidate, ioNewDirPath);
	strcat(dirCandidate, "_");			

	//add index
	i=1;
	do{	
		strcpy(sBufferPath, dirCandidate);	
		itoa(i, strIndex, 10);
		strcat(sBufferPath, strIndex);	
		existFlag = _chdir(sBufferPath);
		i++;
	}while( existFlag == 0);

	//return dir path and name			
	strcpy(ioNewDirPath, sBufferPath);
}

///////////////////////////////////////////////////////////////////////////////
int utl_Min3(int a, int b, int c)
//get min among 3 numbers
{
	int minval;

	minval = min(a, b);
	minval = min(minval, c);	

	return minval;
}

///////////////////////////////////////////////////////////////////////////////
int utl_Max3(int a, int b, int c)
//get max among 3 numbers
{
	int maxval;

	maxval = max(a, b);
	maxval = max(maxval, c);	

	return maxval;
}

///////////////////////////////////////////////////////////////////////////////
void utl_WriteImage(IplImage *imgInput, char* filename)
//write single channel image value to a txt file
{
	FILE *file;
	int row,col;

	file = fopen(filename, "w");		
	
	for (row=0;row<imgInput->height;row++){
		for(col=0;col<imgInput->width;col++){
			fprintf(file, "%.2f ", cvGetReal2D(imgInput, row, col));
		}			
		fprintf(file, "\n");
	}
	fclose(file);
}

///////////////////////////////////////////////////////////////////////////////
void utl_DrawLine(IplImage *imgInput, POINT pt1, POINT pt2, CvScalar pixval)
//write single channel image value to a txt file
{
	int xi;
	int yi;
	int yii;
	int j;
	//check boundary
//	if (pt1.x<0||pt1.x>=imgInput->width||pt1.y<0||pt1.y>=imgInput->height) return;
//	if (pt2.x<0||pt2.x>=imgInput->width||pt2.y<0||pt2.y>=imgInput->height) return;
	if (pt2.x == pt1.x) return;

	//exchange pt1, pt2
	if (pt2.x < pt1.x){
		xi = pt1.x;
		yi = pt1.y;
		pt1.x = pt2.x;
		pt1.y = pt2.y;
		pt2.x = xi;
		pt2.y = yi;
	}
	
	//get linear points (y-y1)/(x-x1) = (y2-y1)/(x2-x1);
	for(xi=pt1.x; xi<=pt2.x; xi++){
		yi = (int)(pt1.y + (xi-pt1.x)*(pt2.y-pt1.y)/(pt2.x-pt1.x));
		for(j=-1;j<=1;j++){
			yii = yi+j;
			if (xi<0||xi>=imgInput->width||yii<0||yii>=imgInput->height) continue;
				cvSet2D(imgInput, yii, xi, pixval);
		}
	}
}
/* Generate a normal random variable with mean 0 and standard deviation
of 1. To adjust to some other distribution, multiply by the standard
deviation and add the mean. Box-Muller method
note: rand() is a function that returns a uniformly distributed random
number from 0 to RAND_MAX
*/
double gaussrand()
{
	static double V2, fac;
	static int phase = 0;
	double S, Z, U1, U2, V1;

	if (phase)
		Z = V2 * fac;
	else
	{
		do {
			U1 = (double)rand() / RAND_MAX;
			U2 = (double)rand() / RAND_MAX;


			V1 = 2 * U1 - 1;
			V2 = 2 * U2 - 1;
			S = V1 * V1 + V2 * V2;
		} while(S >= 1);


		fac = sqrt (-2 * log(S) / S);
		Z = V1 * fac;
	}

    phase = 1 - phase;

    return Z;
}

/////////////////////////////////////////////////////////////////////////////
void utl_WriteBoxToImage(IplImage *imgC3, RECT box, CvScalar featureColor, int thick)
{
	CvPoint pt1;
	CvPoint pt2;

	//add key point box on input image		
	pt1.x = box.left;
	pt1.y = box.top;
	pt2.x = box.right;
	pt2.y = box.bottom;
	cvRectangle(imgC3, pt1, pt2, featureColor,thick,0,0);    
}
/////////////////////////////////////////////////////////////////////////////
void utl_WriteBoxToImage_ctr(IplImage *imgC3, int centerx, int centery, int halfWid, CvScalar featureColor, int thick)
{
	CvPoint pt1;
	CvPoint pt2;

	//add key point box on input image		
	pt1.x = centerx-halfWid;
	pt1.y = centery-halfWid;
	pt2.x = centerx+halfWid;
	pt2.y = centery+halfWid;
	cvRectangle(imgC3, pt1, pt2, featureColor,thick,0,0);    
}