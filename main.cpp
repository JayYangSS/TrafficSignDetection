
#define MAX_OUTPUT_STREAM_SIZE 256
#define RST_LENGTH (5) //ͳ�ƽ���ĳ���
#define RST_NUM (2) 
char rxbuf[MAX_OUTPUT_STREAM_SIZE];
#include"traffic.h"
#include "ClassifierTrain.h"

int main()
{
	/*vector<Rect> found_filtered;
	bool TRAIN=true;
	int bytes_recv = 0;
	


	HOGDescriptor myHOG(Size(20,20),Size(10,10),Size(5,5),Size(5,5),9,1,-1.0,0,0.2,true,30);
	Mat src,temp,re_src;
	deque<int> resultR_static,resultN_static;
	VideoCapture capture; 

	//train
	hogSVMTrain(myHOG,TRAIN);


	//start 
	capture.open("D:\\JY\\JY_TrainingSamples\\light2.avi");
	while(capture.read(src))
	{
		int start=cvGetTickCount();
		resize(src,re_src,Size(640,480));
		found_filtered.clear();
		BoxDetect(re_src,myHOG,found_filtered);
		int end=cvGetTickCount();


		float time=(float)(end-start)/(cvGetTickFrequency()*1000000);
		cout<<"ʱ�䣺"<<time<<endl;
	}	


	system("pause");
	return 0;*/

	ClassifierTrain p;
	//vector<PixelRGB> rgb;
	Mat img=imread("D:\\JY\\JY_TrainingSamples\\hardexample\\57.jpg");
	Mat img1=imread("D:\\JY\\JY_TrainingSamples\\hardexample\\30.jpg");
	vector<Mat> mm;
	mm.push_back(img);mm.push_back(img1);
	p.getRGB(mm,1);

	
	for(vector<PixelRGB>::iterator iter=p.rgb.begin();iter!=p.rgb.end();iter++)
	{
		cout<<"b:"<<(*iter).b<<"g:"<<(*iter).g<<"r:"<<(*iter).r<<"label:"<<(*iter).p_label<<endl;
	}
	cout<<"size:"<<p.rgb.size()<<endl;
	system("pause");
}