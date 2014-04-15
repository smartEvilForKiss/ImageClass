#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <cv.hpp>

using namespace std;
using namespace cv;

struct BOUND
{
	int upperbuond;
	int lowerbound;
};

struct CENTERDIFF
{
	int position;
	int diff;
};

struct GRADIENT
{
	double value[8];
};

///////////////////////////////////////////////////
//1�����ƶ�λ��
CvRect getCaridPosition(IplImage *src);

///////////////////////////////////////////////////
//2�����ƴ���--ˮƽ����
int getMax5Projection_x(IplImage *src);
IplImage* getCorrectImage_x(double angle,IplImage *src); 
IplImage* getCorrectImage_x(CvMat *map,IplImage *src); 
IplImage* getCorrectImage_x_m(double angle,IplImage *src,CvMat *map); 


///////////////////////////////////////////////////
//2�����ƴ���--��ֱ����
int getMax5Projection_y(IplImage *src);
IplImage* getCorrectImage_y(double angle,IplImage *src);
IplImage* getCorrectImage_y(CvMat *map,IplImage *src);
IplImage* getCorrectImage_y_m(double angle,IplImage *src,CvMat *map);

///////////////////////////////////////////////////
//3������ȥ�߿�
CvRect getAccurateCarId(IplImage *src);
bool cmp(CENTERDIFF a,CENTERDIFF b);
bool cmpRect(CvRect a,CvRect b);