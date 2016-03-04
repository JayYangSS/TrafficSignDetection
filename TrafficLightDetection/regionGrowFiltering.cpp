#include "std_tlr.h"
#define IS_CUTIMG 0

extern HOGDescriptor myHOG_vertical;
extern HOGDescriptor myHOG_horz;
extern HOGDescriptor TLRecHOG;
extern HOGDescriptor isTLHOG;

extern MySVM TLRecSVM;//ʶ���ɫ�źŵ�����SVM������
extern MySVM isTLSVM;//ʶ���Ƿ����źŵ�
extern bool TRAIN;
extern bool HORZ;

#if TEST
extern IplImage* tmpRegion2;
#endif

#if TEST
extern FILE* tmpfp;
#endif

#if 0
bool RegionGrowB(
	   int nSeedX, 
	   int nSeedY, 
	   unsigned char* pUnchInput,
	   int nWidth, 
	   int nHeight, 
	   unsigned char* pUnRegion,
	   int nThreshold,
	   CvRect &rect
	   );
#endif


bool BlackAroundLight(IplImage* srcImg,CvRect	iRect)
{
	bool returnStatus = false;
	int iWidth = srcImg->width,topX=iRect.x;
	int iHeight = srcImg->height,topY=iRect.y;
	int RectWidth,RectHeight;
	if(iRect.width<15)RectWidth=15;
	RectWidth=iRect.width;
	if(iRect.height<30)RectHeight=30;
	RectHeight=iRect.height;
	bool flag=false;



	//����SVM+HOG��������
	CvRect rect;
	if(topX+25>iWidth||topX-20<0)return false;
	else{
		rect.x=iRect.x-20;
		rect.width=45;
	}
	
	if(topY+40>iHeight||topY-30<0)return false;
	else{
		rect.y=iRect.y-30;
		rect.height=70;
	}


	cvSetImageROI(srcImg,rect);
	Mat SVMROI(srcImg);

	cvResetImageROI(srcImg);//��һ�����˾ͳ����˰�����
	if(HORZ)
		flag=BoxDetectTL(SVMROI,myHOG_horz,HORZ);
	flag=BoxDetectTL(SVMROI,myHOG_vertical,HORZ);
	//cvReleaseImage(&srcImg);
	return flag;
}


//not using hole traffic ligh as samples,just use the square light
int RecognizeLight(Mat segImg)
{	
	Mat edge,binImg;
	vector<vector<Point>> contour;
	const int cols = segImg.cols;
	const int rows = segImg.rows;
	const double y_epsTh = 1;
	const double x_epsTh = 1;
	int center_x = cols / 2;
	int center_y = rows / 2;
	double x_eps = 0;
	double y_eps = 0;

	//define the recognition result
	const int circular_go = 0;
	const int direction_left = 1;
	const int direction_right = 2;

	static int count = 0;

	//find contours
	threshold(segImg, binImg, 50, 255, THRESH_BINARY_INV);
	imshow("segImg",segImg);
	waitKey(1);
	imshow("binImg", binImg);
	waitKey(1);

	//save the binary image
	char img_name[50];
	sprintf_s(img_name, "D:\\Img\\%d.jpg", count);
	imwrite(img_name, binImg);
	waitKey(1);
	count++;

	Canny(binImg, edge, 0, 50, 5);
	
	imshow("edge", edge);
	waitKey(1);
	findContours(edge, contour, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	cout << "contour size:"<<contour.size() << endl;
	//use the convexity to determine whether it is the arrow TL
	bool isConvex = isContourConvex(contour[0]);
	if (isConvex){
		cout << "convex" << endl;
		return 0;//return the circular recognition result
	}
	
	//calculate the moments od the TL
	Moments TLMoments = moments(segImg, true);
	int gravityCenter_x = (int)(TLMoments.m10 / TLMoments.m00);
	int gravityCenter_y = (int)(TLMoments.m01 / TLMoments.m00);
	y_eps = gravityCenter_y - center_y;
	x_eps = gravityCenter_x - center_x;

	//return the recogintion result
	if (y_eps > y_epsTh&&abs(x_eps)<x_epsTh){
		return circular_go;
	}
	if (x_eps<(0 - x_epsTh) && abs(y_eps) < y_epsTh){
		return direction_left;//return the left arrow recognition result
	}
	if (x_eps>x_epsTh&&abs(y_eps) < y_epsTh){
		return direction_right;//return the right arrow recognition result
	}
}



int isTL(IplImage* srcImg,CvRect iRect)
{
	CvSize cutSize;
	cutSize.width=iRect.width;
	cutSize.height=iRect.height;
	IplImage *tmpCutImg=cvCreateImage(cutSize,srcImg->depth,srcImg->nChannels);
	GetImageRect(srcImg,iRect,tmpCutImg);
#if IS_CUTIMG
	cvShowImage("tmpCutImg",tmpCutImg);
	cvWaitKey(1);
#endif

	Mat cutMat(tmpCutImg);
	Mat tmpIsTL;
	vector<float> descriptor;

	//ʶ���źŵ����
	resize(cutMat,tmpIsTL,Size(TLREC_WIDTH,TLREC_HEIGHT));
	isTLHOG.compute(tmpIsTL,descriptor,Size(8,8));
	int DescriptorDim=descriptor.size();		
	Mat SVMTLRecMat(1,DescriptorDim,CV_32FC1);
	for(int i=0; i<DescriptorDim; i++)
		SVMTLRecMat.at<float>(0,i) = descriptor[i];

	int result=isTLSVM.predict(SVMTLRecMat);
	cvReleaseImage(&tmpCutImg);
	return result;
}






bool RegionGrowB2(
				  int nSeedX, 
				  int nSeedY, 
				  unsigned char* pUnchInput,
				  int nWidth, 
				  int nHeight, 
				  unsigned char* pUnRegion,
				  int nThreshold,
				  CvRect &rect
				  );


bool regionGrowFiltering(IplImage* inputImage,IplImage*srcImg,CvRect iRect,CvRect& oRect,vector<Rect>&found_filtered)//�ҳ��ľ��ο򱣴浽oRect
{
	bool returnStatus = false;
	int iWidth = inputImage->width;
	int iHeight = inputImage->height;
	int iWidthStep = inputImage->widthStep;
	unsigned char* pImageData = (unsigned char*)inputImage->imageData;

	//thresholding for graylevel differences between seedpoints and its neibours
	 const int thresholding = 22;//���ӵ������������ֵ֮�����ֵ

	int i=0,j=0;
	unsigned char* flag = new unsigned char[iWidth*iHeight];
	if(flag==NULL)
		return false;
	memset(flag,0,iWidth*iHeight);

	//���ο����ĵ�
	int seedX = iRect.x+iRect.width/2;
	int seedY = iRect.y+iRect.height/2;
	
	//pImageDataΪָ��ҶȻ�����ͼ����������׵�ַ��RegionGrowB2�������Խ����ص���С��maxSizeOfComponents��2000�����һҶ�ֵ
	//���С��thresholding��22���ľۼ������ҳ�
	if( RegionGrowB2(seedX,seedY,pImageData,iWidth,iHeight,flag,thresholding,oRect)){
		//if(oRect.width<=iRect.width*2 && oRect.height<=iRect.height*3/2)//�˴��������Ե���
		if(oRect.width<=iRect.width*3 && oRect.height<=iRect.height*3&&BlackAroundLight(srcImg,oRect))//�˴��������Ե���
			returnStatus = true;
	}

	if(flag!=NULL)
	{
	 delete []flag;
	 flag = NULL;
	}
	
	return returnStatus;
}


#if 0
bool RegionGrowB(
	   int nSeedX, 
	   int nSeedY, 
	   unsigned char* pUnchInput,
	   int nWidth, 
	   int nHeight, 
	   unsigned char* pUnRegion,
	   int nThreshold,
	   CvRect &rect
	   )
{

	const int maxSizeOfComponents = 10000;

	 int nDx[] = {-1,1,0,0};
	 int nDy[] = {0,0,-1,1};
	 int nSaveWidth = (nWidth+7)/8*8;
	  
	 // �����ջ���洢����
	 int * pnGrowQueX ;
	 int * pnGrowQueY ;

	 // ����ռ�
	 pnGrowQueX = new int [nWidth*nHeight];
	 pnGrowQueY = new int [nWidth*nHeight];

	 // �����ջ�������յ�
	 // ��nStart=nEnd, ��ʾ��ջ��ֻ��һ����
	 int nStart ;
	 int nEnd ;

	 //��ʼ��
	 nStart = 0 ;
	 nEnd = 0 ;

	 // �����ӵ������ѹ��ջ
	 pnGrowQueX[nEnd] = nSeedX;
	 pnGrowQueY[nEnd] = nSeedY;

	 // ��ǰ���ڴ��������
	 int nCurrX ;
	 int nCurrY ;

	 // ѭ�����Ʊ���
	 int k ;

	 // ͼ��ĺ�������,�����Ե�ǰ���ص�8������б���
	 int xx;
	 int yy;

	 while (nStart<=nEnd)
	 {
		  // ��ǰ���ӵ������
		  nCurrX = pnGrowQueX[nStart];
		  nCurrY = pnGrowQueY[nStart];

		  // �Ե�ǰ���4������б���
		  for (k=0; k<4; k++) 
		  { 
			   // 4�������ص�����
			   xx = nCurrX+nDx[k];
			   yy = nCurrY+nDy[k];

			   // �ж�����(xx��yy) �Ƿ���ͼ���ڲ�
			   // �ж�����(xx��yy) �Ƿ��Ѿ������
			   // pUnRegion[yy*nWidth+xx]==0 ��ʾ��û�д���

			   // �����������ж�����(xx��yy)�͵�ǰ����(nCurrX,nCurrY) ����ֵ��ľ���ֵ
			   if( (xx < nWidth) && (xx>=0) && (yy>=0) && (yy<nHeight) 
				   && (pUnRegion[yy*nWidth+xx]==0) 
				   && abs(pUnchInput[yy*nSaveWidth+xx] - pUnchInput[nCurrY*nSaveWidth+nCurrX])<nThreshold)  
			   {
					// ��ջ��β��ָ�����һλ
					nEnd++;

					if(nEnd > maxSizeOfComponents){
						printf("%d\n",nEnd);
						delete []pnGrowQueX;
						delete []pnGrowQueY;
						pnGrowQueX = NULL ;
						pnGrowQueY = NULL ;
						return false;

					}
	
				// ����(xx��yy) ѹ��ջ
				pnGrowQueX[nEnd] = xx;
				pnGrowQueY[nEnd] = yy;

				// ������(xx��yy)���ó��߼�1��255��
				// ͬʱҲ���������ش����
				pUnRegion[yy*nWidth+xx] = 255 ;
		   }
		  }
		  nStart++;
	 }
	    
	 
	 //�ҳ�����ķ�Χ
	 int nMinx=pnGrowQueX[0], nMaxx=pnGrowQueX[0], nMiny=pnGrowQueY[0], nMaxy = pnGrowQueY[0];
	 for (k=0; k<nEnd; k++)
	 {
	   if (pnGrowQueX[k] > nMaxx)
			 nMaxx = pnGrowQueX[k];
	   if (pnGrowQueX[k] < nMinx) 
			nMinx = pnGrowQueX[k];
	   if (pnGrowQueY[k] > nMaxy)
			nMaxy = pnGrowQueY[k];
	   if (pnGrowQueY[k] < nMiny) 
		   nMiny = pnGrowQueY[k];
	 }

	 rect.x=nMinx;
	 rect.y=nMiny;
	 rect.width=nMaxx-nMinx;
	 rect.height=nMaxy-nMiny;


		// �ͷ��ڴ�
	 delete []pnGrowQueX;
	 delete []pnGrowQueY;
	 pnGrowQueX = NULL ;
	 pnGrowQueY = NULL ;
	 return true;
}
#endif


bool RegionGrowB2(
				 int nSeedX, 
				 int nSeedY, 
				 unsigned char* pUnchInput,
				 int nWidth, 
				 int nHeight, 
				 unsigned char* pUnRegion,
				 int nThreshold,
				 CvRect &rect
				 )
{

	//const int maxSizeOfComponents = 2000;
	const int maxSizeOfComponents = 800;

	int nDx[] = {-1,1,0,0};
	int nDy[] = {0,0,-1,1};
	int nSaveWidth = (nWidth+7)/8*8;

	unsigned int valueSeedSum;
	valueSeedSum = pUnchInput[nSeedY*nSaveWidth+nSeedX];
	//printf("total:%u\n",valueSeedSum);
#if 0  //seedpoint is the average of center
	for(int i=0;i<4;i++)
	{
		//valueSeedX += pUnchInput[nSeedX+nDx[i]];
		//valueSeedY += pUnchInput[nSeedY+nDy[i]];
		valueSeedSum += pUnchInput[(nSeedY+nDy[i])*nSaveWidth+(nSeedX+nDx[i])];
		//printf("total:%u\n",valueSeedSum);
	}
	//valueSeedX /= 5;
	//valueSeedY /= 5;
	unsigned char valueSeed = valueSeedSum/5;
#endif
#if 1  //seedpoint is the center element
	unsigned char valueSeed = valueSeedSum;
#endif
	//printf("pointX:%d,pointY:%d,average:%u\n",nSeedX,nSeedY,valueSeed);

#if TEST
	fprintf(tmpfp,"pointX:%d,pointY:%d,average:%u\n",nSeedX,nSeedY,valueSeed);
#endif

	// �����ջ���洢����
	int * pnGrowQueX ;
	int * pnGrowQueY ;

	// ����ռ�
	pnGrowQueX = new int [nWidth*nHeight];
	pnGrowQueY = new int [nWidth*nHeight];

	// �����ջ�������յ�
	// ��nStart=nEnd, ��ʾ��ջ��ֻ��һ����
	int nStart ;
	int nEnd ;

	//��ʼ��
	nStart = 0 ;
	nEnd = 0 ;

	// �����ӵ������ѹ��ջ
	pnGrowQueX[nEnd] = nSeedX;
	pnGrowQueY[nEnd] = nSeedY;

	// ��ǰ���ڴ��������
	int nCurrX ;
	int nCurrY ;

	// ѭ�����Ʊ���
	int k ;

	// ͼ��ĺ�������,�����Ե�ǰ���ص�8������б���
	int xx;
	int yy;

	while (nStart<=nEnd)
	{
		// ��ǰ���ӵ������
		nCurrX = pnGrowQueX[nStart];
		nCurrY = pnGrowQueY[nStart];

		// �Ե�ǰ���4������б���
		for (k=0; k<4; k++) 
		{ 
			// 4�������ص�����
			xx = nCurrX+nDx[k];
			yy = nCurrY+nDy[k];

			// �ж�����(xx��yy) �Ƿ���ͼ���ڲ�
			// �ж�����(xx��yy) �Ƿ��Ѿ������
			// pUnRegion[yy*nWidth+xx]==0 ��ʾ��û�д���

			// �����������ж�����(xx��yy)�͵�ǰ����(nCurrX,nCurrY) ����ֵ��ľ���ֵ
			if( (xx < nWidth) && (xx>=0) && (yy>=0) && (yy<nHeight) 
				&& (pUnRegion[yy*nWidth+xx]==0) 
				&& abs( pUnchInput[yy*nSaveWidth+xx] - valueSeed )<nThreshold)  
			{
				// ��ջ��β��ָ�����һλ
				nEnd++;

				if(nEnd > maxSizeOfComponents){
					//printf("%d\n",nEnd);
					delete []pnGrowQueX;
					delete []pnGrowQueY;
					pnGrowQueX = NULL ;
					pnGrowQueY = NULL ;
					return false;

				}

				// ����(xx��yy) ѹ��ջ
				pnGrowQueX[nEnd] = xx;
				pnGrowQueY[nEnd] = yy;

				// ������(xx��yy)���ó��߼�1��255��
				// ͬʱҲ���������ش����
				pUnRegion[yy*nWidth+xx] = 255 ;
			}
		}
		nStart++;
	}


	//�ҳ�����ķ�Χ
	int nMinx=pnGrowQueX[0], nMaxx=pnGrowQueX[0], nMiny=pnGrowQueY[0], nMaxy = pnGrowQueY[0];
	for (k=0; k<nEnd; k++)
	{
		if (pnGrowQueX[k] > nMaxx)
			nMaxx = pnGrowQueX[k];
		if (pnGrowQueX[k] < nMinx) 
			nMinx = pnGrowQueX[k];
		if (pnGrowQueY[k] > nMaxy)
			nMaxy = pnGrowQueY[k];
		if (pnGrowQueY[k] < nMiny) 
			nMiny = pnGrowQueY[k];
	}

	rect.x=nMinx;
	rect.y=nMiny;
	rect.width=nMaxx-nMinx+1;
	rect.height=nMaxy-nMiny+1;

#if TEST
	for(int k=0;k<nEnd;k++)
	{
		tmpRegion2->imageData[(pnGrowQueY[k]*nSaveWidth+pnGrowQueX[k])] = 255;
	}
#endif


	// �ͷ��ڴ�
	delete []pnGrowQueX;
	delete []pnGrowQueY;
	pnGrowQueX = NULL ;
	pnGrowQueY = NULL ;
	return true;
}


