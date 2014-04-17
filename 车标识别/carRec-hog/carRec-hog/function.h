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
//1、车牌定位器
CvRect getCaridPosition(IplImage *src);
void getHSI(IplImage *psrc,IplImage *hsi_i,IplImage *hsi_s,IplImage *hsi_h);
void setBinH(IplImage *hsi_h,IplImage *binImage,int lowFen,int maxFen);
void setBinS(IplImage *hsi_s,IplImage *binImage,int fen);
vector<CvRect> getCaridPosition_method1(IplImage *src);
vector<CvRect> getCaridPosition_method2(IplImage *src);

///////////////////////////////////////////////////
//2、车牌处理--水平矫正
int getMax5Projection_x(IplImage *src);
IplImage* getCorrectImage_x(double angle,IplImage *src); 
IplImage* getCorrectImage_x(CvMat *map,IplImage *src); 
IplImage* getCorrectImage_x_m(double angle,IplImage *src,CvMat *map); 


///////////////////////////////////////////////////
//2、车牌处理--垂直矫正
int getMax5Projection_y(IplImage *src);
IplImage* getCorrectImage_y(double angle,IplImage *src);
IplImage* getCorrectImage_y(CvMat *map,IplImage *src);
IplImage* getCorrectImage_y_m(double angle,IplImage *src,CvMat *map);

///////////////////////////////////////////////////
//3、车牌去边框
CvRect getAccurateCarId(IplImage *src);


///////////////////////////////////////////////////
//3、车标处理器(方法一)
IplImage* getbrandrect_big(CvRect cvrect,IplImage *src);
BOUND getProject_x(IplImage *src);
BOUND getCenter_y(IplImage *src);
int getCenter_y_help(IplImage *src);
bool cmp(CENTERDIFF a,CENTERDIFF b);
bool cmpRect(CvRect a,CvRect b);
IplImage* getbrandrect_small(double factor,IplImage *brand_big,BOUND &centerBound);


///////////////////////////////////////////////////
//4、车标匹配
void readHogFeature(char *filename,vector<float> &feaVec,int &width,int &height);
void getHogFeature(IplImage *src,vector<float> &feaVec);
void saveFeature(vector<float> &res,int width,int height,char *saveName);
float getFeatureDistance(vector<float> &a,vector<float> &b);

void readHogFeature2(char *filename,vector<float> &feaVec,int &width,int &height);
void getHogFeature2(IplImage *src,vector<float> &feaVec);
void saveFeature2(vector<float> &res,int width,int height,char *saveName);
float getFeatureDistance2(vector<float> &a,vector<float> &b);
