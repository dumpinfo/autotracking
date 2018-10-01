#include <stdio.h>
#include <cv.h>
#include <highgui.h>

#define SIZEMAX			1000
#define POINTDISTANCE	10
#define OBJECTDISTANCE	(320/5)
#define MAXX			5
#define MAXY			4

int Distance(CvPoint *p1, CvPoint *p2)
{
	return sqrt((p1->x-p2->x)*(p1->x-p2->x) + (p1->y-p2->y)*(p1->y-p2->y));
}

int main( int argc, char** argv )
{
	//����IplImageָ��
	IplImage*	pFrame = NULL;		//ÿһ֡ͼ���ָ��
	IplImage*	pFrImg = NULL;		//
	IplImage*	pBkImg = NULL;		//

	CvMat*	pFrameMat	= NULL;
	CvMat*	pFrMat		= NULL;
	CvMat*	pBkMat		= NULL;
  
	CvCapture* pCapture = NULL;
  
	int nFrmNum = 0;

	//��ǰ��ͼ������ر���
	int i, j, k;
	int num = 0;
	int temp[SIZEMAX];
	uchar* ptr = NULL;

	//�������
	CvMemStorage*	storage			= cvCreateMemStorage(0);
	CvSeq*			contour			= 0;
	int				eachtotal		= 0;
	int				contour_area_tmp= 0;//���
	int				contour_area_max= 0;//������ֵ
	int				lpt				= 0;
	CvSeqReader		reader;
	int				maxx, maxy, minx, miny;
	CvBox2D			track_box;
	int				areaCoutour[SIZEMAX];//�洢�������
	RECT			rectCoutour[SIZEMAX];//�洢��Ӿ���
	CvPoint			p1, p2;

//////////////////////////////////////////////////////////////
	//��������
	cvNamedWindow("video", 1);
	//ʹ������������
	cvMoveWindow("video", 30, 0);

//////////////////////////////////////////////////////////////
	//����Ƶ�ļ�
	if( argc != 2 )
    {
		fprintf(stderr, "Usage: bkgrd <video_file_name>\n");
		return -1;
    }

	if( !(pCapture = cvCaptureFromFile(argv[1])))
    {
		fprintf(stderr, "Can not open video file %s\n", argv[1]);
		return -2;
    }
  
//////////////////////////////////////////////////////////////
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
	  		cvSmooth(pFrameMat, pFrameMat, CV_GAUSSIAN, 3, 0, 0, 0);
	  
	  		//��ǰ֡������ͼ���
	  		cvAbsDiff(pFrameMat, pBkMat, pFrMat);

	  		//��ֵ��ǰ��ͼ
	  		cvThreshold(pFrMat, pFrImg, 60, 255.0, CV_THRESH_BINARY);

	  		//���±���
	  	//	cvRunningAvg(pFrameMat, pBkMat, 0.003, 0);
			cvRunningAvg(pFrameMat, pBkMat, 0.0008, 0);
	  		//������ת��Ϊͼ���ʽ��������ʾ
	  		cvConvert(pBkMat, pBkImg);

/////////////////////////////////////////////////////////////////
			//��ǰ��ͼ����д���
			//����Ϊ˳�����������
			for(i = 0; i < pFrImg->height; i++)
			{
				//ָ��ÿ�е�һ��Ԫ��
				ptr = (uchar*)(pFrImg->imageData + i * pFrImg->widthStep);
				
				for (j = 0; j < pFrImg->width; j++)
				{
					if(ptr[j] == 0xFF)
					{
						temp[num] = j;
						num++;
					}
				}

				//�������ڵ�
				for(k=0; k<num-1; k++)
				{
					if(temp[k+1] - temp[k] < POINTDISTANCE)
					{
						for(j = temp[k]+1; j < temp[k+1]; j++ )
							ptr[j] = 0xFF;
					}
				}
				num = 0;
			}

			//����Ϊ˳�����������
			for(i = 0; i < pFrImg->width; i++)
			{
				//ָ��ÿ�е�һ��Ԫ��
				ptr = (uchar*)(pFrImg->imageData + i);
				
				for (j = 0; j < pFrImg->height; j++)
				{
					if(ptr[pFrImg->widthStep * j] == 0xFF)
					{
						temp[num] = j;
						num++;
					}
				}

				//�������ڵ�
				for(k=0; k<num-1; k++)
				{
					if(temp[k+1] - temp[k] < POINTDISTANCE)
					{
						for(j = temp[k]+1; j < temp[k+1]; j++ )
							ptr[pFrImg->widthStep * j] = 0xFF;
					}
				}
				num = 0;
			}
/////////////////////////////////////////////////////////////////
			//Ѱ������
			num = 0;
			eachtotal = cvFindContours( pFrImg, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));
			
			//������������
			for( ; contour != 0; contour = contour->h_next )
			{
			    //����Ƶ���������
			    cvDrawContours( pFrame, contour, CV_RGB(255, 0, 0), CV_RGB(255, 0, 0), 1, CV_FILLED, 8 ,cvPoint(0, 0));
				//�����������
				contour_area_tmp = (int)fabs(cvContourArea( contour, CV_WHOLE_SEQ ));
				//������������������areCountour��
				areaCoutour[num] = contour_area_tmp;

				//���������������Ӿ���
				lpt = contour->total;
				cvStartReadSeq( contour, &reader, 0 );
				minx = maxx = ((CvPoint*)(reader.ptr))->x;
				miny = maxy = ((CvPoint*)(reader.ptr))->y;

				CV_NEXT_SEQ_ELEM( contour->elem_size, reader );
				while( lpt-- > 0 )
				{
					if(((CvPoint*)(reader.ptr))->x > maxx)
						maxx = ((CvPoint*)(reader.ptr))->x;

					if(((CvPoint*)(reader.ptr))->y > maxy)
						maxy = ((CvPoint*)(reader.ptr))->y;
						
					if(((CvPoint*)(reader.ptr))->x < minx)
						minx = ((CvPoint*)(reader.ptr))->x;

					if(((CvPoint*)(reader.ptr))->y < miny)
						miny = ((CvPoint*)(reader.ptr))->y;

					CV_NEXT_SEQ_ELEM( contour->elem_size, reader );
				}

				//�洢�����Ӿ���
				rectCoutour[num].bottom		=	maxy;
				rectCoutour[num].left		=	minx;
				rectCoutour[num].right		=	maxx;
				rectCoutour[num].top		=	miny;
				num++;
			}
			
			//��С�����ϲ�
			//����������Ϣ
			if (num > 0)
			{
				//�ҵ�����������Ǳ����j��
				j = 0;
				contour_area_max = areaCoutour[j];
				for (i=1; i<num; i++)
				{
					if (areaCoutour[i] > contour_area_max)
					{
						contour_area_max = areaCoutour[i];
						j = i;
					}
				}

				//��rectCounter���ҵ�����С��OBJECTDISTANCE�ġ����Ҹ���rectCounter, ��areaCoutour��-1��ʾɾ��
				p1.x = (rectCoutour[j].right - rectCoutour[j].left)/2;
				p1.y = (rectCoutour[j].bottom - rectCoutour[j].top)/2;
				for (i=0; i<num; i++)
				{
					p2.x = (rectCoutour[i].right - rectCoutour[i].left)/2;
					p2.y = (rectCoutour[i].bottom - rectCoutour[i].top)/2;
					//�������С��OBJECTDISTANCE
					k = Distance(&p1, &p2);
					if (k < OBJECTDISTANCE)
					{
						if (rectCoutour[i].bottom > rectCoutour[j].bottom)
						{
							rectCoutour[j].bottom = rectCoutour[i].bottom;
						}
						if (rectCoutour[i].left < rectCoutour[j].left)
						{
							rectCoutour[j].left = rectCoutour[i].left;
						}
						if (rectCoutour[i].right > rectCoutour[j].right)
						{
							rectCoutour[j].right = rectCoutour[i].right;
						}
						if (rectCoutour[i].top < rectCoutour[j].top)
						{
							rectCoutour[j].top = rectCoutour[i].top;
						}
					}
				}

				//����Ƶ��Ȧ����������
				track_box.angle = 0;
				track_box.center = cvPoint2D32f((rectCoutour[j].right+rectCoutour[j].left)/2.0f, (rectCoutour[j].bottom + rectCoutour[j].top)/2.0f);
				track_box.size = cvSize2D32f(10, 10);
				cvEllipseBox( pFrame, track_box, CV_RGB(0,0,255), 3, CV_AA, 0 );
			}

/////////////////////////////////////////////////////////////////
			//�ָ���Ƶ����
			//�зָ�
			for(i=1; i<MAXX; i++)
			{
				ptr = (uchar*)(pFrame->imageData + (int)(((float)i/MAXX) * pFrame->width) * pFrame->nChannels);
				for(j=0; j<pFrame->height; j++)
					ptr[pFrame->widthStep * j + 1] = 0xFF;
			}
			//�зָ�
			for(i=1; i<MAXY; i++)
			{
				ptr = (uchar*)(pFrame->imageData + pFrame->widthStep * (int)(((float)i/MAXY) * pFrame->height));
				for(j=0; j<pFrame->width; j++)
					ptr[pFrame->nChannels * j + 1] = 0xFF;
			}
/////////////////////////////////////////////////////////////////
			//��ʾͼ��
	  		cvShowImage("video", pFrame);
			
	  		//����а����¼���������ѭ��
	  		//�˵ȴ�ҲΪcvShowImage�����ṩʱ�������ʾ
	  		//�ȴ�ʱ����Ը���CPU�ٶȵ���
	  		if( cvWaitKey(2) >= 0 )
	    		break;
		}  // end of if-else
    } // end of while-loop

  	//���ٴ���
  	cvDestroyWindow("video");

  	//�ͷ�ͼ��;���
  	cvReleaseImage(&pFrImg);
  	cvReleaseImage(&pBkImg);

  	cvReleaseMat(&pFrameMat);
  	cvReleaseMat(&pFrMat);
  	cvReleaseMat(&pBkMat);

  	return 0;
}
