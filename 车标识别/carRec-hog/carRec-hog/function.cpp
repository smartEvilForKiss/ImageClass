#include "function.h"




CvRect getCaridPosition(IplImage *src)
{

	vector<CvRect> m_vec1,m_vec2;
	m_vec1 = getCaridPosition_method1(src);
	if(m_vec1.size() == 1)
	{
		return m_vec1[0];
	}
	m_vec2 = getCaridPosition_method2(src);
	if(m_vec1.size() == 0)
	{
		if(m_vec2.size() == 0)
		{
			CvRect rect;
			rect.x = -1;
			return rect;
		}
		else
			return m_vec2[0];
	}

	if(m_vec2.size() == 0)
	{
		return m_vec1[0];
	}

	for(int i=0;i<m_vec1.size();i++)
	{
		for(int j=0;j<m_vec2.size();j++)
		{
			if(!((m_vec2[j].x > m_vec1[i].x+m_vec1[i].width) || (m_vec2[j].y>m_vec1[i].y+m_vec1[i].height) || (m_vec1[i].x > m_vec2[j].x+m_vec2[j].width) || (m_vec1[i].y>m_vec2[j].y+m_vec2[j].height)))
			{
				return m_vec1[i];
			}

		}
	}
	CvRect rect;
	rect.x = -1;
	return rect;
}

void getHSI(IplImage *src,IplImage *hsi_i,IplImage *hsi_s,IplImage *hsi_h)
{
	int step,step_hsi,channels, cd, cdhsi, b, g, r;
	uchar *data, *data_i, *data_s, *data_h;
	int i,j;
	double min_rgb, add_rgb, theta, den, num;

	step     =      src -> widthStep;                                      //存储同列相邻行之间的比特数
	channels =      src -> nChannels;
	data = (uchar *)src -> imageData;                                      //存储指向图象数据的指针
	step_hsi =          hsi_i -> widthStep;                                  //亮度图象的相邻行之间的比特

	data_i = (uchar *)hsi_i -> imageData;                                    //存储指向子图象的数据指针
	data_s = (uchar *)hsi_s->imageData;
	data_h = (uchar *)hsi_h ->imageData;

	for(i=0;i < src->height;i++)
		for(j=0;j < src->width ;j++){
			cd = i*step + j*channels;                                                //计算取元图象数据的位置
			cdhsi = i*step_hsi + j;                                                  //计算子图象数据存储的位置
			b = data[cd], g = data[cd + 1], r = data[cd + 2 ];                       
			data_i[cdhsi] = (int)((r + g + b)/ 3 );                                  //计算亮度子图象
			min_rgb = __min(__min(r,g),b);                                           //取最小值运算
			add_rgb = r + g + b ;
			data_s[cdhsi] = (int) (100 - 300*min_rgb/add_rgb);                       //饱和度S的范围显示为0～255，便于显示
			num = 0.5*((r - g) + (r - b));                                           //下面的式子计算图象的色彩H
			den =  sqrt ((double)((r - g)*(r - g) + (r - b)*(g - b)));

			if ( 0 == den )
				den = 0.01;
			theta = acos( num /den );

			if (b <= g)
				data_h[cdhsi] = (int)(theta*360/(2 * 3.14));
			else 
				data_h[cdhsi] = (int)(360 - theta*360/(2 * 3.14) );
			if (data_s[cdhsi] == 0 )
				data_h[cdhsi] = 0;
		}
}

void setBinH(IplImage *hsi_h,IplImage *binImage,int lowFen,int maxFen)
{
	uchar *data1 = (uchar *)hsi_h->imageData;
	uchar *data2 = (uchar *)binImage->imageData;

	int step = hsi_h->widthStep/sizeof(uchar);
	for(int i=0;i<hsi_h->height;i++)
		for(int j=0;j<hsi_h->width;j++)
		{
			if(data1[i*step+j]>=lowFen && data1[i*step+j]<=maxFen)
				data2[i*step+j] = 255;
			else
				data2[i*step+j] = 0;
		}
}

void setBinS(IplImage *hsi_s,IplImage *binImage,int fen)
{
	uchar *data1 = (uchar *)hsi_s->imageData;
	uchar *data2 = (uchar *)binImage->imageData;

	int step = hsi_s->widthStep/sizeof(uchar);
	for(int i=0;i<hsi_s->height;i++)
		for(int j=0;j<hsi_s->width;j++)
		{
			if(data1[i*step+j]>=fen)
				data2[i*step+j] = 255;
			else
				data2[i*step+j] = 0;
		}
}

vector<CvRect> getCaridPosition_method1(IplImage *src)
{
	IplImage *gray,*sobel,*erodeDilate,*temp;
	vector<CvRect> recvVec;
	vector<CvRect> recvVec_temp;
	vector<CvRect> recvVec_corr;
	vector<CvRect> res_method1;

	gray = cvCreateImage(cvGetSize(src),IPL_DEPTH_8U,1);
	temp = cvCreateImage(cvGetSize(src),IPL_DEPTH_8U,1);
	erodeDilate = cvCreateImage(cvGetSize(src),IPL_DEPTH_8U,1);
	sobel = cvCreateImage(cvGetSize(src),IPL_DEPTH_16S,1);

	cvCvtColor(src,gray,CV_BGR2GRAY);
	cvSobel(gray,sobel,2,0,7);
	cvConvertScale(sobel,temp,0.005,0);
	cvThreshold(temp,temp,0,255,CV_THRESH_BINARY_INV | CV_THRESH_OTSU);

	//膨胀和腐蚀
	IplConvKernel *kernel = cvCreateStructuringElementEx(3,1,1,0,CV_SHAPE_RECT);
	cvErode(temp,erodeDilate,kernel,7);
	cvDilate(erodeDilate,erodeDilate,kernel,7);

	cvReleaseStructuringElement(&kernel);

	kernel = cvCreateStructuringElementEx(1,3,0,1,CV_SHAPE_RECT);
	cvErode(erodeDilate,erodeDilate,kernel,2);
	cvDilate(erodeDilate,erodeDilate,kernel,2);

	cvReleaseStructuringElement(&kernel);

	cvSmooth(erodeDilate,erodeDilate,CV_MEDIAN,7,7);
	cvNot(erodeDilate,erodeDilate);

	//查找轮廓
	CvMemStorage *storage = cvCreateMemStorage();
	CvSeq *contours;
	cvFindContours(erodeDilate,storage,&contours);

	while(contours != 0)
	{
		recvVec.push_back(cvBoundingRect(contours));
		contours = contours->h_next;
	}

	cvReleaseMemStorage(&storage);
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
				recvVec_temp.push_back(recvVec[i]);
			}

	}

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
			if(abs(recvVec_temp[i].x-recvVec_temp[0].x-recvVec_temp[0].width) >= 18)
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
				//cvDrawRect(src,cvPoint(recvVec_corr[i].x,recvVec_corr[i].y),cvPoint(recvVec_corr[i].x + recvVec_corr[i].width,recvVec_corr[i].y+recvVec_corr[i].height),cvScalar(0,0,255));
				res_method1.push_back(recvVec_corr[i]);
			}
	}

	cvReleaseImage(&gray);
	cvReleaseImage(&sobel);
	cvReleaseImage(&erodeDilate);
	cvReleaseImage(&temp);

	return res_method1;
}

vector<CvRect> getCaridPosition_method2(IplImage *src)
{
	IplImage *hsi_h,*hsi_s,*hsi_i,*bin_image;
	hsi_i = cvCreateImage( cvGetSize(src), src->depth, 1 );    //创建亮度图象
	hsi_s = cvCreateImage( cvGetSize(src), src->depth, 1 );    //创建饱和度图象
	hsi_h = cvCreateImage( cvGetSize(src), src->depth, 1 );    //创建色彩图象
	bin_image = cvCreateImage(cvGetSize(src),src->depth,1);
	getHSI(src,hsi_i,hsi_s,hsi_h);
	setBinH(hsi_h,bin_image,220,245); 
	setBinS(hsi_s,bin_image,20);

	IplConvKernel *kernal = cvCreateStructuringElementEx(3,1,1,0,CV_SHAPE_RECT);
	cvDilate(bin_image,bin_image,kernal,7);
	cvErode(bin_image,bin_image,kernal,7);
	cvReleaseStructuringElement(&kernal);

	kernal = cvCreateStructuringElementEx(1,3,0,1,CV_SHAPE_RECT);
	cvErode(bin_image,bin_image,kernal,2);
	cvDilate(bin_image,bin_image,kernal,2);
	cvReleaseStructuringElement(&kernal);

	CvMemStorage *storage = cvCreateMemStorage();
	CvSeq *contours;
	cvFindContours(bin_image,storage,&contours);

	vector<CvRect> recvVec;
	vector<CvRect> res_method2;

	while(contours != 0)
	{
		recvVec.push_back(cvBoundingRect(contours));
		contours = contours->h_next;
	}
	cvReleaseMemStorage(&storage);

	//初步筛选
	for(int i=0;i<recvVec.size();i++)
	{
		int whRatio = recvVec[i].width/recvVec[i].height;
		int width = recvVec[i].width;
		int height = recvVec[i].height;
		float xPosition = recvVec[i].x/(float)src->width;

		if(height >= 15 && height <= 60)
			if(xPosition >= 0.1 && xPosition <=0.9)
				if(whRatio >= 2 && whRatio <=8)
					if(width >= 90 && width <= 180)
					{
						//cvDrawRect(src,cvPoint(recvVec[i].x,recvVec[i].y),cvPoint(recvVec[i].x + recvVec[i].width,recvVec[i].y+recvVec[i].height),cvScalar(0,0,255));
						res_method2.push_back(recvVec[i]);
					}
	}


	cvReleaseImage(&hsi_h);
	cvReleaseImage(&hsi_s);
	cvReleaseImage(&hsi_i);
	cvReleaseImage(&bin_image);

	return res_method2;
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

IplImage* getbrandrect_big(CvRect cvrect,IplImage *src)
{
	int x1,x2,y1,y2;
	
	x1 = cvrect.x;
	x2 = cvrect.x + cvrect.width;
	y2 = cvrect.y;
	y1 = y2 - 6*cvrect.height;
	y1 = y1>0?y1:0;

	IplImage *res = cvCreateImage(cvSize(x2-x1,y2-y1),src->depth,src->nChannels);
	cvSetImageROI(src,cvRect(x1,y1,x2-x1,y2-y1));
	cvCopy(src,res);
	cvResetImageROI(src);

	return res;
}

IplImage* getbrandrect_small(double factor,IplImage *brand_big,BOUND &centerBound)
{
	IplImage *grayImage = cvCreateImage(cvGetSize(brand_big),IPL_DEPTH_8U,1);
	IplImage *sobel_x = cvCreateImage(cvGetSize(brand_big),IPL_DEPTH_16S,1);
	IplImage *sobel_y = cvCreateImage(cvGetSize(brand_big),IPL_DEPTH_16S,1);
	IplImage *sobel_x_res = cvCreateImage(cvGetSize(brand_big),IPL_DEPTH_8U,1);
	IplImage *sobel_y_res = cvCreateImage(cvGetSize(brand_big),IPL_DEPTH_8U,1);

	cvCvtColor(brand_big,grayImage,CV_BGR2GRAY);
	cvCanny(grayImage,sobel_x_res,50,100);

	cvSobel(grayImage,sobel_y,0,1,5);
	cvConvertScale(sobel_y,sobel_y_res,1,0);
	cvThreshold(sobel_y_res,sobel_y_res,50,255,CV_THRESH_BINARY);
	
	uchar* data1=(uchar *)sobel_x_res->imageData;
	uchar* data2=(uchar *)sobel_y_res->imageData;
	int step = sobel_x_res->widthStep/sizeof(uchar);    
	for(int i=0;i<sobel_x_res->height;i++)
		for(int j=0;j<sobel_x_res->width;j++)
		{
			if(data1[i*step+j] == 255 && data2[i*step+j] == 255)
			{
				data1[i*step+j] = 255;
			}
			else
			{
				data1[i*step+j] = 0;
			}
		}

	BOUND bound = getProject_x(sobel_x_res);
	int add_upper=0,add_lower=0;

	//将车牌上下边界补起到8的倍数
	if((bound.lowerbound - bound.upperbuond + 1)%8 != 0)
	{
		int op = 8 - (bound.lowerbound - bound.upperbuond + 1)%8;
		add_upper = op/2;
		add_lower = op - op/2;
	}

	bound.upperbuond -= add_upper;
	if(bound.upperbuond < 0)
		bound.upperbuond = 0;
	bound.lowerbound += add_lower;
	if(bound.lowerbound >= sobel_x_res->height)
		bound.lowerbound = sobel_x_res->height - 1;

	IplImage *x_center_img = cvCreateImage(cvSize(brand_big->width,bound.lowerbound-bound.upperbuond),brand_big->depth,brand_big->nChannels);
	cvSetImageROI(brand_big,cvRect(0,bound.upperbuond,brand_big->width,bound.lowerbound-bound.upperbuond));
	cvCopy(brand_big,x_center_img);
	cvResetImageROI(brand_big);
	
	//cout << "factor: " << factor << endl; 
	int newH = x_center_img->height*factor;
	int newW = x_center_img->width*factor;

	IplImage *factor_img = cvCreateImage(cvSize(newW,newH),x_center_img->depth,x_center_img->nChannels);
	cvResize(x_center_img,factor_img);

	//cout << factor_img->width << "---" <<factor_img->height << endl;
	centerBound = getCenter_y(factor_img);

	cvReleaseImage(&grayImage);
	cvReleaseImage(&sobel_x);
	cvReleaseImage(&sobel_y);
	cvReleaseImage(&sobel_x_res);
	cvReleaseImage(&sobel_y_res);
	cvReleaseImage(&x_center_img);


// 	cvNamedWindow("1");
// 	cvShowImage("1",factor_img);
// 	cvWaitKey(0);
// 	cvDestroyAllWindows();

	return factor_img;
}

BOUND getProject_x(IplImage *src)
{
	IplImage *paintx = cvCreateImage(cvGetSize(src),src->depth,src->nChannels);
	int x,y;
	CvScalar s,t;

	int *h = new int[src->height];
	memset(h,0,src->height*4);
	cvZero(paintx);

	for(y=0;y<src->height;y++)  
	{  
		for(x=0;x<src->width;x++)  
		{  
			s=cvGet2D(src,y,x);           
			if(s.val[0]==255)  

				h[y]++;       
		}     
	}  
	
	//去噪
	int ave = 0;
	for(y=0;y<src->height;y++)
	{
		ave += h[y];
	}
	ave /= src->height;
	for(y=0;y<src->height;y++)
	{
		h[y]=h[y]>ave?ave:h[y];
	}
	ave /= 4;
	for(y=0;y<src->height;y++)
	{
		h[y]=h[y]<ave?0:h[y];
	}

	///////////cvsmooth
	CvMat tempmat = cvMat(1,src->height,CV_32FC1,h);
	cvSmooth(&tempmat,&tempmat,CV_GAUSSIAN,3,3,2,2);

	for(y=0;y<src->height;y++)  
	{  
		for(x=0;x<h[y];x++)  
		{             
			t.val[0]=255;  
			cvSet2D(paintx,y,x,t);            
		}         
	}

	int y1=0,y2=0,temp1=0,temp2=0;
	int area = 0,temparea = 0;
	int flag = 0;
	for(y=0;y<src->height;y++)
	{
		if(flag == 0)//没有连续
		{
			if(h[y] == 0)
			{
				temp1 = temp2 = y;
			}
			else
			{
				temparea = 0;
				flag = 1;
				temp1 = temp2 = y;
				temparea += h[y];
			}
		}
		else//连续
		{
			if(h[y] == 0)
			{
				flag = 0;
				if(temparea > area)
				{
					y1 = temp1;
					y2 = y;
					area =temparea;
					temparea = 0;
				}
				temp1 = y;
			}
			else
			{
				temparea += h[y];
				if(y==src->height-1)
				{
					if(temparea > area)
					{
						y1 = temp1;
						y2 = y;
					}
				}
				
			}
		}
	}

	t.val[0] = 255;
	for(x=0;x<src->width;x++)
	{
		cvSet2D(paintx,y1,x,t);
		cvSet2D(paintx,y2,x,t);
	}


// 	cvNamedWindow("binary");
// 	cvNamedWindow("1");
// 	cvShowImage("binary",paintx);
// 	cvShowImage("1",src);
// 	cvWaitKey(0);
// 	cvReleaseImage(&paintx);
// 	cvDestroyWindow("1");
// 	cvDestroyWindow("binary");

	delete [] h;

	BOUND bound;
	bound.upperbuond = y1;
	bound.lowerbound = y2;
	return bound;
}

BOUND getCenter_y(IplImage *src)
{
	IplImage *gray_img = cvCreateImage(cvGetSize(src),IPL_DEPTH_8U,1);
	IplImage *full_edge = cvCreateImage(cvGetSize(src),IPL_DEPTH_8U,1);
	IplImage *x_edge = cvCreateImage(cvGetSize(src),IPL_DEPTH_8U,1);
	IplImage *y_edge = cvCreateImage(cvGetSize(src),IPL_DEPTH_8U,1);
	IplImage *sobel_x = cvCreateImage(cvGetSize(src),IPL_DEPTH_16S,1);
	IplImage *sobel_y = cvCreateImage(cvGetSize(src),IPL_DEPTH_16S,1);

	cvCvtColor(src,gray_img,CV_BGR2GRAY);
	cvCanny(gray_img,full_edge,50,150);

	cvSobel(gray_img,sobel_y,0,1,5);
	cvConvertScale(sobel_y,y_edge,1,0);
	cvThreshold(y_edge,y_edge,50,255,CV_THRESH_BINARY);

	cvSobel(gray_img,sobel_x,1,0,5);
	cvConvertScale(sobel_x,x_edge,1,0);
	cvThreshold(x_edge,x_edge,50,255,CV_THRESH_BINARY);

	uchar* data1=(uchar *)full_edge->imageData;
	uchar* data2=(uchar *)x_edge->imageData;
	uchar* data3=(uchar *)y_edge->imageData;
	int step = full_edge->widthStep/sizeof(uchar);    
	for(int i=0;i<full_edge->height;i++)
		for(int j=0;j<full_edge->width;j++)
		{
			if(data1[i*step+j] == 255 && data2[i*step+j] == 255)
			{
				data2[i*step+j] = 255;
			}
			else
			{
				data2[i*step+j] = 0;
			}

			if(data1[i*step+j] == 255 && data3[i*step+j] == 255)
			{
				data3[i*step+j] = 255;
			}
			else
			{
				data3[i*step+j] = 0;
			}
		}


	int position1 = getCenter_y_help(full_edge);
	int position2 = getCenter_y_help(x_edge);
	int position3 = getCenter_y_help(y_edge);

	position1 = 0.3*position1+0.35*position2+0.35*position3;


	BOUND bound;
	bound.lowerbound = position1;
	bound.upperbuond = 0;

	return bound;
}

int getCenter_y_help(IplImage *src)
{
	BOUND centerBound,dectBound;

	int center = src->width/2;
	int dectWidth = (src->width*3/(double)4)/2;
	centerBound.upperbuond = center - 15;
	centerBound.lowerbound = center + 15;
	dectBound.upperbuond = center - dectWidth;
	dectBound.lowerbound = center + dectWidth;
	

	IplImage* painty=cvCreateImage( cvGetSize(src),IPL_DEPTH_8U, 1 );  
	cvZero(painty);

	int* v=new int[src->width];  
	memset(v,0,src->width*4);  

	int x,y;  
	CvScalar s,t;  
	for(x=0;x<src->width;x++)  
	{  
	 for(y=0;y<src->height;y++)  
	 {  
		 s=cvGet2D(src,y,x);           
		 if(s.val[0]==255)  
			 v[x]++;                   
	 }         
	}  

	for(x=0;x<src->width;x++)  
	{  
	 for(y=src->height-v[x];y<src->height;y++) 
	 {         
		 t.val[0]=255;  
		 cvSet2D(painty,y,x,t);        
	 }         
	} 

	//滤波平滑
	CvMat tempmat = cvMat(1,src->width,CV_32FC1,v);
	cvSmooth(&tempmat,&tempmat,CV_GAUSSIAN,5,5,2,2);


	int maxvalue=0,fence=0;
	for(int i=0;i<src->width;i++)
	{
		if(v[i] > maxvalue)
		{
			maxvalue = v[i];
			fence = i;
		}
	}

	vector<CENTERDIFF> diffvec;
	CENTERDIFF tempdiff;
	int diff1,position1;
	int diff2,position2;

	//正投影
	for(int i=centerBound.upperbuond;i<=centerBound.lowerbound;i++)
	{
		int leftsum = 0;
		int rightsum = 0;
		for(int j=dectBound.upperbuond;j<i;j++)
		{
			leftsum += v[j];
		}
		for(int j=i+1;j<=dectBound.lowerbound;j++)
		{
			rightsum += v[j];
		}
		tempdiff.position = i;
		tempdiff.diff = abs(leftsum - rightsum);
		diffvec.push_back(tempdiff);
	}

	sort(diffvec.begin(),diffvec.end(),cmp);

	diff1 = diffvec[0].diff;position1 = diffvec[0].position;

	//反投影
	diffvec.clear();
	for(int i=dectBound.upperbuond;i<=dectBound.lowerbound;i++)
		v[i] = maxvalue - v[i];
	for(int i=centerBound.upperbuond;i<=centerBound.lowerbound;i++)
	{
		int leftsum = 0;
		int rightsum = 0;
		for(int j=dectBound.upperbuond;j<i;j++)
		{
			leftsum += v[j];
		}
		for(int j=i+1;j<=dectBound.lowerbound;j++)
		{
			rightsum += v[j];
		}
		tempdiff.position = i;
		tempdiff.diff = abs(leftsum - rightsum);
		diffvec.push_back(tempdiff);
	}

	sort(diffvec.begin(),diffvec.end(),cmp);
	diff2 = diffvec[0].diff;position2 = diffvec[0].position;
// 	cout << diff1 << " " << position1 << endl;
// 	cout << diff2 << " " << position2 << endl;

	if(diff1 == diff2)
	{
		position1 = (position1 + position2)/(double)2;
	}
	else if(diff1 < diff2)
	{
		;
	}
	else
	{
		position1 = 0.6*position1 + 0.4*position2;
	}

	for(int i=0;i<src->height;i++)
	{
		t.val[0]=255;  
		cvSet2D(src,i,position1,t);   
	}

	cvReleaseImage(&painty);
	delete [] v;
	return position1;
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
	rect.height = lowerFen - upperFen + 1;
	rect.width = rightFen - leftFen + 1;
	return rect;
}

void getHogFeature(IplImage *src,vector<float> &feaVec)
{
	IplImage *scal = cvCreateImage(cvSize(64,128),src->depth,src->nChannels);
	cvResize(src,scal);

	HOGDescriptor *hog = new HOGDescriptor;
	hog->compute(scal,feaVec,Size(1,1),Size(0,0));
	delete hog;
	cvReleaseImage(&scal);
}

float getFeatureDistance(vector<float> &a,vector<float> &b)
{
	float sum = 0;
	for(int i=0;i<a.size();i++)
	{
		sum += (a[i]-b[i])*(a[i]-b[i]);
	}
	return sum;
}

void readHogFeature(char *filename,vector<float> &feaVec,int &width,int &height)
{
	int len;
	float *pdata;
	fstream infile(filename,ios::binary | ios::in);
	infile.read((char *)(&width),sizeof(int));
	infile.read((char *)(&height),sizeof(int));
	infile.read((char *)(&len),sizeof(int));
	pdata = new float[len];
	infile.read((char *)pdata,sizeof(float)*len);
// 	for(int i=0;i<len;i++)
// 	{
// 		feaVec.push_back(pdata[i]);
// 	}
	feaVec = vector<float>(pdata,pdata+len);
	delete [] pdata;
	infile.close();
}

void saveFeature(vector<float> &res,int width,int height,char *saveName)
{
	int len = res.size();
	fstream outfile(saveName,ios::binary | ios::out);
	outfile.write((char *)(&width),sizeof(int));
	outfile.write((char *)(&height),sizeof(int));
	outfile.write((char *)(&len),sizeof(int));
	for(int i=0;i<len;i++)
	{
		float temp = res[i];
		outfile.write((char *)(&temp),sizeof(float));
	}
	outfile.close();
}