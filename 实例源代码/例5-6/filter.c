//  Filtering for Image with variaty filtering kernel
//
// CV_PREWITT_3x3_V A gradient filter (vertical Prewitt operator).
//         -1  0  1
//         -1  0  1
//         -1  0  1
// CV_PREWITT_3x3_H A gradient filter (horizontal Prewitt operator).
//          1  1  1
//          0  0  0
//         -1 -1 -1
// CV_SOBEL_3x3_V A gradient filter (vertical Sobel operator).
//         -1  0  1
//         -2  0  2
//         -1  0  1
// CV_SOBEL_3x3_H A gradient filter (horizontal Sobel operator).
//          1  2  1
//          0  0  0
//         -1 -2 -1
// CV_LAPLACIAN_3x3 A 3x3 Laplacian highpass filter.
//         -1 -1 -1
//         -1  8 -1
//         -1 -1 -1
// CV_LAPLACIAN_3x3 A 3x3 Laplacian highpass filter (another kernel)
// This kernel is similar with function: cvLaplace with aperture_size=1
//          0  1  0
//          1  -4  1
//          0  1  0      注：直接用cvFilter2D得到的结果与用cvLaplace得到的结果
//                           略有不同
// CV_LAPLACIAN_5x5 A 5x5 Laplacian highpass filter.
//         -1 -3 -4 -3 -1
//         -3  0  6  0 -3
//         -4  6 20  6 -4
//         -3  0  6  0 -3
//         -1 -3 -4 -3 -1
// CV_GAUSSIAN_3x3 A 3x3 Gaussian lowpass filter.
// This filter uses the kernel A/16,where
//         1 2 1
//     A =  2 4 2
//         1 2 1
// These filter coefficients correspond to a 2-dimensional Gaussian
// distribution with standard deviation 0.85.
//
// CV_GAUSSIAN_5x5 A 5x5 Gaussian lowpass filter.
// This filter uses the kernel A/571,where
//         2  7  12  7  2
//         7 31  52 31  7
//    A =  12 52 127 52 12
//         7 31  52 31  7
//         2  7  12  7  2

#include <cv.h>
#include <highgui.h>
#include <stdio.h>

int main( int argc, char** argv )
{ 
	IplImage *src = 0, *dst = 0, *dst2 = 0;
    /*float k[9] = { 0, 1, 0,
                   1,-4, 1,
                   0, 1, 0}; */
    float k[9] = { 1.f/16, 2.f/16, 1.f/16,
                   2.f/16, 4.f/16, 2.f/16,
                   1.f/16, 2.f/16, 1.f/16};   // 这里高斯核滤波器归一化
    CvMat Km;
    //cvInitMatHeader( &Km, 3, 3, CV_32FC1, k, CV_AUTOSTEP );
    Km = cvMat( 3, 3, CV_32F, k );

    // 0:  force to gray image
	src = cvLoadImage("lena.jpg", 0);
    dst = cvCloneImage( src );

    cvNamedWindow("src", 0);
    cvShowImage("src",src);
    
    cvNamedWindow("filtering", 0);
    cvFilter2D( src, dst, &Km, cvPoint(-1,-1));
    cvShowImage("filtering",dst);
    cvWaitKey(0);

    cvReleaseImage( &src );
    cvReleaseImage( &dst );
    return 0;
}
