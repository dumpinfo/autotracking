#include "myfgdetector.h"

MyFGDetector::MyFGDetector()
{
  this->Create(0.003, 60.0);
}

MyFGDetector::MyFGDetector(double alpha, double threshold)
{
  this->Create(alpha, threshold);
}

MyFGDetector::~MyFGDetector()
{
  this->Release();
}

void MyFGDetector::Create(double alpha, double threshold)
{
  m_framecount = 0;
  m_alpha = alpha;
  m_threshold = threshold;

  m_pFrame=0;
  m_pFGMask=0;

  m_pFrameMat=0;
  m_pFGMat=0;
  m_pBkMat=0;
}


IplImage* MyFGDetector::GetMask()
{
  return m_pFGMask;
}

void MyFGDetector::Release()
{
  m_framecount = 0;

  if(m_pFrame) cvReleaseImage(&m_pFrame);
  if(m_pFGMask) cvReleaseImage(&m_pFGMask);

  if(m_pFrameMat) cvReleaseMat(&m_pFrameMat);
  if(m_pFGMat) cvReleaseMat(&m_pFGMat);
  if(m_pBkMat) cvReleaseMat(&m_pBkMat);

}


void MyFGDetector::Process(IplImage* pFrame)
{
  if(m_framecount==0)
    {
      m_pFGMask = cvCreateImage(cvSize(pFrame->width, pFrame->height),  IPL_DEPTH_8U,1);

      m_pBkMat = cvCreateMat(pFrame->height, pFrame->width, CV_32FC1);
      m_pFGMat = cvCreateMat(pFrame->height, pFrame->width, CV_32FC1);
      m_pFrameMat = cvCreateMat(pFrame->height, pFrame->width, CV_32FC1);
      
      if( !m_pFGMask || !m_pBkMat || !m_pFGMat || !m_pFrameMat)
	{
	  fprintf(stderr, "Can not alloc memeory.\n");
	  return ;
	}

      //convert to single channel
      cvCvtColor(pFrame, m_pFGMask, CV_BGR2GRAY);

      cvConvert(m_pFGMask, m_pFrameMat);
      cvConvert(m_pFGMask, m_pFGMat);
      cvConvert(m_pFGMask, m_pBkMat);
    }

  //ת���ɵ��ŵ�ͼ���ٴ���
  cvCvtColor(pFrame, m_pFGMask, CV_BGR2GRAY);
  cvConvert(m_pFGMask, m_pFrameMat);
  //��˹�˲��ȣ���ƽ��ͼ��
  cvSmooth(m_pFrameMat, m_pFrameMat, CV_GAUSSIAN, 3, 0, 0);
	  
  //��ǰ֡������ͼ���
  cvAbsDiff(m_pFrameMat, m_pBkMat, m_pFGMat);

  //��ֵ��ǰ��ͼ
  cvThreshold(m_pFGMat, m_pFGMask, m_threshold, 255.0, CV_THRESH_BINARY);

  //������̬ѧ�˲���ȥ������  
  cvErode(m_pFGMask, m_pFGMask, 0, 1);
  cvDilate(m_pFGMask, m_pFGMask, 0, 1);

  //���±���
  cvRunningAvg(m_pFrameMat, m_pBkMat, 0.003, 0);
 

  m_framecount++;
}

