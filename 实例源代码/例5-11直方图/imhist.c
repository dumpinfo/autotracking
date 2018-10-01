#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <ctype.h>

int main( int argc, char** argv )
{
    IplImage *src = 0;
    IplImage *histimg = 0;
    CvHistogram *hist = 0;
    
    int hdims = 50;     // 划分HIST的个数，越高越精确
    float hranges_arr[] = {0,255};
    float* hranges = hranges_arr;
    int bin_w;  
    float max_val;
    int i;
    
    if( argc != 2 || (src=cvLoadImage(argv[1], 0)) == NULL)  // force to gray image
        return -1;
    
    cvNamedWindow( "Histogram", 0 );
    cvNamedWindow( "src", 0);
    
    hist = cvCreateHist( 1, &hdims, CV_HIST_ARRAY, &hranges, 1 );  // 计算直方图
    histimg = cvCreateImage( cvSize(320,200), 8, 3 );
    cvZero( histimg );
    cvCalcHist( &src, hist, 0, 0 ); // 计算直方图
    cvGetMinMaxHistValue( hist, 0, &max_val, 0, 0 );  // 只找最大值
cvConvertScale( hist->bins, 
hist->bins, max_val ? 255. / max_val : 0., 0 ); // 缩放 bin 到区间 [0,255] 
    cvZero( histimg );
    bin_w = histimg->width / hdims;  // hdims: 条的个数，则 bin_w 为条的宽度
    
    // 画直方图
    for( i = 0; i < hdims; i++ )
    {
        double val = ( cvGetReal1D(hist->bins,i)*histimg->height/255 );
        CvScalar color = CV_RGB(255,255,0); //(hsv2rgb(i*180.f/hdims);
        cvRectangle( histimg, cvPoint(i*bin_w,histimg->height),
            cvPoint((i+1)*bin_w,(int)(histimg->height - val)),
            color, 1, 8, 0 );
    }
    
    cvShowImage( "src", src);
    cvShowImage( "Histogram", histimg );
    cvWaitKey(0);

    cvDestroyWindow("src");
    cvDestroyWindow("Histogram");
    cvReleaseImage( &src );
    cvReleaseImage( &histimg );
    cvReleaseHist ( &hist );
    
    return 0;
}
