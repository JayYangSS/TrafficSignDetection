#include "std_tlr.h"
extern HOGDescriptor TLRecHOG;
extern MySVM TLRecSVM;//ʶ���ɫ�źŵ�����SVM������

#define IS_CUTIMG 0

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
	
	
	bool returnStatus = false;
	//int iSrcWidthStep = srcImage->widthStep;

	//thresholding for graylevel differences between seedpoints and its neibours
	const int grayThresholding =70;//70
	const int RatioThreshold =  55;//�����к�ɫ������ռ����

	int iDrawRectWidth = (iRect.width+iRect.height)/2 + 6;
	int iDrawRectHeight = 3*(iDrawRectWidth-4)+6;
	int iDrawRectX1=0, iDrawRectY1=0;
	int iDrawRectX2=0, iDrawRectY2=0;

	if(iColor==RED_PIXEL_LABEL){
		iDrawRectY1 = iRect.y - 3;
	}
	else if(iColor == GREEN_PIXEL_LABEL){
		iDrawRectY1 = iRect.y-iDrawRectHeight/3*2;
	}

	iDrawRectY2 = iDrawRectY1 + iDrawRectHeight;
	iDrawRectX1 = iRect.x-3;
	iDrawRectX2 = iDrawRectX1 + iDrawRectWidth;


	if( iDrawRectX1<0 || iDrawRectY1<0 || iDrawRectX2>=iWidth || iDrawRectY2>=iHeight)
	{
		cvReleaseImage(&imageGrayScale);//when return the result, the image must be released, otherwise,the memory will be leaked
		return returnStatus;
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

	iDrawRectHeight=iDrawRectY2-iDrawRectY1;
	iDrawRectWidth=iDrawRectX2-iDrawRectX1;

	int ratio = (float)sum*100/(float)((iDrawRectWidth+1)*((float)iDrawRectHeight+1));//���ο��к�ɫ������ռ����
	
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

	 if(ratio>=RatioThreshold&&ratio<=90&&BlackAroundLight(srcImage,iRect))
		returnStatus = true;

	//�������ľ��ο��������������ԭʼͼ���ϻ������α�ʾ��
	if(returnStatus==true)
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
			CvSize cutSize;
			cutSize.width=iRect.width;
			cutSize.height=iRect.height;
			IplImage *tmpCutImg=cvCreateImage(cutSize,srcImage->depth,srcImage->nChannels);
			GetImageRect(srcImage,iRect,tmpCutImg);
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
			Mat SVMTriangleMat(1,DescriptorDim,CV_32FC1);
			for(int i=0; i<DescriptorDim; i++)
				SVMTriangleMat.at<float>(0,i) = descriptor[i];

			int result=TLRecSVM.predict(SVMTriangleMat);
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
			cvReleaseImage(&tmpCutImg);//ʹ����Mat�����ͷţ�������IplImage������������Mat�ͱ������
		}
	}

	cvReleaseImage(&imageGrayScale);
	return returnStatus;
}