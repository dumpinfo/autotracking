/*  	程序名： image.cpp
功能： 本程序显示如何用C++类来创建和显示图像，这个C++类在 cxcore.hpp 中定义，与 矩阵类(CvMatrix) 相似
*/

#include "cv.h"
#include "highgui.h"

int main( int argc, char** argv )
{
    // 结构中载入图像：图像也是BMP图像（cvLoadImage）或者其它格式
    // XML/YAML (cvLoad)
    CvImage img(argc > 1 ? argv[1] : "lena.jpg", 0, CV_LOAD_IMAGE_COLOR),
        img_yuv, y, noise;
    CvRNG rng = cvRNG(-1);

    if( !img.data() ) // 检查图像是否被载入
        return -1;

    img_yuv = img.clone(); // 克隆图像
    cvCvtColor( img, img_yuv, CV_BGR2YCrCb ); // 色彩空间转换

    y.create( img.size(), IPL_DEPTH_8U, 1 ); // 创建图像
    noise.create( img.size(), IPL_DEPTH_32F, 1 );

    cvSplit( img_yuv, y, 0, 0, 0 ); // 分解
// 正态分布的随机数组
cvRandArr( &rng, noise, CV_RAND_NORMAL, cvScalarAll(0), cvScalarAll(20) ); 
    cvSmooth( noise, noise, CV_GAUSSIAN, 5, 5, 1, 1 ); // GAUSSIAN滤波做平衡
    cvAcc( y, noise ); 	// noise = noise + y
    cvConvert( noise, y ); // y = noise * 1 + 0
    cvMerge( y, 0, 0, 0, img_yuv );  // 图层合并
    cvCvtColor( img_yuv, img, CV_YCrCb2BGR ); // 图像色彩空间转换

    cvNamedWindow( "image with grain", CV_WINDOW_AUTOSIZE );
    img.show( "image with grain" ); // cvShowImage的另外一种形式
    cvWaitKey();

    return 0;
// 所有图像自动释放，这是使用C++类比较方便的地方
}
