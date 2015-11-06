#include "traffic.h"

/************************************************************************/
/* Jayn 2015.11.4
/* @param XMLName:the training XML file name(with path)                                                                     */
/************************************************************************/

int HOGTrainingTrafficSign(const String path,HOGDescriptor &hog,int num_folder,int imgWidth,int imgHeight,String XMLName)
{
	stringstream SS_folder;
	String img_num,txt_path,folder,img_path;

	vector<float> pixelVector;
	vector<float> descriptors;//��¼HOG��������
	
	Mat img,sampleFeatureMat,sampleLabelMat;
	float ClassId=0;
	int sampleNum=0;
	MySVM svm;

	//calculate the number of samples
	for (int j=0;j<num_folder;j++)
	{
		//get the folder name
		SS_folder.clear();
		SS_folder<<j;
		SS_folder>>folder;
		txt_path=path+"\\"+folder+"\\description.txt";
		ifstream txt(txt_path);
		if (!txt)
		{
			cout<<"can't open the txt file!"<<endl;
			exit(1);
		}
		//count the number of samples
		while(getline(txt,img_path))sampleNum++;
	}


	//folder ID loop
	int count_img=0;//��count_img������
	for(int j=0;j<num_folder;j++)
	{
		//get the folder name
		SS_folder.clear();
		SS_folder<<j;
		SS_folder>>folder;
		txt_path=path+"\\"+folder+"\\description.txt";
		ifstream txt(txt_path);
		if (!txt)
		{
			cout<<"can't open the txt file!"<<endl;
			exit(1);
		}

		while(getline(txt,img_path))
		{
			
			//read image
			img=imread(img_path);
			Mat resizedImg(imgHeight,imgWidth,CV_8UC3) ;
			resize(img,resizedImg,resizedImg.size());

			//calculate the HOG feature,set it into descriptors
			hog.compute(resizedImg,descriptors,Size(8,8));
			cout<<"HOG Descriptor size:"<<descriptors.size()<<endl;
			int DescriptorDim= descriptors.size();
			//if it is the first time,initialize the size of Mat
			if( 0 == count_img)
			{
				sampleFeatureMat = Mat::zeros(sampleNum, DescriptorDim, CV_32FC1);
				sampleLabelMat = Mat::zeros(sampleNum, 1, CV_32FC1);
			}

			for(int i=0; i<DescriptorDim; i++)
				sampleFeatureMat.at<float>(count_img,i) = descriptors[i];//��count_img�����������������еĵ�i��Ԫ��
			sampleLabelMat.at<float>(count_img,0) =j;//��j���ļ��е������ı�ǩ����Ϊj
			count_img++;
		}		
	}
	cout<<"sample number:"<<count_img<<endl;


	CvTermCriteria criteria = cvTermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 1000, FLT_EPSILON);
	//SVM������SVM����ΪC_SVC�����Ժ˺������ɳ�����C=0.01
	CvSVMParams param(CvSVM::C_SVC, CvSVM::LINEAR, 0, 1, 0, 0.01, 0, 0, 0, criteria);
	cout<<"��ʼѵ��SVM������"<<endl;
	svm.train_auto(sampleFeatureMat, sampleLabelMat, Mat(), Mat(), param);//ѵ��������
	cout<<"ѵ�����"<<endl;

	svm.save(XMLName.c_str());//��ѵ���õ�SVMģ�ͱ���Ϊxml�ļ�
	return sampleNum;
}