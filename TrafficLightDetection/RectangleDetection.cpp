#include "std_tlr.h"

void GetImageRect(IplImage* orgImage, CvRect rectInImage, IplImage* imgRect)
{
	//��ͼ��orgImage����ȡһ�飨rectInImage����ͼ��imgRect
	IplImage *result=imgRect;
	CvSize size;
	size.width=rectInImage.width;
	size.height=rectInImage.height;
	//result=cvCreateImage( size, orgImage->depth, orgImage->nChannels );
	//��ͼ������ȡ��ͼ��
	cvSetImageROI(orgImage,rectInImage);
	cvCopy(orgImage,result);
	cvResetImageROI(orgImage);
}


bool isLighInBox(Mat src)
{
	//TODO:����Ĵ����߼���û�����
	Mat topHat;
	Mat kernal=getStructuringElement(MORPH_ELLIPSE,Size(5,5));
	morphologyEx(src,topHat,MORPH_TOPHAT,kernal,Point(-1,-1),4);
	double max_topHat,min_topHat;
	Point maxPoint,minPoint;
	minMaxLoc(topHat,&min_topHat,&max_topHat,&minPoint,&maxPoint);
	imshow("src",src);
	waitKey(1);
	imshow("topHat",topHat);
	waitKey(1);
	double max_tempGray,min_tempGray;
	Point maxPoint_tempGray,minPoint_tempGray;
	minMaxLoc(src,&min_tempGray,&max_tempGray,&minPoint_tempGray,&maxPoint_tempGray);

#if ISDEBUG_TL
	cout<<"min_topHat:"<<min_topHat<<endl;
	cout<<"max_topHat:"<<max_topHat<<endl;
	cout<<"min_gray:"<<min_tempGray<<endl;
	cout<<"max_gray:"<<max_tempGray<<endl;

	ofstream outfile;
	outfile.open(debugTLPath,ios::app);
	outfile<<"min_topHat:"<<min_topHat<<endl;
	outfile<<"max_topHat:"<<max_topHat<<endl;
	outfile<<"min_gray:"<<min_tempGray<<endl;
	outfile<<"max_gray:"<<max_tempGray<<endl;
	outfile<<""<<endl;
	outfile.close();
#endif
	return false;
}

bool rectangleDetection(IplImage* inputImage,IplImage* srcImage,CvRect iRect,int iColor,int* p1,int* p2)//p1Ϊǰ��λ��p2Ϊ��תλ
{
	
	const int iWidth = inputImage->width;
	const int iHeight = inputImage->height;
	IplImage* imageGrayScale = cvCreateImage(cvSize(iWidth,iHeight),IPL_DEPTH_8U,1);
	int iWidthStep = imageGrayScale->widthStep; 
	cvCvtColor(srcImage,imageGrayScale,CV_BGR2GRAY);
	bool VerticalReturnStatus = false;
	bool HorzReturnStatus=false;

	//�������
	int HorzRectHeight=(iRect.width+iRect.height)/2 + 6;
	int HorzRectWidth=3*(HorzRectHeight-4)+6;
	int HorzRectX1=0, HorzRectY1=0;
	int HorzRectX2=0, HorzRectY2=0;


	//int iSrcWidthStep = srcImage->widthStep;

	//thresholding for graylevel differences between seedpoints and its neibours
	const int grayThresholding =70;//70
	const int RatioThreshold =  55;//�����к�ɫ������ռ����

	//�������
	int iDrawRectWidth = (iRect.width+iRect.height)/2 + 6;
	int iDrawRectHeight = 3*(iDrawRectWidth-4)+6;
	int iDrawRectX1=0, iDrawRectY1=0;
	int iDrawRectX2=0, iDrawRectY2=0;

	if(iColor==RED_PIXEL_LABEL){
		iDrawRectY1 = iRect.y - 3;
		HorzRectX1= iRect.x-3;
	}
	else if(iColor == GREEN_PIXEL_LABEL){
		iDrawRectY1 = iRect.y-iDrawRectHeight/3*2;
		HorzRectX1=iRect.x-HorzRectWidth/3*2;
	}

	iDrawRectY2 = iDrawRectY1 + iDrawRectHeight;
	iDrawRectX1 = iRect.x-3;
	iDrawRectX2 = iDrawRectX1 + iDrawRectWidth;

	HorzRectX2= HorzRectX1+HorzRectWidth;
	HorzRectY1= iRect.y-3;
	HorzRectY2= HorzRectY1+HorzRectHeight;

	if(HorzRectX1<0 || HorzRectY1<0 || HorzRectX2>=iWidth || HorzRectY2>=iHeight)
	{
		cvReleaseImage(&imageGrayScale);//when return the result, the image must be released, otherwise,the memory will be leaked
		return HorzReturnStatus;
	}
	
	if( iDrawRectX1<0 || iDrawRectY1<0 || iDrawRectX2>=iWidth || iDrawRectY2>=iHeight)
	{
		cvReleaseImage(&imageGrayScale);//when return the result, the image must be released, otherwise,the memory will be leaked
		return VerticalReturnStatus;
	}

	int sum=0;
	int grayValue=0;
	//int bValue=0,gValue=0,rValue=0;
	//int bgrMax=0,bgrMin=0;
	unsigned char* pData;
	//unsigned char* pSrcData;
	for(int j=iDrawRectY1; j<=iDrawRectY2; j++){
		pData = (unsigned char*)imageGrayScale->imageData + j*iWidthStep;
		for(int i=iDrawRectX1; i<=iDrawRectX2; i++){
			grayValue = pData[i];
			if((grayValue<=grayThresholding))
				sum++;
		}
	}	

	//ˮƽ����ͳ�ƺ�ɫ���ر���
	int HorzSum=0;
	for(int j=HorzRectY1; j<=HorzRectY2; j++){
		pData = (unsigned char*)imageGrayScale->imageData + j*iWidthStep;
		for(int i=HorzRectX1; i<=HorzRectX2; i++){
			grayValue = pData[i];
			if((grayValue<=grayThresholding))
				HorzSum++;
		}
	}	

	//��ֱ��ⴰ
	iDrawRectHeight=iDrawRectY2-iDrawRectY1;
	iDrawRectWidth=iDrawRectX2-iDrawRectX1;

	//ˮƽ��ⴰ
	HorzRectHeight=HorzRectY2-HorzRectY1;
	HorzRectWidth=HorzRectX2-HorzRectX1;

	int VerticalBlackRatio = (float)sum*100/(float)((iDrawRectWidth+1)*((float)iDrawRectHeight+1));//���ο��к�ɫ������ռ����
	int HorzBlackRatio=(float)HorzSum*100/(float)((HorzRectWidth+1)*((float)HorzRectHeight+1));//���ο��к�ɫ������ռ����
	
#if ISDEBUG_TL
	ofstream outfile;
	outfile.open(debugTLPath,ios::app);//ios::app�� ��׷�ӵķ�ʽ���ļ�
	outfile<<"===black ratio===:"<<ratio<<endl;//����������ļ���
	cout<<"===black ratio===:"<<ratio<<endl;//���������̨
	outfile.close();
#endif

#if ISDEBUG_TL
	Mat grayMat(imageGrayScale);
	Rect drawRect;
	drawRect.x=iDrawRectX1;
	drawRect.y=iDrawRectY1;
	drawRect.width=iDrawRectX2-iDrawRectX1;
	drawRect.height=iDrawRectY2-iDrawRectY1;
	Mat tmpMat=grayMat(drawRect);
	isLighInBox(tmpMat);
#endif

	int DetectResult=isTL(srcImage,iRect);cout<<"�����źŵƣ�"<<DetectResult<<endl;
	if (DetectResult==1)
	{
		if(VerticalBlackRatio>=RatioThreshold&&VerticalBlackRatio<=90)
			VerticalReturnStatus = true;
		else if (HorzBlackRatio>=RatioThreshold&&HorzBlackRatio<=90)
		{
			HorzReturnStatus=true;
		}
	}
	 

	//�������ľ��ο��������������ԭʼͼ���ϻ������α�ʾ��
	if(VerticalReturnStatus==true)
	{
		if(iColor==GREEN_PIXEL_LABEL)
		{
			cvRectangle(srcImage,cvPoint(iDrawRectX1,iDrawRectY1),cvPoint(iDrawRectX2,iDrawRectY2),cvScalar(0,255,0),2);
			//*p2=*p2+1;
		}

		else if(iColor==RED_PIXEL_LABEL)
		{
			cvRectangle(srcImage,cvPoint(iDrawRectX1,iDrawRectY1),cvPoint(iDrawRectX2,iDrawRectY2),cvScalar(0,0,255),2);
			//*p1=*p1+1;


			//TODO:ʶ���źŵ�ָ��
			int result=RecognizeLight(srcImage,iRect);
			switch(result)
			{
			case 0://Բ��
				//cout<<"Բ��"<<endl;
				*p1=1;
				break;
			case 1://��ֹ��ת
				//cout<<"��ֹ��ת"<<endl;
				*p2=1;
				break;
			case 2://��ת
				//cout<<"��ֹ��ת"<<endl;
				//*p3=1;
				break;
			default:
				break;
			}
		}
	}else if (HorzReturnStatus)
	{
		//������
		if(iColor==GREEN_PIXEL_LABEL)
		{
			cvRectangle(srcImage,cvPoint(HorzRectX1,HorzRectY1),cvPoint(HorzRectX2,HorzRectY2),cvScalar(0,255,0),2);
			//*p2=*p2+1;
		}

		else if(iColor==RED_PIXEL_LABEL)
		{
			cvRectangle(srcImage,cvPoint(HorzRectX1,HorzRectY1),cvPoint(HorzRectX2,HorzRectY2),cvScalar(0,0,255),2);
			//*p1=*p1+1;


			//TODO:ʶ���źŵ�ָ��
			int result=RecognizeLight(srcImage,iRect);
			switch(result)
			{
			case 0://Բ��
				//cout<<"Բ��"<<endl;
				*p1=1;
				break;
			case 1://��ֹ��ת
				//cout<<"��ֹ��ת"<<endl;
				*p2=1;
				break;
			case 2://��ת
				//cout<<"��ֹ��ת"<<endl;
				//*p3=1;
				break;
			default:
				break;
			}
		}
	}

	cvReleaseImage(&imageGrayScale);
	return VerticalReturnStatus;
}