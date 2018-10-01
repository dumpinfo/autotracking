/*
�������ƣ�laplace.c
���ܣ�������ͷ����AVI�ļ��еõ���Ƶ��������Ƶ�����б�Ե��⣬����������
*/
#include "cv.h"
#include "highgui.h"
#include <ctype.h>
#include <stdio.h>

int main( int argc, char** argv )
{
    IplImage* laplace = 0;
    IplImage* colorlaplace = 0;
    IplImage* planes[3] = { 0, 0, 0 };  // ���ͼ����
    CvCapture* capture = 0;
    
    // ��������˵����������ִ�г���ʱ�����ָ��AVI�ļ�����ô�����
	// AVI�ļ���ȡ����Ƶ���������ָ�������������ô���������ͷ��ȡ
	// ����Ƶ��
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

	// ѭ����׽��ֱ���û���������ѭ����
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
