/* 	程序名：convert.c
功能：图像格式的转换
*/
#include <cv.h>
#include <highgui.h>
#include <stdio.h>
int main( int argc, char** argv )
{ 
	IplImage* src;
 
	// -1:  the loaded image will be loaded as is (with number of channels depends on the file).
	if(argc != 3)
	{
	    printf("CONV: Image format convertion, support JPG,BMP,TIF,PNG,PPM\n");
	    printf("Usage: conv srcImage dstImage\n");
	    return 0;
	}
	if( (  strstr(argv[1],".jpg")==NULL    
		&& strstr(argv[1],".bmp")==NULL 
		&& strstr(argv[1],".tif")==NULL 
		&& strstr(argv[1],".png")==NULL 
		&& strstr(argv[1],".ppm")==NULL  )
		|| (  strstr(argv[2],".jpg")==NULL 
		&& strstr(argv[2],".bmp")==NULL 
		&& strstr(argv[2],".tif")==NULL 
		&& strstr(argv[2],".png")==NULL 
		&& strstr(argv[2],".ppm")==NULL  ))
	{
		printf("WARNING: CONV only support JPG,BMP,TIF,PPM,TGA and PPM\n");
	}
	else {
		if( (src=cvLoadImage(argv[1], -1))!= 0 ) {
	        cvSaveImage( argv[2], src);
            cvReleaseImage(&src);
            printf("\n Convert successfully.\n");
		}
		else
		{
		   	printf("\n*** Read or write image fails *** \n");
		}
}
return 0;
}
