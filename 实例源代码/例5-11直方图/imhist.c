#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <ctype.h>

int main( int argc, char** argv )
{
    IplImage *src = 0;
    IplImage *histimg = 0;
    CvHistogram *hist = 0;
    
    int hdims = 50;     // ����HIST�ĸ�����Խ��Խ��ȷ
    float hranges_arr[] = {0,255};
    float* hranges = hranges_arr;
    int bin_w;  
    float max_val;
    int i;
    
    if( argc != 2 || (src=cvLoadImage(argv[1], 0)) == NULL)  // force to gray image
        return -1;
    
    cvNamedWindow( "Histogram", 0 );
    cvNamedWindow( "src", 0);
    
    hist = cvCreateHist( 1, &hdims, CV_HIST_ARRAY, &hranges, 1 );  // ����ֱ��ͼ
    histimg = cvCreateImage( cvSize(320,200), 8, 3 );
    cvZero( histimg );
    cvCalcHist( &src, hist, 0, 0 ); // ����ֱ��ͼ
    cvGetMinMaxHistValue( hist, 0, &max_val, 0, 0 );  // ֻ�����ֵ
cvConvertScale( hist->bins, 
hist->bins, max_val ? 255. / max_val : 0., 0 ); // ���� bin ������ [0,255] 
    cvZero( histimg );
    bin_w = histimg->width / hdims;  // hdims: ���ĸ������� bin_w Ϊ���Ŀ��
    
    // ��ֱ��ͼ
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
