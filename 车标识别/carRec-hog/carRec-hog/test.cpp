#include "function.h"

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <opencv.hpp>
#include <iostream>
#include <vector>


using namespace std;


int imageMatch(IplImage *src,int centerPos)
{
	int tempWidth[10];
	int tempHeight[10];
	vector<float> resVec[10];
	int width = src->width;
	int height = src->height;
	int center = centerPos;
	int leftFen,rightFen,upperFen,lowerFen;
	IplImage *tempImage;
	string filename;
	int stride = height/15;

	char featureName[100];
	int brandFlag = 0;
	double mindis = 100000000;
	int final_x,final_y,final_width,final_height;

	final_x = 0;final_y = 0;
	final_width = 1;final_height = 1;

	//cvSmooth(src,src,CV_GAUSSIAN,7,7,1,1);
	

	readHogFeature("1.feature",resVec[0],tempWidth[0],tempHeight[0]);
	readHogFeature("2.feature",resVec[1],tempWidth[1],tempHeight[1]);
	readHogFeature("3.feature",resVec[2],tempWidth[2],tempHeight[2]);
	readHogFeature("4.feature",resVec[3],tempWidth[3],tempHeight[3]);
	readHogFeature("5.feature",resVec[4],tempWidth[4],tempHeight[4]);
	readHogFeature("6.feature",resVec[5],tempWidth[5],tempHeight[5]);
	readHogFeature("7.feature",resVec[6],tempWidth[6],tempHeight[6]);
	readHogFeature("8.feature",resVec[7],tempWidth[7],tempHeight[7]);


	for(int k=0;k<8;k++)
	{
		int position_x=0,position_y=0;
		double minDistance = 1000000000;
		double tempDistance;

		if(tempHeight[k] > height)
		{
			continue;
		}

		tempImage = cvCreateImage(cvSize(tempWidth[k],tempHeight[k]),src->depth,src->nChannels);


		for(int i=center-tempWidth[k]/2-8;i<=center-tempWidth[k]/2+8;i+=stride)
		{
			leftFen = i;
			rightFen = leftFen + tempWidth[k];
			for(int j=0;j<src->height-tempHeight[k]+1;j+=2)
			{
				upperFen = j;
				lowerFen = j+tempHeight[k];


				cvSetImageROI(src,cvRect(leftFen,upperFen,tempWidth[k],tempHeight[k]));
				cvCopy(src,tempImage);
				cvResetImageROI(src);

				vector<float> tempVec;
				getHogFeature(tempImage,tempVec);

				//计算平均距离
				tempDistance = getFeatureDistance(resVec[k],tempVec);
				if(tempDistance < minDistance)
				{
					minDistance = tempDistance;
					position_x = leftFen;
					position_y = upperFen;
				}

				
			}
		}

		if(minDistance < mindis)
		{
			mindis = minDistance;
			brandFlag = k+1;
			final_x = position_x;
			final_y = position_y;
			final_height = tempHeight[k];
			final_width = tempWidth[k];
		}

		cvReleaseImage(&tempImage);
	}


	if(brandFlag == 1)
	{
		cout << "车标----奥迪" << endl;
	}
	else if(brandFlag == 2)
	{
		cout << "车标----本田" << endl;
	}
	else if(brandFlag == 3)
	{
		cout << "车标----大众" << endl;
	}
	else if(brandFlag == 4)
	{
		cout << "车标----丰田" << endl;
	}
	else if(brandFlag == 5)
	{
		cout << "车标----现代" << endl;
	}
	else if(brandFlag == 6)
	{
		cout << "车标----雪弗兰" << endl;
	}
	else if(brandFlag == 7)
	{
		cout << "车标----别克" << endl;
	}
	else if(brandFlag == 8)
	{
		cout << "车标----马自达" << endl;
	}
	else
	{
		cout << "不能识别这是什么车标" << endl;
	}

	tempImage = cvCreateImage(cvSize(final_width,final_height),src->depth,src->nChannels);
	cvSetImageROI(src,cvRect(final_x,final_y,final_width,final_height));
	cvCopy(src,tempImage);
	cvResetImageROI(src);


	cvNamedWindow("2");
	cvShowImage("2",tempImage);
	cvWaitKey(0);
	cvDestroyAllWindows();
	cvReleaseImage(&tempImage);

	return brandFlag;
}

int main()
{
	IplImage *src = cvLoadImage("38.jpg");
	IplImage *pcarId,*pcarId_gray,*res_correct_x,*res_correct_y;
	IplImage *res_correct_x_gray;
	IplImage *brand_big;
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

	//cout << "水平矫正参数： " << angle_x << " " << max_his_x << endl; 
	res_correct_x_gray = cvCreateImage(cvGetSize(pcarId),IPL_DEPTH_8U,1);
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

	//cout << "垂直矫正参数： " << angle_y << " " << max_his_y << endl;
	res_correct_y = getCorrectImage_y_m(angle_y,res_correct_x,&map_y);

	IplImage *res_x = getCorrectImage_x(&map_x,src);
	IplImage *res_y = getCorrectImage_y(&map_y,res_x);

	CvRect tempRect = getAccurateCarId(res_correct_y);
	carid_rec.x += tempRect.x;
	carid_rec.y += tempRect.y;
	carid_rec.width = tempRect.width;
	carid_rec.height = tempRect.height;

	int leftbutton_x;
	int leftbutton_y;
	leftbutton_x = carid_rec.x;
	leftbutton_y = carid_rec.y + carid_rec.height;

	carid_rec.x = carid_rec.x*m_x[0] + carid_rec.y*m_x[1] + m_x[2];
	carid_rec.y = carid_rec.x*m_x[3] + carid_rec.y*m_x[4] + m_x[5];
	carid_rec.x = carid_rec.x*m_y[0] + carid_rec.y*m_y[1] + m_y[2];
	carid_rec.y = carid_rec.x*m_y[3] + carid_rec.y*m_y[4] + m_y[5];


	leftbutton_x = leftbutton_x*m_x[0] + leftbutton_y*m_x[1] + m_x[2];
	leftbutton_y = leftbutton_x*m_x[3] + leftbutton_y*m_x[4] + m_x[5];
	leftbutton_x = leftbutton_x*m_y[0] + leftbutton_y*m_y[1] + m_y[2];
	leftbutton_y = leftbutton_x*m_y[3] + leftbutton_y*m_y[4] + m_y[5];

	carid_rec.height = leftbutton_y - carid_rec.y;

	brand_big = getbrandrect_big(carid_rec,res_y);

	double factor;
	if(carid_rec.height < 27)
		factor = (double)32/27;
	else
		factor = (double)32/carid_rec.height;
	BOUND centerBound;
	IplImage *x_center_img = getbrandrect_small(factor,brand_big,centerBound);

	imageMatch(x_center_img,centerBound.lowerbound);

	cvReleaseImage(&src);
	cvReleaseImage(&pcarId);
	cvReleaseImage(&pcarId_gray);
	cvReleaseImage(&res_correct_x);
	cvReleaseImage(&res_correct_x_gray);
	cvReleaseImage(&res_correct_y);
	cvReleaseImage(&brand_big);
	cvReleaseImage(&res_x);
	cvReleaseImage(&res_y);
	cvReleaseImage(&x_center_img);
}