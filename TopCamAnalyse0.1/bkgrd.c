#include <stdio.h>
#include <cv.h>
#include <highgui.h>

int main( int argc, char** argv )
{
  //����IplImageָ��
  IplImage* pFrame = NULL;		//ÿһ֡ͼ���ָ��
  IplImage* pFrImg = NULL;		//
  IplImage* pBkImg = NULL;		//

  CvMat* pFrameMat = NULL;
  CvMat* pFrMat = NULL;
  CvMat* pBkMat = NULL;
  
  CvCapture* pCapture = NULL;
  
  int nFrmNum = 0;

  //�������
  CvMemStorage* storage = cvCreateMemStorage(0);
  CvSeq* contour = 0;
  IplImage* dst = 0;
  int		total = 0;
  int		eachtotal = 0;

  //��������
  cvNamedWindow("video", 1);
  cvNamedWindow("background",1);
  cvNamedWindow("foreground",1);
  //ʹ������������
  cvMoveWindow("video", 30, 0);
  cvMoveWindow("background", 360, 0);
  cvMoveWindow("foreground", 690, 0);

  if( argc != 2 )
    {
      fprintf(stderr, "Usage: bkgrd <video_file_name>\n");
      return -1;
    }

  //����Ƶ�ļ�
  if( !(pCapture = cvCaptureFromFile(argv[1])))
    {
      fprintf(stderr, "Can not open video file %s\n", argv[1]);
      return -2;
    }
	pFrame = cvQueryFrame( pCapture );
	pFrImg = cvCreateImage(cvSize(pFrame->width, pFrame->height), IPL_DEPTH_8U,1);
	dst = cvCreateImage( cvGetSize(pFrImg), 8, 3 );
  
  //��֡��ȡ��Ƶ
  	while(pFrame = cvQueryFrame( pCapture ))
    {
      	nFrmNum++;
      
      	//����ǵ�һ֡����Ҫ�����ڴ棬����ʼ��
      	if(nFrmNum == 1)
		{
	  		pBkImg = cvCreateImage(cvSize(pFrame->width, pFrame->height), IPL_DEPTH_8U,1);
	  		pFrImg = cvCreateImage(cvSize(pFrame->width, pFrame->height), IPL_DEPTH_8U,1);

			pBkMat = cvCreateMat(pFrame->height, pFrame->width, CV_32FC1);
	  		pFrMat = cvCreateMat(pFrame->height, pFrame->width, CV_32FC1);
	  		pFrameMat = cvCreateMat(pFrame->height, pFrame->width, CV_32FC1);

	  		//ת���ɵ�ͨ��ͼ���ٴ���
	  		cvCvtColor(pFrame, pBkImg, CV_BGR2GRAY);
	  		cvCvtColor(pFrame, pFrImg, CV_BGR2GRAY);

	  		cvConvert(pFrImg, pFrameMat);
	  		cvConvert(pFrImg, pFrMat);
	  		cvConvert(pFrImg, pBkMat);
		}
      	else
		{
			//ÿһ֡ͼ��ת���ɵ�ͨ��ͼ��
	  		cvCvtColor(pFrame, pFrImg, CV_BGR2GRAY);
	  		cvConvert(pFrImg, pFrameMat);
	  		//������˹�˲�����ƽ��ͼ��
//	  		cvSmooth(pFrameMat, pFrameMat, CV_GAUSSIAN, 3, 0, 0);
	  
	  		//��ǰ֡������ͼ���
	  		cvAbsDiff(pFrameMat, pBkMat, pFrMat);

	  		//��ֵ��ǰ��ͼ
	  		cvThreshold(pFrMat, pFrImg, 60, 255.0, CV_THRESH_BINARY);

	  		//������̬ѧ�˲���ȥ������  
//	 		cvErode(pFrImg, pFrImg, 0, 1);
//	  		cvDilate(pFrImg, pFrImg, 0, 1);

	  		//���±���
	  		cvRunningAvg(pFrameMat, pBkMat, 0.003, 0);
	  		//������ת��Ϊͼ���ʽ��������ʾ
	  		cvConvert(pBkMat, pBkImg);

			//Ѱ������
			eachtotal = cvFindContours( pFrImg, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));
			cvZero( dst );

			if(contour != 0)
				printf("%d\n", total += eachtotal);
			
			for( ; contour != 0; contour = contour->h_next )
			{
			    CvScalar color = CV_RGB( rand()&255, rand()&255, rand()&255 );
			    /* ��1��� CV_FILLED  ��ָʾ���������� */
			    cvDrawContours( dst, contour, color, color, -1, CV_FILLED, 8 ,cvPoint(0, 0));
			}
	  		
			//��ʾͼ��
	  		cvShowImage("video", pFrame);
	  		cvShowImage("background", dst);
	  		cvShowImage("foreground", pFrImg);
			
	  		//����а����¼���������ѭ��
	  		//�˵ȴ�ҲΪcvShowImage�����ṩʱ�������ʾ
	  		//�ȴ�ʱ����Ը���CPU�ٶȵ���
	  		if( cvWaitKey(2) >= 0 )
	    		break;
		}  // end of if-else
    } // end of while-loop

  	//���ٴ���
  	cvDestroyWindow("video");
  	cvDestroyWindow("background");
  	cvDestroyWindow("foreground");

  	//�ͷ�ͼ��;���
  	cvReleaseImage(&pFrImg);
  	cvReleaseImage(&pBkImg);

  	cvReleaseMat(&pFrameMat);
  	cvReleaseMat(&pFrMat);
  	cvReleaseMat(&pBkMat);

  	return 0;
}
