#include <stdio.h>
#include <cv.h>
#include <highgui.h>

#define MAX 1000
#define DISTANCE 10
#define MAXX 5
#define MAXY 4

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

	//对前景图像处理相关变量
	int i, j, k;
	int num = 0;
	int temp[MAX];
	uchar* ptr = NULL;

	//轮廓相关
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contour = 0;
	int		eachtotal = 0;
	int		contour_area_tmp = 0;//面积
	int		contour_area_max = 0;//面积最大值

//////////////////////////////////////////////////////////////
	//创建窗口
	cvNamedWindow("video", 1);
	//使窗口有序排列
	cvMoveWindow("video", 30, 0);

//////////////////////////////////////////////////////////////
	//打开视频文件
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
	  		cvSmooth(pFrameMat, pFrameMat, CV_GAUSSIAN, 3, 0, 0, 0);
	  
	  		//当前帧跟背景图相减
	  		cvAbsDiff(pFrameMat, pBkMat, pFrMat);

	  		//二值化前景图
	  		cvThreshold(pFrMat, pFrImg, 60, 255.0, CV_THRESH_BINARY);

	  		//更新背景
			Sleep(100);
	  		cvRunningAvg(pFrameMat, pBkMat, 0.03, 0);
	  		//将背景转化为图像格式，用以显示
	  		cvConvert(pBkMat, pBkImg);

/////////////////////////////////////////////////////////////////
			//对前景图像进行处理
			//以行为顺序遍历数据区
			for(i = 0; i < pFrImg->height; i++)
			{
				//指向每行第一个元素
				ptr = (uchar*)(pFrImg->imageData + i * pFrImg->widthStep);
				
				for (j = 0; j < pFrImg->width; j++)
				{
					if(ptr[j] == 0xFF)
					{
						temp[num] = j;
						num++;
					}
				}

				//连接相邻点
				for(k=0; k<num-1; k++)
				{
					if(temp[k+1] - temp[k] < DISTANCE)
					{
						for(j = temp[k]+1; j < temp[k+1]; j++ )
							ptr[j] = 0xFF;
					}
				}
				num = 0;
			}

			//以列为顺序遍历数据区
			for(i = 0; i < pFrImg->width; i++)
			{
				//指向每列第一个元素
				ptr = (uchar*)(pFrImg->imageData + i);
				
				for (j = 0; j < pFrImg->height; j++)
				{
					if(ptr[pFrImg->widthStep * j] == 0xFF)
					{
						temp[num] = j;
						num++;
					}
				}

				//连接相邻点
				for(k=0; k<num-1; k++)
				{
					if(temp[k+1] - temp[k] < DISTANCE)
					{
						for(j = temp[k]+1; j < temp[k+1]; j++ )
							ptr[pFrImg->widthStep * j] = 0xFF;
					}
				}
				num = 0;
			}
/////////////////////////////////////////////////////////////////
			//寻找轮廓
			eachtotal = cvFindContours( pFrImg, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));
			
			for( ; contour != 0; contour = contour->h_next )
			{
			    /* 用1替代 CV_FILLED  所指示的轮廓外形 */
			    cvDrawContours( pFrame, contour, CV_RGB(255, 0, 0), CV_RGB(255, 0, 0), 1, CV_FILLED, 8 ,cvPoint(0, 0));
				contour_area_tmp = fabs(cvContourArea( contour, CV_WHOLE_SEQ )); //获取当前轮廓面积
				
				if( contour_area_tmp > contour_area_max )
				{
					contour_area_max = contour_area_tmp; //找到面积最大的轮廓
					printf("当前帧轮廓个数：%8d，	当前帧轮廓最大面积：%8d\n", eachtotal, contour_area_max);
		        }
			}
			contour_area_max = 0;

/////////////////////////////////////////////////////////////////
			//分割视频区域
			//列分割
			for(i=1; i<MAXX; i++)
			{
				ptr = (uchar*)(pFrame->imageData + (int)(((float)i/MAXX) * pFrame->width) * pFrame->nChannels);
				for(j=0; j<pFrame->height; j++)
					ptr[pFrame->widthStep * j + 1] = 0xFF;
			}
			//行分割
			for(i=1; i<MAXY; i++)
			{
				ptr = (uchar*)(pFrame->imageData + pFrame->widthStep * (int)(((float)i/MAXY) * pFrame->height));
				for(j=0; j<pFrame->width; j++)
					ptr[pFrame->nChannels * j + 1] = 0xFF;
			}
/////////////////////////////////////////////////////////////////
			//显示图像
	  		cvShowImage("video", pFrame);
			
	  		//如果有按键事件，则跳出循环
	  		//此等待也为cvShowImage函数提供时间完成显示
	  		//等待时间可以根据CPU速度调整
	  		if( cvWaitKey(2) >= 0 )
	    		break;
		}  // end of if-else
    } // end of while-loop

  	//销毁窗口
  	cvDestroyWindow("video");

  	//释放图像和矩阵
  	cvReleaseImage(&pFrImg);
  	cvReleaseImage(&pBkImg);

  	cvReleaseMat(&pFrameMat);
  	cvReleaseMat(&pFrMat);
  	cvReleaseMat(&pBkMat);

  	return 0;
}
