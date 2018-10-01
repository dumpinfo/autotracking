/* 	程序名：invImage.c
功能：读入图像文件，做图像反转，然后显示图像在屏幕上
*/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>

int main(int argc, char *argv[])
{
  	IplImage* img = 0; 
  	int height,width,step,channels;
  	uchar *data;
 	int i,j,k;
  	
	if(argc<2)
	{
    	printf("Usage: main <image-file-name>\n\7");
    	exit(0);
  	}
  	
	// 载入图像  
  	img=cvLoadImage(argv[1],-1);
  	if(!img)
	{
   		printf("Could not load image file: %s\n",argv[1]);
    	exit(0);
  	}
  	
	// 获取图像信息
  	height    = img->height;  
  	width     = img->width;	
  	step      = img->widthStep;	
  	channels  = img->nChannels;
  	data      = (uchar *)img->imageData;
  	printf("Processing a %dx%d image with %d channels\n",height,width,channels); 
  	// 创建窗口
  	cvNamedWindow("mainWin", CV_WINDOW_AUTOSIZE); 
  	cvMoveWindow("mainWin", 600, 600);
  	// 反转图像
  	for(i=0;i<height;i++) 
		for(j=0;j<width;j++) 
			for(k=0;k<channels;k++)
    	data[i*step+j*channels+k]=100-data[i*step+j*channels+k];
  	// 显示图像
  	cvShowImage("mainWin", img );
	cvWaitKey(0);
  	cvReleaseImage(&img );
  	return 0;
}
