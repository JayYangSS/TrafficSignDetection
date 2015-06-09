#pragma once
#include"traffic.h"


struct PixelRGB{
	int r;
	int g;
	int b;
	int p_label;
};

class ClassifierTrain
{
public:
	ClassifierTrain(void);
	~ClassifierTrain(void);

	//vector<PixelRGB> rgb;//�洢һ��ͼƬ������rgb
	MySVM svm;
	void getRGB(vector<Mat> &imgPosArray,vector<PixelRGB> &rgb,float label);//����Ӧ����ͼ��ͱ�ǩ����
	void train(vector<PixelRGB> &rgb);
	void TrainSVM(bool isTrain);
	void svmInfo();
	Mat colorThreshold(Mat img);
};



