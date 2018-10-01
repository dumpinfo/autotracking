#include "cxcore.h"
#include "cvcam.h"
#include "windows.h"

void callback(IplImage* image);

int main()
{
	int ncams = cvcamGetCamerasCount( );//返回可以访问的摄像头数目
	HWND MyWin;
    
	// 设置系统属性
	cvcamSetProperty(0, CVCAM_PROP_ENABLE, CVCAMTRUE); //选择第一个摄像头   
	//camera
    cvcamSetProperty(0, CVCAM_PROP_RENDER, CVCAMTRUE);  //We'll render stream
    // 在本例中
    // 假设创建一个窗口，并且窗口的ID是在变量 MyWin 中定义
    // MyWin 是窗口 HWND 的类型
    MyWin = (HWND) cvGetWindowHandle("cvcam window");   
    cvcamSetProperty(0, CVCAM_PROP_WINDOW, &MyWin);   // Selects a window for video rendering
	//回调函数将处理每一帧
	cvcamSetProperty(0, CVCAM_PROP_CALLBACK, callback);
    cvcamInit( );
    cvcamStart( );
    // 现在程序开始工作
    cvWaitKey(0);
    cvcamStop( );
    cvcamExit( );
    return 0;
}

// 在图像中画兰色水平线
void callback(IplImage* image)
{
    IplImage* image1 = image;
    int i,j;
    
    assert (image);
    
    for(i=0; i<image1->height; i+=10)
    {
        for(j=(image1->widthStep)*i; j<(image1->widthStep)*(i+1); 
        j+=image1->nChannels)
        {
            image1->imageData[j]   = (char)255;
            image1->imageData[j+1] = 0;
            image1->imageData[j+2] = 0;
        } 
     }
}
