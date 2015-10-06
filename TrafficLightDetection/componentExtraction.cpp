#include"std_tlr.h"

#define SIZE_FILTER 1
#define REGION_GROW_FILTER 1
#define RECT_FILTER 1

extern deque<float> TLFilters[2];
extern bool TLD_flag[2];//traffic lighs control flags

bool regionGrowA(int nSeedX,int nSeedY,BYTE * pUnchInput,int nWidth,int nHeight,
	             BYTE * pUnRegion,int nThreshold,int& color,CvRect &rect,int& pixelNum);


void componentExtraction(IplImage* inputImage, IplImage* srcImage,float* TLDSend,vector<Rect> &found_filtered)
{
    int r=0;int g=0;
	int iWidth = inputImage->width;
	int iHeight = inputImage->height;
	int iWidthStep = inputImage->widthStep;
	unsigned char* pImageData = (unsigned char*)inputImage->imageData;//imageData��ָ��ͼ�����������׵�ַ��ָ�룬����Ϊchar*������

	IplImage* imageGrayScale = cvCreateImage(cvSize(iWidth,iHeight),IPL_DEPTH_8U,1);
	if(!imageGrayScale)
		exit(EXIT_FAILURE);
	cvCvtColor(srcImage,imageGrayScale,CV_BGR2GRAY);

#if ISDEBUG_TL
	cvShowImage("gray",imageGrayScale);
	cvWaitKey(5);
#endif

	//thresholding for size of components
	const int thresholding = 4;
	int i=0,j=0;
	CvRect oRect;
	//CvRect ooRect;
	int rectNum = 0;
	int rectNum2 = 0;
	int pixelNum=0;
	int oColor=0;
	unsigned char* flag = new unsigned char[iWidth*iHeight];
	if(flag==NULL)
		return;
	memset(flag,0,iWidth*iHeight);//flag�����ʼ��Ϊ0
	for(i=0;i<iHeight;i++)
	{
		for(j=0;j<iWidth;j++)
		{
			if(pImageData[i*iWidthStep+j]!=0 && flag[i*iWidth+j]==0)//ͼ������ֵ��Ϊ0��û�б������
			{     
				//����ۺ���������ص���Ӧ��־λ�ã���flag�洢������Ϊ255�����ӵ������ֵ���洢��oColor�У������ڵ����ص���Ŀ����pixelNum
				if(regionGrowA(j,i,pImageData,iWidth,iHeight,flag,thresholding,oColor,oRect,pixelNum))
				{

#if SIZE_FILTER
					//��ѡ�����������Χ��������Ҫ����һ�����������߱����Ʋ��ܱ�����
					if(sizeFiltering(oRect))
					{
						rectNum++;
#if REGION_GROW_FILTER
						//rectangleDetection(imageGrayScale,srcImage,oRect,oColor);
						CvRect ooRect;
						if( regionGrowFiltering(imageGrayScale,srcImage,oRect,ooRect,found_filtered) )
						{
							rectNum2++;
#if  RECT_FILTER
							rectangleDetection(imageGrayScale,srcImage,ooRect,oColor,&r,&g);
#endif	//RECT_FILTER

						} //regionGrowFiltering_if
#endif //REGION_GROW_FILTER

					} //sizeFiltering_if
#endif //SIZE_FILTER

				} //regionGrowA_if

			}

		}
	}



	//filter the result to make it stable
	deque<float>::iterator it;
	int containCount=0;//������������Ч�������Ŀ
	if (r>=1)
	{
		TLFilters[0].push_back(8.0);
		if (TLFilters[0].size()>5)
			TLFilters[0].pop_front();
		TLD_flag[0]=true;
		it=TLFilters[0].begin();
		while (it<TLFilters[0].end())
		{
			if(*it==1.0)containCount++;
			it++;
		}
		if ((float)(containCount)/(float)TLFilters[0].size()>=0.4)
		{
			TLDSend[0]=8.0;//��ʾ��⵽���
		}else
		{
			TLDSend[0]=0;
		}
		containCount=0;
	}

	if(g>=1)
		TLDSend[1]=9.0;//p[1]=1����ʾ��⵽�̵�
	else TLDSend[1]=0;

	if(flag!=NULL){
		delete [] flag;
		flag = NULL;
	}

	cvReleaseImage(&imageGrayScale);


}



//���루nSeedX,nSeedY�����ص���ͬ�����������ҳ����������Ҫ����nThreshold*nThresholdʱ��������Χ�洢��rect��
//���ҵ��򷵻�true��û�ҵ�����false��������ۺ���������ص���Ӧ��־λ�ã���pUnRegion�洢������Ϊ255
bool regionGrowA(int nSeedX,int nSeedY,BYTE * pUnchInput,int nWidth,int nHeight,
	BYTE * pUnRegion,int nThreshold,int& color,CvRect &rect,int& pixelNum)
{


	 int nDx[] = {-1,1,0,0};
	 int nDy[] = {0,0,-1,1};
	 int nSaveWidth = (nWidth+7)/8*8;//���ϲ�ȫ����
	  
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


	 int seedpointLabel = pUnchInput[nSeedY*nSaveWidth+nSeedX];
	 color = seedpointLabel;//��ͨ��ͼ�������ӵ�����ֵ
		


	 // ѭ�����Ʊ���
	 int k ;

	 // ͼ��ĺ�������,�����Ե�ǰ���ص�8������б���
	 int xx;
	 int yy;

	 while (nStart<=nEnd)//while�ⲿ�ֵ�ѭ�������ǰ������ӵ㣨nSeedX,nSeedY������ֵ��ͬ�����ص�����ȫ������pnGrowQueX[],pnGrowQueY[]��
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
			   if ( (xx < nWidth) && (xx>=0) && (yy>=0) && (yy<nHeight) 
					&& (pUnRegion[yy*nWidth+xx]==0) && (pUnchInput[yy*nSaveWidth+xx]==seedpointLabel)) 
			   {
					// ��ջ��β��ָ�����һλ
					nEnd++;

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
		int nMinx=pnGrowQueX[0], nMaxx=pnGrowQueX[0], nMiny=pnGrowQueY[0], nMaxy = pnGrowQueY[0];//�����ӵ�����ĺ������귶Χ�ҳ�
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


	// �ͷ��ڴ�
#if(!TEST)
	 delete []pnGrowQueX;
	 delete []pnGrowQueY;
	 pnGrowQueX = NULL ;
	 pnGrowQueY = NULL ;
#endif

//Ѱ�ҵ��߽�㣬����һ��nThreshold*nThreshold�ľ��βſ������
#if (!TEST)
	if((nMaxy-nMiny)>=nThreshold && (nMaxx - nMinx)>=nThreshold){
		rect.x=nMinx;
		rect.y=nMiny;
		rect.width=nMaxx-nMinx+1;
		rect.height=nMaxy-nMiny+1;
		pixelNum = nEnd;
		return true;
	}
#endif		
	 return false;

}