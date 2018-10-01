/*
程序名称：laplace.c
功能：从摄像头或者AVI文件中得到视频流，对视频流进行边缘检测，并输出结果。
*/
#include "cv.h"
#include "highgui.h"
#include <ctype.h>
#include <stdio.h>

int main( int argc, char** argv )
{
    IplImage* laplace = 0;
    IplImage* colorlaplace = 0;
    IplImage* planes[3] = { 0, 0, 0 };  // 多个图像面
    CvCapture* capture = 0;
    
    // 下面的语句说明在命令行执行程序时，如果指定AVI文件，那么处理从
	// AVI文件读取的视频流，如果不指定输入变量，那么处理从摄像头获取
	// 的视频流
	if( argc == 1 || (argc == 2 && strlen(argv[1]) == 1 && isdigit(argv[1][0])))
        capture = cvCaptureFromCAM( argc == 2 ? argv[1][0] - '0' : 0 );
    else if( argc == 2 )
        capture = cvCaptureFromAVI( argv[1] ); 

    if( !capture )
    {
        fprintf(stderr,"Could not initialize capturing...\n");
        return -1;
    }
        
    cvNamedWindow( "Laplacian", 0 );

	// 循环捕捉，直到用户按键跳出循环体
    for(;;)
    {
        IplImage* frame = 0;
        int i;

        frame = cvQueryFrame( capture );
        if( !frame )
            break;

        if( !laplace )
        {
            for( i = 0; i < 3; i++ )
                planes[i] = cvCreateImage( cvSize(frame->width,frame->height), 8, 1 );
			laplace = cvCreateImage( cvSize(frame->width,frame->height),IPL_DEPTH_16S, 1 );
            colorlaplace = cvCreateImage( cvSize(frame->width,frame->height), 8, 3 );
        }
        
		cvCvtPixToPlane( frame, planes[0], planes[1], planes[2], 0 );
        for( i = 0; i < 3; i++ )
        {
            cvLaplace( planes[i], laplace, 3 ); // 3: aperture_size
            cvConvertScaleAbs( laplace, planes[i], 1, 0 );  // planes[] = ABS(laplace)
        }
        cvCvtPlaneToPix( planes[0], planes[1], planes[2], 0, colorlaplace );
        colorlaplace->origin = frame->origin;

        cvShowImage("Laplacian", colorlaplace );

        if( cvWaitKey(10) >= 0 )
            break;
    }
    cvReleaseCapture( &capture );
    cvDestroyWindow("Laplacian");
    return 0;
}
