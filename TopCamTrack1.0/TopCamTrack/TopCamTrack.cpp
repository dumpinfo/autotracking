#include <stdio.h>
#include <cv.h>
#include <highgui.h>

////////////////////////////////////////////////////////
//宏定义
#define SIZEMAX			1000		//分配内存最大值
#define POINTDISTANCE	10			//点之间的距离域值
#define MAXXX			10//10		//横向分割个数
#define MAXY			8//8		//纵向分割个数
#define OBJECTDISTANCE	(320/4.5)	//跟踪物体之间的最小距离
#define MINAREA			5			//最小面积域值
#define TRACKEDMAX		10			//跟踪物体的最大数目
#define BACKFRAMEDELTA	0.0005		//背景更新速度，越小越慢
#define MAXDISTANCE		65535		//默认的最远距离
/////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
//声明IplImage指针
IplImage*	pFrame = NULL;		//每一帧图像的指针
IplImage*	pFrImg = NULL;		//前景图像（二值化的）
IplImage*	pBkImg = NULL;		//背景图像（二值化的）

CvMat*	pFrameMat	= NULL;
CvMat*	pFrMat		= NULL;
CvMat*	pBkMat		= NULL;

CvCapture* pCapture = NULL;

int nFrmNum = 0;				//帧序号
/////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
//对前景图像处理相关变量
int i, j, k;
int num = 0;
int temp[SIZEMAX];
uchar* ptr = NULL;
/////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
//轮廓相关
CvMemStorage*	storage			= cvCreateMemStorage(0);
CvSeq*			contour			= 0;
int				eachtotal		= 0;

int				contour_area_tmp= 0;	//面积
int				contour_area_max= 0;	//面积最大之

int				lpt				= 0;
CvSeqReader		reader;

int				maxx, maxy, minx, miny;

int				areaCoutour[SIZEMAX];	//存储轮廓面积
RECT			rectCoutour[SIZEMAX];	//存储外接矩形
CvPoint			p1, p2;
int				MaxArea = 0;

RECT			trackRect[SIZEMAX];
int				numOfTrackRect = 0;

RECT			lastRect[TRACKEDMAX];	//上一帧的跟踪物体
int				NumOfTracked = 0;
BOOL			newTracked = FALSE;		//TRUE为需要重绘，FALSE不需要重绘

CvPoint			trackPoint[TRACKEDMAX];	//历史跟踪数据，第0个元素存储最后一个的角标
/////////////////////////////////////////////////////////

//计算两个点之间的距离
int Distance(CvPoint *p1, CvPoint *p2)
{
	return (int)sqrt((double)(p1->x-p2->x)*(p1->x-p2->x) + (p1->y-p2->y)*(p1->y-p2->y));
}

//背景差分算法
void BackDiff()
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
	//	cvRunningAvg(pFrameMat, pBkMat, 0.003, 0);
	cvRunningAvg(pFrameMat, pBkMat, BACKFRAMEDELTA, 0);
	//将背景转化为图像格式，用以显示
	cvConvert(pBkMat, pBkImg);
}

//对前景图像进行处理：对相邻的点进行填充
void FillFrImg()
{
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
			if(temp[k+1] - temp[k] < POINTDISTANCE)
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
			if(temp[k+1] - temp[k] < POINTDISTANCE)
			{
				for(j = temp[k]+1; j < temp[k+1]; j++ )
					ptr[pFrImg->widthStep * j] = 0xFF;
			}
		}
		num = 0;
	}
}

//寻找轮廓
//areaCoutour: 存每个轮廓的面积
//rectCoutour: 存每个轮廓的外接举行
void FindContours()
{
	num = 0;
	eachtotal = cvFindContours( pFrImg, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));

	//遍历每一帧中所有轮廓，保存轮廓信息
	//areaCoutour: 存每个轮廓的面积
	//rectCoutour: 存每个轮廓的外接举行
	for( ; contour != 0; contour = contour->h_next )
	{
		//在视频中填充轮廓
		//		    cvDrawContours( pFrame, contour, CV_RGB(255, 0, 0), CV_RGB(255, 0, 0), 1, CV_FILLED, 8 ,cvPoint(0, 0));
		//计算轮廓面积
		contour_area_tmp = (int)fabs(cvContourArea( contour, CV_WHOLE_SEQ ));

		//如果面积小于一定阈值则忽略
		if (contour_area_tmp < MINAREA)
			continue;

		//计算这个轮廓的外接矩形
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

		//存储这个外界矩形
		rectCoutour[num].bottom		=	maxy;
		rectCoutour[num].left		=	minx;
		rectCoutour[num].right		=	maxx;
		rectCoutour[num].top		=	miny;
		//将这个轮廓的面积存入areaCountour中
		areaCoutour[num] = (int)fabs((double)(maxy-miny) * (minx-maxx));
		num++;
	}
}


//将小轮廓合并
//将轮廓放入trackRect中，个数放入numOfTrackRect中
void MergeSmallCoutour()
{
	numOfTrackRect = 0;
	do
	{
		//找到最大的面积，角标存入MaxArea中
		MaxArea = 0;
		contour_area_max = areaCoutour[MaxArea];
		for (i=1; i<num; i++)
		{
			if (areaCoutour[i] > contour_area_max)
			{
				contour_area_max = areaCoutour[i];
				MaxArea = i;
			}
		}
		if (contour_area_max <= 0)
			break;

		//找出离最大轮廓距离小于OBJECTDISTANCE的轮廓
		//p1 存最大轮廓的中心位置
		p1.x = (rectCoutour[MaxArea].right + rectCoutour[MaxArea].left)/2;
		p1.y = (rectCoutour[MaxArea].bottom + rectCoutour[MaxArea].top)/2;
		//遍历所有当前轮廓，找到外接矩形
		for (i=0; i<num ; i++)
		{
			if (areaCoutour[i] < 0)	//如果这个轮廓已经消除，或者就是这个最大面积轮廓
				continue;

			p2.x = (rectCoutour[i].right + rectCoutour[i].left)/2;
			p2.y = (rectCoutour[i].bottom + rectCoutour[i].top)/2;

			//如果距离小于OBJECTDISTANCE并且不是这个最大轮廓本身
			k = Distance(&p1, &p2);
			if (k < OBJECTDISTANCE)
			{
				if (rectCoutour[i].bottom > rectCoutour[MaxArea].bottom)
				{
					rectCoutour[MaxArea].bottom = rectCoutour[i].bottom;
				}
				if (rectCoutour[i].left < rectCoutour[MaxArea].left)
				{
					rectCoutour[MaxArea].left = rectCoutour[i].left;
				}
				if (rectCoutour[i].right > rectCoutour[MaxArea].right)
				{
					rectCoutour[MaxArea].right = rectCoutour[i].right;
				}
				if (rectCoutour[i].top < rectCoutour[MaxArea].top)
				{
					rectCoutour[MaxArea].top = rectCoutour[i].top;
				}

				//删除这个轮廓
				areaCoutour[i] = -1;
			}
		}
		trackRect[numOfTrackRect] = rectCoutour[MaxArea];
		numOfTrackRect++;
		areaCoutour[MaxArea] = -1;
	}while (contour_area_max > 0);
}


//分割视频区域
void SegmentFrame()
{
	//列分割
	for(i=1; i<MAXXX; i++)
	{
		ptr = (uchar*)(pFrame->imageData + (int)(((float)i/MAXXX) * pFrame->width) * pFrame->nChannels);
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
}
//选择跟踪物体
void SearchObject()
{
	static CvPoint	lastPoint;				//上一次跟踪物体的中心位置
	static int		numOfTrackPoint = 0;	//防止抖动产生的参数
	int				indexOfTrackedRect = 0;	//选定的被跟踪物体
	int				k = MAXDISTANCE;		//

	//选择跟踪物体，策略：与上一次跟踪距离最小
	CvPoint pt;
	for (int i=0; i < numOfTrackRect; i++)
	{
		pt.x = (trackRect[i].left + trackRect[i].right)/2;
		pt.y = (trackRect[i].bottom + trackRect[i].top)/2;

//		cvRectangle(pFrame, cvPoint(trackRect[i].left, trackRect[i].top), cvPoint(trackRect[i].right, trackRect[i].bottom), CV_RGB(0,255,0), 5, 8, 0);

		if(Distance(&lastPoint, &pt) < k)
		{
			indexOfTrackedRect = i;
			k = Distance(&lastPoint, &pt);
		}
	}

	pt.x = (trackRect[indexOfTrackedRect].left + trackRect[indexOfTrackedRect].right)/2;
	pt.y = (trackRect[indexOfTrackedRect].bottom + trackRect[indexOfTrackedRect].top)/2;
	lastPoint = pt;

	//计算属于哪个选定区域
	p1.x = (trackRect[indexOfTrackedRect].right + trackRect[indexOfTrackedRect].left)/2;
	p1.y = (trackRect[indexOfTrackedRect].bottom + trackRect[indexOfTrackedRect].top)/2;
	p1.x = p1.x * MAXXX / 320;	//第几个横的点
	p1.y = p1.y * MAXY  / 240;  //第几个纵的点

	//防止频繁变动，并且把最新信息发送出去
	//如果是第一次跟踪物体
	if (numOfTrackPoint == 3)
	{
		if (trackPoint[2].x != p1.x || trackPoint[2].y != p1.y)	//防止重复
		{
			if (trackPoint[1].x != p1.x || trackPoint[1].y != p1.y)	//防止抖动
			{
				trackPoint[3] = p1;
				printf("%d, %d\n", p1.x, p1.y);
				trackPoint[0] = trackPoint[1];
				trackPoint[1] = trackPoint[2];
				trackPoint[2] = trackPoint[3];
			}
		}
	}

	if (numOfTrackPoint == 0)
	{
		trackPoint[0] = trackPoint[1] = trackPoint[2] = p1;
		printf("%d, %d\n", p1.x, p1.y);
		numOfTrackPoint = 3;
	}
	
	//找出警报区域
	//矩形的坐标 P1存储
	p2.x = trackPoint[2].x * 320/MAXXX;
	p2.y = trackPoint[2].y * 240/MAXY;
}
//对每一帧图像进行处理
void FrameProcess()
{
	//打开视频文件
	if( !(pCapture = cvCaptureFromFile("E:\\中科院实习\\主动跟踪\\top-down1.avi")))
	{
		return ;
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

			trackPoint[0].x = trackPoint[0].y = 1;	//初始化历史跟踪数据
		}
		else
		{
			//运行背景差分算法
			BackDiff();

			//填充背景差分计算出来的前景图FrImg
			FillFrImg();
			
			//寻找轮廓
			FindContours();	

			//将小轮廓合并
			MergeSmallCoutour();

			//选择跟踪物体
			SearchObject();

			//分割视频区域
			SegmentFrame();

			//标出跟踪物体矩形
			cvRectangle(pFrame, cvPoint(p2.x, p2.y), cvPoint(p2.x+320/MAXXX, p2.y+240/MAXY), CV_RGB(255,0,0), 5, 8, 0);

			//显示图像
			cvShowImage("video", pFrame);

			//如果有按键事件，则跳出循环
			//次等待也为cvShowImage函数提供时间完成显示
			//等待时间可以根据CPU速度调整
			if( cvWaitKey(2) >= 0 )
				break;
		}  // end of if-else
	} // end of while-loop
}

int main( int argc, char** argv )
{
	//创建窗口
	cvNamedWindow("video", 1);
	//使窗口有序排列
	cvMoveWindow("video", 30, 0);

	//对每一帧图像进行处理
	FrameProcess();

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
