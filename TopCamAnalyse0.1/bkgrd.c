#include <stdio.h>
#include <cv.h>
#include <highgui.h>

int main( int argc, char** argv )
{
  //声明IplImage指针
  IplImage* pFrame = NULL;		//每一帧图像的指针
  IplImage* pFrImg = NULL;		//
  IplImage* pBkImg = NULL;		//

  CvMat* pFrameMat = NULL;
  CvMat* pFrMat = NULL;
  CvMat* pBkMat = NULL;
  
  CvCapture* pCapture = NULL;
  
  int nFrmNum = 0;

  //轮廓相关
  CvMemStorage* storage = cvCreateMemStorage(0);
  CvSeq* contour = 0;
  IplImage* dst = 0;
  int		total = 0;
  int		eachtotal = 0;

  //创建窗口
  cvNamedWindow("video", 1);
  cvNamedWindow("background",1);
  cvNamedWindow("foreground",1);
  //使窗口有序排列
  cvMoveWindow("video", 30, 0);
  cvMoveWindow("background", 360, 0);
  cvMoveWindow("foreground", 690, 0);

  if( argc != 2 )
    {
      fprintf(stderr, "Usage: bkgrd <video_file_name>\n");
      return -1;
    }

  //打开视频文件
  if( !(pCapture = cvCaptureFromFile(argv[1])))
    {
      fprintf(stderr, "Can not open video file %s\n", argv[1]);
      return -2;
    }
	pFrame = cvQueryFrame( pCapture );
	pFrImg = cvCreateImage(cvSize(pFrame->width, pFrame->height), IPL_DEPTH_8U,1);
	dst = cvCreateImage( cvGetSize(pFrImg), 8, 3 );
  
  //逐帧读取视频
  	while(pFrame = cvQueryFrame( pCapture ))
    {
      	nFrmNum++;
      
      	//如果是第一帧，需要申请内存，并初始化
      	if(nFrmNum == 1)
		{
	  		pBkImg = cvCreateImage(cvSize(pFrame->width, pFrame->height), IPL_DEPTH_8U,1);
	  		pFrImg = cvCreateImage(cvSize(pFrame->width, pFrame->height), IPL_DEPTH_8U,1);

			pBkMat = cvCreateMat(pFrame->height, pFrame->width, CV_32FC1);
	  		pFrMat = cvCreateMat(pFrame->height, pFrame->width, CV_32FC1);
	  		pFrameMat = cvCreateMat(pFrame->height, pFrame->width, CV_32FC1);

	  		//转化成单通道图像再处理
	  		cvCvtColor(pFrame, pBkImg, CV_BGR2GRAY);
	  		cvCvtColor(pFrame, pFrImg, CV_BGR2GRAY);

	  		cvConvert(pFrImg, pFrameMat);
	  		cvConvert(pFrImg, pFrMat);
	  		cvConvert(pFrImg, pBkMat);
		}
      	else
		{
			//每一帧图像转换成单通道图像
	  		cvCvtColor(pFrame, pFrImg, CV_BGR2GRAY);
	  		cvConvert(pFrImg, pFrameMat);
	  		//先做高斯滤波，以平滑图像
//	  		cvSmooth(pFrameMat, pFrameMat, CV_GAUSSIAN, 3, 0, 0);
	  
	  		//当前帧跟背景图相减
	  		cvAbsDiff(pFrameMat, pBkMat, pFrMat);

	  		//二值化前景图
	  		cvThreshold(pFrMat, pFrImg, 60, 255.0, CV_THRESH_BINARY);

	  		//进行形态学滤波，去掉噪音  
//	 		cvErode(pFrImg, pFrImg, 0, 1);
//	  		cvDilate(pFrImg, pFrImg, 0, 1);

	  		//更新背景
	  		cvRunningAvg(pFrameMat, pBkMat, 0.003, 0);
	  		//将背景转化为图像格式，用以显示
	  		cvConvert(pBkMat, pBkImg);

			//寻找轮廓
			eachtotal = cvFindContours( pFrImg, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));
			cvZero( dst );

			if(contour != 0)
				printf("%d\n", total += eachtotal);
			
			for( ; contour != 0; contour = contour->h_next )
			{
			    CvScalar color = CV_RGB( rand()&255, rand()&255, rand()&255 );
			    /* 用1替代 CV_FILLED  所指示的轮廓外形 */
			    cvDrawContours( dst, contour, color, color, -1, CV_FILLED, 8 ,cvPoint(0, 0));
			}
	  		
			//显示图像
	  		cvShowImage("video", pFrame);
	  		cvShowImage("background", dst);
	  		cvShowImage("foreground", pFrImg);
			
	  		//如果有按键事件，则跳出循环
	  		//此等待也为cvShowImage函数提供时间完成显示
	  		//等待时间可以根据CPU速度调整
	  		if( cvWaitKey(2) >= 0 )
	    		break;
		}  // end of if-else
    } // end of while-loop

  	//销毁窗口
  	cvDestroyWindow("video");
  	cvDestroyWindow("background");
  	cvDestroyWindow("foreground");

  	//释放图像和矩阵
  	cvReleaseImage(&pFrImg);
  	cvReleaseImage(&pBkImg);

  	cvReleaseMat(&pFrameMat);
  	cvReleaseMat(&pFrMat);
  	cvReleaseMat(&pBkMat);

  	return 0;
}
