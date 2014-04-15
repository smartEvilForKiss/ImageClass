#include "function.h"




CvRect getCaridPosition(IplImage *src)
{
	IplImage *gray,*sobel,*erodeDilate,*temp;
	vector<CvRect> recvVec;
	vector<CvRect> recvVec_temp;
	vector<CvRect> recvVec_corr;

	gray = cvCreateImage(cvGetSize(src),IPL_DEPTH_8U,1);
	temp = cvCreateImage(cvGetSize(src),IPL_DEPTH_8U,1);
	erodeDilate = cvCreateImage(cvGetSize(src),IPL_DEPTH_8U,1);
	sobel = cvCreateImage(cvGetSize(src),IPL_DEPTH_16S,1);

	cvCvtColor(src,gray,CV_BGR2GRAY);
	cvSobel(gray,sobel,2,0,7);
	cvConvertScale(sobel,temp,0.005,0);
	cvThreshold(temp,temp,0,255,CV_THRESH_BINARY_INV | CV_THRESH_OTSU);

// 	cvNamedWindow("1");
// 	cvShowImage("1",temp);
// 	cvWaitKey(0);
// 	cvDestroyWindow("1");


	
	//膨胀和腐蚀
	IplConvKernel *kernel = cvCreateStructuringElementEx(3,1,1,0,CV_SHAPE_RECT);
	cvErode(temp,erodeDilate,kernel,7);
	cvDilate(erodeDilate,erodeDilate,kernel,7);

	kernel = cvCreateStructuringElementEx(1,3,0,1,CV_SHAPE_RECT);
	cvErode(erodeDilate,erodeDilate,kernel,2);
	cvDilate(erodeDilate,erodeDilate,kernel,2);

 	cvSmooth(erodeDilate,erodeDilate,CV_MEDIAN,7,7);
	cvNot(erodeDilate,erodeDilate);
	
// 	cvNamedWindow("1");
// 	cvShowImage("1",erodeDilate);
// 	cvWaitKey(0);
// 	cvDestroyWindow("1");

	//查找轮廓
	CvMemStorage *storage = cvCreateMemStorage();
	CvSeq *contours;
	cvFindContours(erodeDilate,storage,&contours);

	while(contours != 0)
	{
		recvVec.push_back(cvBoundingRect(contours));
		contours = contours->h_next;
	}

	int position_x1,position_y1,position_x2,position_y2;
	int rec_width,rec_height;
	CvRect res_rec;
	res_rec.x = -1;

	//初步筛选
	for(int i=0;i<recvVec.size();i++)
	{
		int whRatio = recvVec[i].width/recvVec[i].height;
		int width = recvVec[i].width;
		int height = recvVec[i].height;
		float xPosition = recvVec[i].x/(float)src->width;

		if(height >= 15 && height <= 60)
			if(xPosition >= 0.2 && xPosition <=0.8)
			{
				//cvDrawRect(src,cvPoint(recvVec[i].x,recvVec[i].y),cvPoint(recvVec[i].x + recvVec[i].width,recvVec[i].y+recvVec[i].height),cvScalar(255,255,0));
				recvVec_temp.push_back(recvVec[i]);
			}
		
	}
	cvNamedWindow("1");
	cvShowImage("1",src);
	cvWaitKey(0);
	cvDestroyWindow("1");

	//合并矩形
	while(!recvVec_temp.empty())
	{
		if(recvVec_temp.size() == 1)
		{
			recvVec_corr.push_back(recvVec_temp[0]);
			break;
		}
		//排序，从左到右，从上到下
		sort(recvVec_temp.begin(),recvVec_temp.end(),cmpRect);
		for(int i=1;i<recvVec_temp.size();i++)
		{
			if(abs(recvVec_temp[i].x-recvVec_temp[0].x-recvVec_temp[0].width) > 20)
			{
				recvVec_corr.push_back(recvVec_temp[0]);
				recvVec_temp.erase(recvVec_temp.begin());
				break;
			}
			if(recvVec_temp[0].y > recvVec_temp[i].y)
			{
				if(recvVec_temp[i].y+recvVec_temp[i].height < recvVec_temp[0].y)
				{
					if(i == recvVec_temp.size()-1)
					{
						recvVec_corr.push_back(recvVec_temp[0]);
						recvVec_temp.erase(recvVec_temp.begin());
						break;
					}
				}
				else if(recvVec_temp[i].y+recvVec_temp[i].height > recvVec_temp[0].y+recvVec_temp[0].height)
				{
					if(recvVec_temp[0].height >= 15)
					{
						recvVec_temp[i].width = recvVec_temp[i].x - recvVec_temp[0].x + recvVec_temp[i].width;
						recvVec_temp[i].height = recvVec_temp[i].height;
						recvVec_temp[i].x = recvVec_temp[0].x;
						recvVec_temp.erase(recvVec_temp.begin());
						break;
					}
					else
					{
						if(i == recvVec_temp.size()-1)
						{
							recvVec_corr.push_back(recvVec_temp[0]);
							recvVec_temp.erase(recvVec_temp.begin());
							break;
						}
					}
				}
				else
				{
					if(recvVec_temp[i].y+recvVec_temp[i].height-recvVec_temp[0].y >= 15)
					{
						recvVec_temp[i].width = recvVec_temp[i].x - recvVec_temp[0].x + recvVec_temp[i].width;
						recvVec_temp[i].height = recvVec_temp[i].height+recvVec_temp[0].height-(recvVec_temp[i].y+recvVec_temp[i].height-recvVec_temp[0].y);
						recvVec_temp[i].x = recvVec_temp[0].x;
						recvVec_temp.erase(recvVec_temp.begin());
						break;
					}
					else
					{
						if(i == recvVec_temp.size()-1)
						{
							recvVec_corr.push_back(recvVec_temp[0]);
							recvVec_temp.erase(recvVec_temp.begin());
							break;
						}
					}
				}

			}
			else
			{
				if(recvVec_temp[0].y+recvVec_temp[0].height < recvVec_temp[i].y)
				{
					if(i == recvVec_temp.size()-1)
					{
						recvVec_corr.push_back(recvVec_temp[0]);
						recvVec_temp.erase(recvVec_temp.begin());
						break;
					}
				}
				else if(recvVec_temp[i].y+recvVec_temp[i].height < recvVec_temp[0].y+recvVec_temp[0].height)
				{
					if(recvVec_temp[i].height >= 15)
					{
						recvVec_temp[i].width = recvVec_temp[i].x - recvVec_temp[0].x + recvVec_temp[i].width;
						recvVec_temp[i].height = recvVec_temp[i].height;
						recvVec_temp[i].x = recvVec_temp[0].x;
						recvVec_temp[i].y = recvVec_temp[0].y;
						recvVec_temp.erase(recvVec_temp.begin());
						break;
					}
					else
					{
						if(i == recvVec_temp.size()-1)
						{
							recvVec_corr.push_back(recvVec_temp[0]);
							recvVec_temp.erase(recvVec_temp.begin());
							break;
						}
					}
				}
				else
				{
					if(recvVec_temp[0].y+recvVec_temp[0].height-recvVec_temp[i].y >= 15)
					{
						recvVec_temp[i].width = recvVec_temp[i].x - recvVec_temp[0].x + recvVec_temp[i].width;
						recvVec_temp[i].height = recvVec_temp[i].height+recvVec_temp[0].height - (recvVec_temp[0].y+recvVec_temp[0].height-recvVec_temp[i].y);
						recvVec_temp[i].x = recvVec_temp[0].x;
						recvVec_temp[i].y = recvVec_temp[0].y;
						recvVec_temp.erase(recvVec_temp.begin());
						break;
					}
					else
					{
						if(i == recvVec_temp.size()-1)
						{
							recvVec_corr.push_back(recvVec_temp[0]);
							recvVec_temp.erase(recvVec_temp.begin());
							break;
						}
					}
				}
			}
		}
	}
	
// 	for(int i=0;i<recvVec_corr.size();i++)
// 	{
// 		cvDrawRect(src,cvPoint(recvVec_corr[i].x,recvVec_corr[i].y),cvPoint(recvVec_corr[i].x + recvVec_corr[i].width,recvVec_corr[i].y+recvVec_corr[i].height),cvScalar(0,255,255));
// 	}

	//精确筛选
	for(int i=0;i<recvVec_corr.size();i++)
	{
		int whRatio = recvVec_corr[i].width/recvVec_corr[i].height;
		int width = recvVec_corr[i].width;
		int height = recvVec_corr[i].height;
		float xPosition = recvVec_corr[i].x/(float)src->width;
		float yPosition = recvVec_corr[i].y/(float)src->height;

		if(whRatio >= 2 && whRatio <=8)
			if(width >= 100 && width <= 180)
				{
					position_x1 = recvVec_corr[i].x;
					position_x2 = recvVec_corr[i].x+width;
					position_y1 = recvVec_corr[i].y;
					position_y2 = recvVec_corr[i].y+height;
					rec_width = recvVec_corr[i].width;
					rec_height = recvVec_corr[i].height;
					res_rec = recvVec_corr[i];
					//cvDrawRect(src,cvPoint(recvVec_corr[i].x,recvVec_corr[i].y),cvPoint(recvVec_corr[i].x + recvVec_corr[i].width,recvVec_corr[i].y+recvVec_corr[i].height),cvScalar(0,0,255));
					break;
				}
	}


// 	cvNamedWindow("1",0);
// 	cvShowImage("1",src);
// 	cvWaitKey(0);
// 	cvDestroyWindow("1");

	IplImage *pcarId = cvCreateImage(cvSize(position_x2-position_x1,position_y2-position_y1),src->depth,src->nChannels);
	cvSetImageROI(src,cvRect(position_x1,position_y1,position_x2-position_x1,position_y2-position_y1));
	cvCopy(src,pcarId);
	cvResetImageROI(src);
	cvSaveImage("res.jpg",pcarId);


	cvReleaseImage(&gray);
	cvReleaseImage(&sobel);
	cvReleaseImage(&erodeDilate);
	cvReleaseImage(&temp);
	cvReleaseImage(&pcarId);

	return res_rec;
}

bool cmpRect(CvRect a,CvRect b)
{
	if(a.x != b.x)
		return a.x < b.x;
	else
		return a.height < b.height;
}

int getMax5Projection_x(IplImage *src)
{
	int x,y;
	CvScalar s;

	int *h = new int[src->height];
	memset(h,0,src->height*4);
	for(y=0;y<src->height;y++)
	{
		for(x=0;x<src->width;x++)
		{
			s = cvGet2D(src,y,x);
			if(s.val[0] == 255)
				h[y]++;
		}
	}
	sort(h,h+src->height);
	int sum = 0;

	for(x=5;x>0;x--)
		sum += h[src->height - x];

	delete [] h;
	return sum;
}

IplImage* getCorrectImage_x(double angle,IplImage *src)
{
	double angle_rad;
	IplImage *res;

	angle_rad = (CV_PI* (angle/180));
	res = cvCreateImage(cvGetSize(src),src->depth,src->nChannels);
	cvFillImage(res,0);
	float m[6];            
	CvMat M = cvMat( 2, 3, CV_32F, m );

	CvPoint2D32f center;
	center.x=float (src->width/2.0+0.5);
	center.y=float (src->height/2.0+0.5);  
	cv2DRotationMatrix( center, angle,1, &M);

	cvWarpAffine( src, res, &M,CV_INTER_LINEAR ,cvScalarAll(0) );
	return res;
}

IplImage* getCorrectImage_x_m(double angle,IplImage *src,CvMat *map)
{
	double angle_rad;
	IplImage *res;

	angle_rad = (CV_PI* (angle/180));
	res = cvCreateImage(cvGetSize(src),src->depth,src->nChannels);
	cvFillImage(res,0);

	CvPoint2D32f center;
	center.x=float (src->width/2.0+0.5);
	center.y=float (src->height/2.0+0.5);  
	cv2DRotationMatrix( center, angle,1, map);

	cvWarpAffine( src, res, map,CV_INTER_LINEAR ,cvScalarAll(0) );
	return res;
}

IplImage* getCorrectImage_x(CvMat *map,IplImage *src)
{
	double angle_rad;
	IplImage *res;

	res = cvCreateImage(cvGetSize(src),src->depth,src->nChannels);
	cvFillImage(res,0);
	cvWarpAffine( src, res, map,CV_INTER_LINEAR ,cvScalarAll(0) );
	return res;
}

int getMax5Projection_y(IplImage *src)
{
	int x,y;
	CvScalar s,t;

	int *v = new int[src->width];
	memset(v,0,src->width*4);

	for(x=0;x<src->width;x++)  
	{  
		for(y=0;y<src->height;y++)  
		{  
			s=cvGet2D(src,y,x);           
			if(s.val[0]==255)  
				v[x]++;                   
		}         
	} 

	sort(v,v+src->width);
	int sum = 0;

	for(x=5;x>0;x--)
	{
		sum += v[src->width - x];
	}

	delete [] v;
	return sum;
}

IplImage* getCorrectImage_y(double degree,IplImage *imgSrc)
{
	double angle = degree  * CV_PI / 180.;   

	int w_src = imgSrc->width;  
	int h_src = imgSrc->height;  

	CvPoint2D32f src_point[4];  
	CvPoint2D32f dst_point[4];  
	src_point[0].x=-w_src/2;  
	src_point[0].y=-h_src/2;  
	dst_point[0].x=-w_src/2-h_src/2*sin(angle)*cos(angle);  
	dst_point[0].y=-h_src/2*cos(angle)*cos(angle);  
	src_point[1].x=w_src/2;  
	src_point[1].y=-h_src/2;  
	dst_point[1].x=w_src/2-h_src/2*sin(angle)*cos(angle);  
	dst_point[1].y=-h_src/2*cos(angle)*cos(angle);  
	src_point[2].x=w_src/2;  
	src_point[2].y=h_src/2;  
	dst_point[2].x=w_src/2+h_src/2*sin(angle)*cos(angle);  
	dst_point[2].y=h_src/2*cos(angle)*cos(angle);  
	src_point[3].x=-w_src/2;  
	src_point[3].y=h_src/2;  
	dst_point[3].x=-w_src/2+h_src/2*sin(angle)*cos(angle);;  
	dst_point[3].y=h_src/2*cos(angle)*cos(angle);  
	float m[6];  
	CvMat M= cvMat(2, 3, CV_32F, m);  
	cvGetAffineTransform( src_point, dst_point,&M);  

	IplImage *imgDst2 = cvCreateImage(cvSize(w_src, h_src), imgSrc->depth, imgSrc->nChannels); 
	cvFillImage(imgDst2,0);	
	cvWarpAffine(imgSrc,imgDst2,&M,CV_INTER_LINEAR,cvScalarAll(0));  

	return imgDst2;
}

IplImage* getCorrectImage_y_m(double degree,IplImage *imgSrc,CvMat *map)
{
	double angle = degree  * CV_PI / 180.;   

	int w_src = imgSrc->width;  
	int h_src = imgSrc->height;  

	CvPoint2D32f src_point[4];  
	CvPoint2D32f dst_point[4];  
	src_point[0].x=-w_src/2;  
	src_point[0].y=-h_src/2;  
	dst_point[0].x=-w_src/2-h_src/2*sin(angle)*cos(angle);  
	dst_point[0].y=-h_src/2*cos(angle)*cos(angle);  
	src_point[1].x=w_src/2;  
	src_point[1].y=-h_src/2;  
	dst_point[1].x=w_src/2-h_src/2*sin(angle)*cos(angle);  
	dst_point[1].y=-h_src/2*cos(angle)*cos(angle);  
	src_point[2].x=w_src/2;  
	src_point[2].y=h_src/2;  
	dst_point[2].x=w_src/2+h_src/2*sin(angle)*cos(angle);  
	dst_point[2].y=h_src/2*cos(angle)*cos(angle);  
	src_point[3].x=-w_src/2;  
	src_point[3].y=h_src/2;  
	dst_point[3].x=-w_src/2+h_src/2*sin(angle)*cos(angle);;  
	dst_point[3].y=h_src/2*cos(angle)*cos(angle);  
	
	cvGetAffineTransform( src_point, dst_point,map);  

	IplImage *imgDst2 = cvCreateImage(cvSize(w_src, h_src), imgSrc->depth, imgSrc->nChannels); 
	cvFillImage(imgDst2,0);	
	cvWarpAffine(imgSrc,imgDst2,map,CV_INTER_LINEAR,cvScalarAll(0));  

	return imgDst2;
}

IplImage* getCorrectImage_y(CvMat *map,IplImage *imgSrc)
{
	IplImage *imgDst2 = cvCreateImage(cvGetSize(imgSrc), imgSrc->depth, imgSrc->nChannels); 
	cvFillImage(imgDst2,0);	
	cvWarpAffine(imgSrc,imgDst2,map,CV_INTER_LINEAR,cvScalarAll(0));  

	return imgDst2;
}

bool cmp(CENTERDIFF a,CENTERDIFF b)
{
	return a.diff < b.diff;
}

CvRect getAccurateCarId(IplImage *src)
{
	IplImage *gray = cvCreateImage(cvGetSize(src),IPL_DEPTH_8U,1);
	cvCvtColor(src,gray,CV_BGR2GRAY);
	cvThreshold(gray,gray,0,255,CV_THRESH_BINARY | CV_THRESH_OTSU);

	IplImage* paintx=cvCreateImage( cvGetSize(gray),IPL_DEPTH_8U, 1 );  
	IplImage* painty=cvCreateImage( cvGetSize(gray),IPL_DEPTH_8U, 1 );  
	cvZero(paintx);  
	cvZero(painty);  
	int* v=new int[src->width];  
	int* h=new int[src->height];  
	memset(v,0,src->width*4);  
	memset(h,0,src->height*4);  

	//水平投影
	int x,y;  
	CvScalar s,t,z; 
	for(y=0;y<src->height;y++)  
	{  
		for(x=0;x<src->width;x++)  
		{  
			s=cvGet2D(gray,y,x);           
			if(s.val[0]==255)  
				h[y]++;       
		}     
	}  
	for(y=0;y<src->height;y++)  
	{  
		for(x=0;x<h[y];x++)  
		{             
			t.val[0]=255;  
			cvSet2D(painty,y,x,t);            
		}         
	}  
		
	//确定上下边界
	int sum = 0;
	for(int i=0;i<src->height;i++)
	{
		sum += h[i];
	}
	sum /= src->height;

	for(int i=0;i<src->height;i++)
		cvSet2D(painty,i,sum,t);

	int upperFen=-1,lowerFen=-1;
	int flag = 0;

	for(int i=src->height/2;i>=0;i--)
	{
		if(h[i]<sum/2)
		{
			
			upperFen = i;
			break;
		}
	}
	for(int i=src->height/2+1;i<src->height;i++)
	{
		if(h[i]<sum/2)
		{
			
			lowerFen = i;
			break;
		}
	}

	t.val[1] = 255;
	t.val[2] = 255;
	z.val[0]=0;z.val[1]=0;z.val[2]=0;
	
	for(int i=0;i<src->width;i++)
	{
		if(upperFen != -1)
		{
			cvSet2D(src,upperFen,i,t);
		}
		if(lowerFen != -1)
		{
			cvSet2D(src,lowerFen,i,t);
		}
	}
	if(upperFen != -1)
	{
		for(int i=0;i<=upperFen;i++)
		{
			for(int j=0;j<src->width;j++)
			{
				cvSet2D(src,i,j,z);
				cvSet2D(gray,i,j,z);
			}
		}
	}
	if(lowerFen != -1)
	{
		for(int i=lowerFen;i<src->height;i++)
		{
			for(int j=0;j<src->width;j++)
			{
				cvSet2D(src,i,j,z);
				cvSet2D(gray,i,j,z);
			}
		}
	}

	//垂直投影
	for(x=0;x<src->width;x++)  
	{  
		for(y=0;y<src->height;y++)  
		{  
			s=cvGet2D(gray,y,x);           
			if(s.val[0]==255)  
				v[x]++;                   
		}         
	}  

	for(x=0;x<src->width;x++)  
	{  
		for(y=src->height-v[x];y<src->height;y++)  
		{         
			t.val[0]=255;  
			cvSet2D(paintx,y,x,t);        
		}         
	}  
	//确定左右边界
	sum = 0;
	for(int i=0;i<src->width;i++)
	{
		sum += v[i];
	}
	sum /= src->width;
	for(int i=0;i<src->width;i++)
	{
		cvSet2D(paintx,src->height - sum,i,t);
	}
	for(int i=0;i<src->width;i++)
	{
		if(v[i] < sum/3)
		{
			v[i] = 0;
		}
	}
	cvZero(paintx);
	for(x=0;x<src->width;x++)  
	{  
		for(y=src->height-v[x];y<src->height;y++)  
		{         
			t.val[0]=255;  
			cvSet2D(paintx,y,x,t);        
		}         
	}  
	for(int i=0;i<src->width;i++)
	{
		cvSet2D(paintx,src->height - sum,i,t);
	}

	int leftFen=-1,rightFen=-1;
	flag = 0;
	int tempLen = 0;

	if(v[0] != 0)
		flag = 1;
	for(int i=0;i<src->width/2;i++)
	{
		if(flag == 0)
		{
			if(v[i] == 0)
			{
				flag = 0;
				leftFen = i;
				tempLen = 0;
			}
			else
			{
				flag = 1;
				tempLen++;
			}
		}
		else
		{
			if(v[i] == 0)
			{
				flag = 0;
				if(tempLen <= 6)
				{
					leftFen = i;
					tempLen = 0;
				}
				else
				{
					//cout << "左边界 " << tempLen << endl;
					tempLen = 0;
					break;
				}
			}
			else
			{
				tempLen++;
			}
		}
		//cout << "flag: " << flag << " " << "len: " << tempLen << endl;
	}

	int oldrightFen = 0;
	int oldtempLen = 0;
	int tempLen2 = 0;
	tempLen = 0;
	flag = 0;
	if(v[src->width-1] != 0)
		flag = 1;
	for(int i=src->width-1;i>src->width/2;i--)
	{
		if(flag == 0)
		{
			if(v[i] == 0)
			{
				flag = 0;
				rightFen = i;
				tempLen = 0;
				tempLen2++;
			}
			else
			{
				flag = 1;
				tempLen++;
				if(tempLen2 >= 6 && oldtempLen>=3)
				{
					rightFen = oldrightFen;
					break;
				}
				tempLen2 = 0;
			}
		}
		else
		{
			if(v[i] == 0)
			{
				flag = 0;
				tempLen2 = 0;
				if(tempLen <= 6)
				{
					oldrightFen = rightFen;
					rightFen = i;
					oldtempLen = tempLen;
					tempLen = 0;
				}
				else
				{
					//cout << "右边界 " << tempLen << endl;
					tempLen = 0;
					break;
				}
			}
			else
			{
				tempLen++;
			}
		}
		//cout << "flag: " << flag << " " << "len: " << tempLen << endl;
	}

// 	for(int i=0;i<src->height;i++)
// 	{
// 		if(leftFen != -1)
// 			cvSet2D(src,i,leftFen,t);
// 		if(rightFen != -1)
// 			cvSet2D(src,i,rightFen,t);
// 	}

	if(leftFen != -1)
	{
		for(int i=0;i<src->height;i++)
		{
			for(int j=0;j<leftFen;j++)
			{
				cvSet2D(src,i,j,z);
			}
		}
	}
	if(rightFen != -1)
	{
		for(int i=0;i<src->height;i++)
		{
			for(int j=rightFen;j<src->width;j++)
			{
				cvSet2D(src,i,j,z);
			}
		}
	}

	delete [] v;
	delete [] h;
	cvReleaseImage(&paintx);
	cvReleaseImage(&painty);
	cvReleaseImage(&gray);
	
	if(leftFen == -1)
		leftFen = 0;
	if(rightFen == -1)
		rightFen = src->width;
	if(upperFen == -1)
		upperFen = 0;
	if(lowerFen == -1)
		lowerFen = src->height;

	CvRect rect;
	rect.x = leftFen;
	rect.y = upperFen;
	rect.height = lowerFen - upperFen;
	rect.width = rightFen - leftFen;

	
	IplImage *pres = cvCreateImage(cvSize(rect.width,rect.height),src->depth,src->nChannels);
	cvSetImageROI(src,cvRect(rect.x,rect.y,rect.width,rect.height));
	cvCopy(src,pres);
	cvResetImageROI(src);



	cvNamedWindow("1");
	cvShowImage("1",pres);
	cvWaitKey(0);
	cvDestroyWindow("1");

	cvSaveImage("ress.jpg",pres);

	cvReleaseImage(&pres);
	return rect;
}