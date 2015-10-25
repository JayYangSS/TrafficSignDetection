#ifndef TRAFFIC_H
#define TRAFFIC_H

#include <iostream>
#include <fstream>
#include "opencv2/opencv.hpp"
#include <vector>
#include <stdlib.h>

#define OPENMP 1//�Ƿ�ʹ��OPENMP���д���
#define ISDEBUG_TL 0//����ʱ��������м����ͼ����Ϣ(Traffic Light)
#define ISDEBUG_TS 0//����ʱ��������м����ͼ����Ϣ(Traffic Sign)
#define SAVEVIDEO 1//�Ƿ񱣴���Ƶ
#define GREEN_PIXEL_LABEL 255
#define RED_PIXEL_LABEL 128
#define NON_BLOB_PIXEL_LABEL 0
#define ROIHeight 480
#define ROIWidth 0
#define PI 3.1415
#define RESULT_G 0
#define RESULT_R 1
#define RESULT_NON 2
#define BIASX 50  //���ο�ƫ��


#define PosSamNO    69 //����������
#define HORZ_PosSamNO    42 //����������
#define NegSamNO 2000   //����������
#define HORZ_NegSamNO 3042
#define HardExampleNO 18
#define HORZ_HardExampleNO 21

//��״��Ϣ����
#define TRIANGLE 0
#define CIRCLE 1
#define HEXA 2
#define RECTANGLE 3


//HardExample�����������������HardExampleNO����0����ʾ�������ʼ���������󣬼�������HardExample����������
//��ʹ��HardExampleʱ��������Ϊ0����Ϊ������������������������ά����ʼ��ʱ�õ����ֵ


using namespace std;
using namespace cv;


class DetecResult{
public:
	int LightResult[8];
	//int LightPos[8];
	Rect LightPos[8];
	DetecResult()
	{
		for(int i=0;i<8;i++)
		{
			LightResult[i]=RESULT_NON;
		}

	}

};
class ShapeRecResult{
public:
	Rect box;
	int color;
	int shape;
};


class MySVM : public CvSVM
{
public:
	//���SVM�ľ��ߺ����е�alpha����
	double * get_alpha_vector()
	{
		return this->decision_func->alpha;
	}

	//���SVM�ľ��ߺ����е�rho����,��ƫ����
	float get_rho()
	{
		return this->decision_func->rho;
	}
};

IplImage* colorSegmentation(IplImage* inputImage);
void showHist(Mat src);
void hogSVMTrain( HOGDescriptor &myHOG,bool TRAIN);
void BoxDetect(Mat src_test,HOGDescriptor &myHOG,vector<Rect> &found_filtered);
void rgb2hsi(int red, int green, int blue, int& hue, int& saturation, int& intensity );
void setLabel(cv::Mat& im, const std::string label, Rect r);//show the label information
int detect_result(Mat src_test,vector<Rect> &found_filtered,DetecResult *detct,char Direct);
int SortRect(Mat src_test,int num,DetecResult *Rst,char Direct);
Mat ShapeRecognize(Mat src,vector<ShapeRecResult>&shapeResult);

#endif