#include "cv.h"
#include "highgui.h"

char wndname[] = "Edge";
char tbarname[] = "Threshold";
int edge_thresh = 1;

IplImage *image = 0, *cedge = 0, *gray = 0, *edge = 0;

// 定义跟踪条的 callback 函数
void on_trackbar(int h)
{
    cvSmooth( gray, edge, CV_BLUR, 3, 3, 0 );
    cvNot( gray, edge );

    // 对灰度图像进行边缘检测
    cvCanny(gray, edge, (float)edge_thresh, (float)edge_thresh*3, 3);
    cvZero( cedge );
    // copy edge points
    cvCopy( image, cedge, edge );
    // 显示图像
    cvShowImage(wndname, cedge);
}

int main( int argc, char** argv )
{
    char* filename = argc == 2 ? argv[1] : (char*)"fruits.jpg";
    
    if( (image = cvLoadImage( filename, 1)) == 0 )
        return -1;

    // Create the output image
    cedge = cvCreateImage(cvSize(image->width,image->height), IPL_DEPTH_8U, 3);

    // 将彩色图像转换为灰度图像
    gray = cvCreateImage(cvSize(image->width,image->height), IPL_DEPTH_8U, 1);
    edge = cvCreateImage(cvSize(image->width,image->height), IPL_DEPTH_8U, 1);
    cvCvtColor(image, gray, CV_BGR2GRAY);

    // Create a window
    cvNamedWindow(wndname, 1);

    // create a toolbar 
    cvCreateTrackbar(tbarname, wndname, &edge_thresh, 100, on_trackbar);

    // Show the image
    on_trackbar(0);

    // Wait for a key stroke; the same function arranges events processing
    cvWaitKey(0);
    cvReleaseImage(&image);
    cvReleaseImage(&gray);
    cvReleaseImage(&edge);
    cvDestroyWindow(wndname);

    return 0;
}
