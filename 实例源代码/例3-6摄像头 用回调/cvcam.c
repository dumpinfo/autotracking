#include "cxcore.h"
#include "cvcam.h"
#include "windows.h"

void callback(IplImage* image);

int main()
{
	int ncams = cvcamGetCamerasCount( );//���ؿ��Է��ʵ�����ͷ��Ŀ
	HWND MyWin;
    
	// ����ϵͳ����
	cvcamSetProperty(0, CVCAM_PROP_ENABLE, CVCAMTRUE); //ѡ���һ������ͷ   
	//camera
    cvcamSetProperty(0, CVCAM_PROP_RENDER, CVCAMTRUE);  //We'll render stream
    // �ڱ�����
    // ���贴��һ�����ڣ����Ҵ��ڵ�ID���ڱ��� MyWin �ж���
    // MyWin �Ǵ��� HWND ������
    MyWin = (HWND) cvGetWindowHandle("cvcam window");   
    cvcamSetProperty(0, CVCAM_PROP_WINDOW, &MyWin);   // Selects a window for video rendering
	//�ص�����������ÿһ֡
	cvcamSetProperty(0, CVCAM_PROP_CALLBACK, callback);
    cvcamInit( );
    cvcamStart( );
    // ���ڳ���ʼ����
    cvWaitKey(0);
    cvcamStop( );
    cvcamExit( );
    return 0;
}

// ��ͼ���л���ɫˮƽ��
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
