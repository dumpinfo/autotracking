#include "cv.h"
#include "highgui.h"

#define HDIM    256    // bin of HIST, default = 256

int main( int argc, char** argv ) 
{
    IplImage *src = 0, *dst = 0;
    CvHistogram *hist = 0;
    
    int n = HDIM;     
    double nn[HDIM];
    uchar T[HDIM];
    CvMat *T_mat;
    
    int x;
    int sum = 0; // sum of pixels of the source image ͼ�������ص���ܺ�
    double val = 0;
    
    if( argc != 2 || (src=cvLoadImage(argv[1], 0)) == NULL)  // force to gray image
        return -1;
    
    cvNamedWindow( "source", 1 );
    cvNamedWindow( "result", 1 );
    
    // ����ֱ��ͼ
    hist = cvCreateHist( 1, &n, CV_HIST_ARRAY, 0, 1 );  
    cvCalcHist( &src, hist, 0, 0 ); 
    
    // Create Accumulative Distribute Function of histgram
    val = 0;
    for ( x = 0; x < n; x++)
    {
        val = val + cvGetReal1D (hist->bins, x);
        nn[x] = val;
    }

    // ��һ��ֱ��ͼ
    sum = src->height * src->width;
    for( x = 0; x < n; x++ )
    {
        T[x] = (uchar) (255 * nn[x] / sum); // range is [0,255]
    }

    // Using look-up table to perform intensity transform for source image 
    dst = cvCloneImage( src );
    T_mat = cvCreateMatHeader( 1, 256, CV_8UC1 );
    cvSetData( T_mat, T, 0 );    
    // ֱ�ӵ����ڲ�������� look-up-table �Ĺ���
    cvLUT( src, dst, T_mat ); 

    cvShowImage( "source", src );
    cvShowImage( "result", dst );
    cvWaitKey(0);

    cvDestroyWindow("source");
    cvDestroyWindow("result");
    cvReleaseImage( &src );
    cvReleaseImage( &dst );
    cvReleaseHist ( &hist );
    
    return 0;
}
