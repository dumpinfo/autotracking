/*  	�������� image.cpp
���ܣ� ��������ʾ�����C++������������ʾͼ�����C++���� cxcore.hpp �ж��壬�� ������(CvMatrix) ����
*/

#include "cv.h"
#include "highgui.h"

int main( int argc, char** argv )
{
    // �ṹ������ͼ��ͼ��Ҳ��BMPͼ��cvLoadImage������������ʽ
    // XML/YAML (cvLoad)
    CvImage img(argc > 1 ? argv[1] : "lena.jpg", 0, CV_LOAD_IMAGE_COLOR),
        img_yuv, y, noise;
    CvRNG rng = cvRNG(-1);

    if( !img.data() ) // ���ͼ���Ƿ�����
        return -1;

    img_yuv = img.clone(); // ��¡ͼ��
    cvCvtColor( img, img_yuv, CV_BGR2YCrCb ); // ɫ�ʿռ�ת��

    y.create( img.size(), IPL_DEPTH_8U, 1 ); // ����ͼ��
    noise.create( img.size(), IPL_DEPTH_32F, 1 );

    cvSplit( img_yuv, y, 0, 0, 0 ); // �ֽ�
// ��̬�ֲ����������
cvRandArr( &rng, noise, CV_RAND_NORMAL, cvScalarAll(0), cvScalarAll(20) ); 
    cvSmooth( noise, noise, CV_GAUSSIAN, 5, 5, 1, 1 ); // GAUSSIAN�˲���ƽ��
    cvAcc( y, noise ); 	// noise = noise + y
    cvConvert( noise, y ); // y = noise * 1 + 0
    cvMerge( y, 0, 0, 0, img_yuv );  // ͼ��ϲ�
    cvCvtColor( img_yuv, img, CV_YCrCb2BGR ); // ͼ��ɫ�ʿռ�ת��

    cvNamedWindow( "image with grain", CV_WINDOW_AUTOSIZE );
    img.show( "image with grain" ); // cvShowImage������һ����ʽ
    cvWaitKey();

    return 0;
// ����ͼ���Զ��ͷţ�����ʹ��C++��ȽϷ���ĵط�
}
