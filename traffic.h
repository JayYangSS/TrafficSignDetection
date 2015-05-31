#ifndef TRAFFIC_H
#define TRAFFIC_H

#include <iostream>
#include <fstream>
#include "opencv2/opencv.hpp"
#include <vector>
#include <stdlib.h>
/*
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/ml/ml.hpp>*/
//#include <socket_manager.h>

#define GREEN_PIXEL_LABEL 255
#define RED_PIXEL_LABEL 128
#define NON_BLOB_PIXEL_LABEL 0
//#define ROIHeight 300
#define ROIHeight 250
#define	ROIWidth 0
#define PI 3.1415
#define RESULT_G 0
#define RESULT_R 1
#define RESULT_NON 2
#define BIASX 50  //���ο�ƫ��


#define PosSamNO    69 //����������
//#define PosSamNO    28 //����������
#define HORZ_PosSamNO    42 //����������
//#define PosSamNO 10    //����������
//#define NegSamNO 2  //����������
#define NegSamNO 2979   //����������
#define HORZ_NegSamNO 3042
#define HardExampleNO 18
#define HORZ_HardExampleNO 21

//HardExample�����������������HardExampleNO����0����ʾ�������ʼ���������󣬼�������HardExample����������
//��ʹ��HardExampleʱ��������Ϊ0����Ϊ������������������������ά����ʼ��ʱ�õ����ֵ


using namespace std;
using namespace cv;

/*struct DetecResult{
	int LightResult[8];
	//int LightPos[8];
	Rect LightPos[8];
};*/


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

int colorSegmentation(IplImage* inputImage);
void rgb2hsi(int red, int green, int blue, int& hue, int& saturation, int& intensity );
//void hog_svmDetect(Mat src_test,bool TRAIN,vector<Rect> &found_filtered);
int detect_result(Mat src_test,vector<Rect> &found_filtered,DetecResult *detct,char Direct);
void hogSVMTrain( HOGDescriptor &myHOG,bool TRAIN);
//void BoxDetect(Mat src,Mat src_test,HOGDescriptor &myHOG,vector<Rect> &found_filtered);
void BoxDetect(Mat src_test,HOGDescriptor &myHOG,vector<Rect> &found_filtered);
int SortRect(Mat src_test,int num,DetecResult *Rst,char Direct);



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


#endif