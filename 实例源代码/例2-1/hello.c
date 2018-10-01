/*	程序名：hello.c
  功能：从磁盘中读入图像文件，并将图像显示在屏幕上
*/
#include "cv.h"
#include "highgui.h"

int main( int argc, char** argv )
{
    IplImage* pImg; //声明IplImage指针

    //载入图像
    if( argc == 2 && (pImg = cvLoadImage( argv[1], 1)) != 0 )
    {
        cvNamedWindow( "Image", 1 );	//创建窗口
        cvShowImage( "Image", pImg );	//显示图像
        cvWaitKey(0);					//等待按键
        cvDestroyWindow( "Image" );		//销毁窗口
        cvReleaseImage( &pImg );		//释放图像
        return 0;
    }
    return -1;
}
