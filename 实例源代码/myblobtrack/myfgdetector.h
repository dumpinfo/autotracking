#ifndef  MYFGDETECTOR_H
#define  MYFGDETECTOR_H

#include <cxcore.h>
#include <cv.h>
#include <cvaux.h>
#include <stdio.h>

class MyFGDetector:public CvFGDetector
{
 protected:
  int m_framecount;
  double m_alpha;
  double m_threshold;

  IplImage* m_pFrame; 
  IplImage* m_pFGMask;

  CvMat* m_pFrameMat;
  CvMat* m_pFGMat;
  CvMat* m_pBkMat;

  void Create(double alpha, double threshold);

 public:
  MyFGDetector();
  MyFGDetector(double alpha, double threshold);
  ~MyFGDetector();

  IplImage* GetMask();
  /* process current image */
  void    Process(IplImage* pFrame);
  /* release foreground detector */
  void    Release();
};

#endif
