#include "cv.h"
#include "highgui.h"
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <stdio.h>

// various tracking parameters (in seconds)
const double MHI_DURATION = 0.5;
const double MAX_TIME_DELTA = 0.5;
const double MIN_TIME_DELTA = 0.05;
// �����˶�����ѭ��֡����������ٶ��Լ�FPS�����й�
const int N = 2;

// ring image buffer
IplImage **buf = 0;
int last = 0;

// temporary images
IplImage *mhi = 0; // MHI: motion history image
IplImage *orient = 0; // orientation
IplImage *mask = 0; // valid orientation mask
IplImage *segmask = 0; // motion segmentation map
CvMemStorage* storage = 0; // temporary storage

// parameters:
//  img - input video frame
//  dst - resultant motion picture
//  args - optional parameters
void  update_mhi( IplImage* img, IplImage* dst, int diff_threshold )
{
    double timestamp = clock()/1000.; // get current time in seconds
    CvSize size = cvSize(img->width,img->height); // get current frame size
    int i, idx1 = last, idx2;
    IplImage* silh;
    CvSeq* seq;
    CvRect comp_rect;
    double count;
    double angle;
    CvPoint center;
    double magnitude;          
    CvScalar color;

    // allocate images at the beginning or
    // reallocate them if the frame size is changed
    if( !mhi || mhi->width != size.width || mhi->height != size.height ) 
    {
        if( buf == 0 ) 
        {
            buf = (IplImage**)malloc(N*sizeof(buf[0]));
            memset( buf, 0, N*sizeof(buf[0]));
        }
        
        for( i = 0; i < N; i++ ) 
        {
            cvReleaseImage( &buf[i] );
            buf[i] = cvCreateImage( size, IPL_DEPTH_8U, 1 );
            cvZero( buf[i] );
        }
        cvReleaseImage( &mhi );
        cvReleaseImage( &orient );
        cvReleaseImage( &segmask );
        cvReleaseImage( &mask );
        
        mhi = cvCreateImage( size, IPL_DEPTH_32F, 1 );
        cvZero( mhi ); // clear MHI at the beginning
        orient = cvCreateImage( size, IPL_DEPTH_32F, 1 );
        segmask = cvCreateImage( size, IPL_DEPTH_32F, 1 );
        mask = cvCreateImage( size, IPL_DEPTH_8U, 1 );
    }

    cvCvtColor( img, buf[last], CV_BGR2GRAY ); // convert frame to grayscale

    idx2 = (last + 1) % N; // index of (last - (N-1))th frame
    last = idx2;

    silh = buf[idx2];
    // ������֡�Ĳ�
    cvAbsDiff( buf[idx1], buf[idx2], silh ); // get difference between frames
    
    // �Բ�ͼ������ֵ��
    cvThreshold( silh, silh, diff_threshold, 1, CV_THRESH_BINARY ); // and threshold it
    cvUpdateMotionHistory( silh, mhi, timestamp, MHI_DURATION ); // update MHI

    // convert MHI to blue 8u image
    // cvCvtScale�ĵ��ĸ����� shift = (MHI_DURATION - timestamp)*255./MHI_DURATION
    // ����֡�����ʧ����
cvCvtScale( mhi, mask, 255./MHI_DURATION,
 (MHI_DURATION - timestamp)*255./MHI_DURATION );

    cvZero( dst );
    cvCvtPlaneToPix(mask, 0, 0, 0, dst );  // B,G,R,0 ��> dist : convert to BLUE image

    // �����˶����ݶȷ����Լ���ȷ�ķ�����ģmask
    // Filter size = 3
cvCalcMotionGradient( mhi, mask, orient, 
MAX_TIME_DELTA, MIN_TIME_DELTA, 3 ); 
    
    if( !storage )
        storage = cvCreateMemStorage(0);
    else
        cvClearMemStorage(storage);
    
    // �˶��ָ�: ����˶���������������
    // segmask is marked motion components map. It is not used further
    seq = cvSegmentMotion( mhi, segmask, storage, timestamp, MAX_TIME_DELTA );

    // iterate through the motion components,
    // One more iteration (i == -1) corresponds to the whole image (global motion)
    for( i = 0; i < seq->total; i++ ) 
    {

        if( i < 0 ) { // case of the whole image��������ͼ��������
            comp_rect = cvRect( 0, 0, size.width, size.height );
            color = CV_RGB(255,255,255);  // white color
            magnitude = 100;  // ���߳����Լ�Բ�뾶�Ĵ�С����
        }
        else { // i-th motion component
            comp_rect = ((CvConnectedComp*)cvGetSeqElem( seq, i ))->rect;
// ȥ��С�Ĳ���
if( comp_rect.width + comp_rect.height < 100 ) 
                continue;
            color = CV_RGB(255,0,0);    // red color
            magnitude = 30;
            //if(seq->total > 0) MessageBox(NULL,"Motion Detected",NULL,0);
        }

        // select component ROI
        cvSetImageROI( silh, comp_rect );
        cvSetImageROI( mhi, comp_rect );
        cvSetImageROI( orient, comp_rect );
        cvSetImageROI( mask, comp_rect );

        // ��ѡ���������, �����˶�����
        angle = cvCalcGlobalOrientation( orient, mask, mhi, timestamp,
 MHI_DURATION);
        angle = 360.0 - angle;  // adjust for images with top-left origin

        // �������ڼ������
        // Norm(L1) = sum of total pixel values
        count = cvNorm( silh, 0, CV_L1, 0 ); 

        // The function cvResetImageROI releases image ROI
        cvResetImageROI( mhi );
        cvResetImageROI( orient );
        cvResetImageROI( mask );
        cvResetImageROI( silh );

        // check for the case of little motion
        if( count < comp_rect.width*comp_rect.height * 0.05 )  // five percent of pixel
            continue;

        // draw a clock with arrow indicating the direction
        center = cvPoint( (comp_rect.x + comp_rect.width/2),
                          (comp_rect.y + comp_rect.height/2) );

        cvCircle( dst, center, cvRound(magnitude*1.2), color, 3, CV_AA, 0 );
        cvLine( dst, center, cvPoint( cvRound( center.x + 
magnitude*cos(angle*CV_PI/180)),
                	cvRound( center.y - magnitude*sin(angle*CV_PI/180))), 
color, 3, CV_AA, 0 );
    }
}

int main(int argc, char** argv)
{
    IplImage* motion = 0;
    CvCapture* capture = 0;
    
    if( argc == 1 || (argc == 2 && strlen(argv[1]) == 1 && isdigit(argv[1][0])))
        capture = cvCaptureFromCAM( argc == 2 ? argv[1][0] - '0' : 0 );
    else if( argc == 2 )
        capture = cvCaptureFromAVI( argv[1] ); 

    if( capture )
    {
        cvNamedWindow( "Motion", 1 );
        for(;;)
        {
            IplImage* image;
            if( !cvGrabFrame( capture ))
                break;
            image = cvRetrieveFrame( capture );

            if( image )
            {
                if( !motion )
                {
motion = cvCreateImage( cvSize(image->width,image->height), 
8, 3 );
                    cvZero( motion );
                    motion->origin = image->origin;
                }
            }

            update_mhi( image, motion, 60 );
            cvShowImage( "Motion", motion );

            if( cvWaitKey(10) >= 0 )
                break;
        }
        cvReleaseCapture( &capture );
        cvDestroyWindow( "Motion" );
    }

    return 0;
}
