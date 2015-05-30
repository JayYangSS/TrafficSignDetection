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

	vector<PixelRGB> rgb;//�洢һ��ͼƬ������rgb
	void ClassifierTrain::getRGB(vector<Mat> &imgPosArray,vector<Mat> &imgNegArray);//����Ӧ����ͼ��ͱ�ǩ����
	void train(MySVM& svm);
	void svmInfo(MySVM svm);
};



