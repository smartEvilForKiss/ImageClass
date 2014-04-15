#include "function.h"
#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>

using namespace std; 

int main()
{
	IplImage *src = cvLoadImage("4.jpg");
	IplImage *pcarId,*pcarId_gray,*res_correct_x,*res_correct_y;
	IplImage *res_correct_x_gray;
	float m_x[6],m_y[6];
	CvMat map_x = cvMat(2, 3, CV_32F, m_x);
	CvMat map_y = cvMat(2, 3, CV_32F, m_y);  

	//获得车牌截图
	CvRect carid_rec = getCaridPosition(src);

	//return 0;

	pcarId = cvLoadImage("res.jpg");

	//水平矫正
	pcarId_gray = cvCreateImage(cvGetSize(pcarId),IPL_DEPTH_8U,1);
	res_correct_x = cvCreateImage(cvGetSize(pcarId),IPL_DEPTH_8U,1);
	cvCvtColor(pcarId,pcarId_gray,CV_BGR2GRAY);
	cvThreshold(pcarId_gray,pcarId_gray,0,255,CV_THRESH_BINARY | CV_THRESH_OTSU);

	double angle,temp;
	double angle_x,max_his_x = 0;
	double angle_y,max_his_y = 0;
	for(angle=10;angle>=-10;angle-=0.1)
	{
		res_correct_x = getCorrectImage_x(angle,pcarId_gray);
		cvCanny(res_correct_x,res_correct_x,50,150);
		temp = getMax5Projection_x(res_correct_x);


		if(temp > max_his_x)
		{
			max_his_x = temp;
			angle_x = angle;
		}
		cvReleaseImage(&res_correct_x);
	}

	cout << "水平矫正参数： " << angle_x << " " << max_his_x << endl; 
	res_correct_x = cvCloneImage(pcarId);
	res_correct_x_gray = cvCreateImage(cvGetSize(res_correct_x),IPL_DEPTH_8U,1);
	res_correct_x = getCorrectImage_x_m(angle_x,pcarId,&map_x);

	//垂直矫正
	cvCvtColor(res_correct_x,res_correct_x_gray,CV_BGR2GRAY);
	cvThreshold(res_correct_x_gray,res_correct_x_gray,0,255,CV_THRESH_BINARY | CV_THRESH_OTSU);

	for (angle=10;angle>=-10;angle-=0.1)
	{
		res_correct_y = getCorrectImage_y(angle,res_correct_x_gray);
		cvCanny(res_correct_y,res_correct_y,50,150);
		temp = getMax5Projection_y(res_correct_y);

		if(temp > max_his_y)
		{
			max_his_y = temp;
			angle_y = angle;
		}
		cvReleaseImage(&res_correct_y);
	}

	cout << "垂直矫正参数： " << angle_y << " " << max_his_y << endl;
	res_correct_y = cvCloneImage(pcarId);
	res_correct_y = getCorrectImage_y_m(angle_y,res_correct_x,&map_y);

	IplImage *res_x = getCorrectImage_x(&map_x,src);
	IplImage *res_y = getCorrectImage_y(&map_y,res_x);

	CvRect tempRect = getAccurateCarId(res_correct_y);




	/////////////////////////////////////////////
	IplImage *srcSeg = cvLoadImage("ress.jpg");




	cvReleaseImage(&srcSeg);
	cvReleaseImage(&src);
	cvReleaseImage(&pcarId);
	cvReleaseImage(&pcarId_gray);
	cvReleaseImage(&res_correct_x);
	cvReleaseImage(&res_correct_x_gray);
	cvReleaseImage(&res_correct_y);
}