#include "std_tlr.h"
#define IS_CUTIMG 0

extern HOGDescriptor myHOG_vertical;
extern HOGDescriptor myHOG_horz;
extern HOGDescriptor TLRecHOG;
extern HOGDescriptor isTLHOG;

extern MySVM TLRecSVM;//ʶ���ɫ�źŵ�����SVM������
//extern MySVM isTLSVM;//ʶ���Ƿ����źŵ�
extern MySVM isVerticalTLSVM;//ʶ��ʶ���Ƿ�Ϊ��ֱ�źŵƵ�SVM������
extern MySVM isHorzTLSVM;//ʶ��ʶ���Ƿ�Ϊˮƽ�źŵƵ�SVM������
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
int RecognizeLight(IplImage* srcImg,CvRect iRect)
{
	CvSize cutSize;
	cutSize.width=iRect.width;
	cutSize.height=iRect.height;
	IplImage *tmpCutImg=cvCreateImage(cutSize,srcImg->depth,srcImg->nChannels);
	GetImageRect(srcImg,iRect,tmpCutImg);
#if IS_CUTIMG
	cvShowImage("tmpCutImg",tmpCutImg);
	cvWaitKey(1);
	char tmpName[100];
	static int ppp=0;
	ppp++;
	sprintf_s(tmpName,"ImgCut//%d.jpg",ppp);
	cvSaveImage(tmpName,tmpCutImg);
#endif

	Mat cutMat(tmpCutImg);
	Mat tmpTLRec;
	vector<float> descriptor;

	//ʶ���źŵ����
	resize(cutMat,tmpTLRec,Size(TLREC_WIDTH,TLREC_HEIGHT));
	TLRecHOG.compute(tmpTLRec,descriptor,Size(8,8));
	int DescriptorDim=descriptor.size();		
	Mat SVMTLRecMat(1,DescriptorDim,CV_32FC1);
	for(int i=0; i<DescriptorDim; i++)
		SVMTLRecMat.at<float>(0,i) = descriptor[i];

	int result=TLRecSVM.predict(SVMTLRecMat);
	cvReleaseImage(&tmpCutImg);
	return result;
}



int isTL(IplImage* srcImg,CvRect iRect,bool isVertical)
{
	CvSize cutSize;
	cutSize.width=iRect.width;
	cutSize.height=iRect.height;
	IplImage *tmpCutImg=cvCreateImage(cutSize,srcImg->depth,srcImg->nChannels);
	GetImageRect(srcImg,iRect,tmpCutImg);

	Mat cutMat(tmpCutImg);
	Mat tmpIsTL;
	vector<float> descriptor;

	//ʶ���źŵ����
	if (isVertical){
		resize(cutMat, tmpIsTL, Size(HOG_TLVertical_Width, HOG_TLVertical_Height));
		myHOG_vertical.compute(tmpIsTL, descriptor, Size(8, 8));
	}
	else{
		resize(cutMat, tmpIsTL, Size(HOG_TLHorz_Width, HOG_TLHorz_Height));
		myHOG_horz.compute(tmpIsTL, descriptor, Size(8, 8));
	}
		
	int DescriptorDim=descriptor.size();		
	Mat SVMTLRecMat(1,DescriptorDim,CV_32FC1);
	for(int i=0; i<DescriptorDim; i++)
		SVMTLRecMat.at<float>(0,i) = descriptor[i];

	//int result=isTLSVM.predict(SVMTLRecMat);
	int result = 0;
	if (isVertical)
		result = isVerticalTLSVM.predict(SVMTLRecMat);
	else
	{
		result = isHorzTLSVM.predict(SVMTLRecMat);
	}
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


