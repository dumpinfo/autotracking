#include <stdio.h>
#include "cv.h"
#include "highgui.h"
#define max_corners 100

int main( int argc, char** argv )
{
    int cornerCount=max_corners;
    CvPoint2D32f corners[max_corners];
    IplImage *srcImage = 0, *grayImage = 0, *corners1 = 0, *corners2 = 0;
    int i;
    CvScalar color = CV_RGB(255,0,0);
    char* filename = argc == 2 ? argv[1] : (char*)"..//..//c//pic3.png";  // 注意相对路径
    
        cvNamedWindow( "image", 1 ); // create HighGUI window with name "image"
    
    //Load the image to be processed
    srcImage = cvLoadImage(filename, 1);
    
    grayImage = cvCreateImage(cvGetSize(srcImage), IPL_DEPTH_8U, 1);
    
    //copy the source image to copy image after converting the format
    cvCvtColor(srcImage, grayImage, CV_BGR2GRAY);
    
    //create empty images of same size as the copied images
    corners1= cvCreateImage(cvGetSize(srcImage), IPL_DEPTH_32F, 1);
    corners2= cvCreateImage(cvGetSize(srcImage),IPL_DEPTH_32F, 1);
    
    cvGoodFeaturesToTrack (grayImage, corners1, 
        corners2, corners,
        &cornerCount, 0.05,
        5, 
        0,
        3,  // block size
        0,  // not use harris
        0.4 );
    
    printf("num corners found: %d\n", cornerCount);
    
    // draw circles at each corner location in the gray image and
    //print out a list the corners 
    if(cornerCount>0) 
    {
        for (i=0; i<cornerCount; i++) 
        {
            cvCircle(srcImage, cvPoint((int)(corners[i].x), (int)(corners[i].y)), 6,
                color, 2, CV_AA, 0);
        }
    }
    
    cvShowImage( "image", srcImage ); 
    
    cvReleaseImage(&srcImage);
    cvReleaseImage(&grayImage);
    cvReleaseImage(&corners1);
    cvReleaseImage(&corners2);
    
    cvWaitKey(0); // wait for key. The function has
    return 0;
}
