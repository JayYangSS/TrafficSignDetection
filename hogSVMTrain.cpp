#include"traffic.h"
void hogSVMTrain( HOGDescriptor &myHOG,bool TRAIN)
{
	HOGDescriptor hog(Size(20,20),Size(10,10),Size(5,5),Size(5,5),9,1,-1.0,0,0.2,true,30);
    int DescriptorDim;
    MySVM svm;
    int WinStride;
 

  if(TRAIN)
  {
    string ImgName;
    ifstream finPos("D:\\VS2010_Projects\\JY_TrafficLight\\HOG_Traffic\\positive\\positivePath.txt");//������ͼƬ���ļ����б�
	ifstream finNeg("D:\\VS2010_Projects\\JY_TrafficLight\\HOG_Traffic\\N_Neg\\N_Neg.txt");//������ͼƬ���ļ����б�
    Mat sampleFeatureMat;
    Mat sampleLabelMat;
	


    //load positive samples and compute hog descriptor
    for(int num=0; num<PosSamNO && getline(finPos,ImgName); num++)
    {
		cout<<"����"<<ImgName<<endl;
		Mat src = imread(ImgName);
 		resize(src,src,Size(15,30));
		vector<float> descriptors;//HOG descriptor
		hog.compute(src,descriptors,Size(8,8));//block stride(8,8)
		cout<<"������ά����"<<descriptors.size()<<endl;

        //initialize the first mat
        if( 0 == num )
        {
          DescriptorDim = descriptors.size();//HOG�����ӵ�ά��
		  sampleFeatureMat = Mat::zeros(PosSamNO+NegSamNO+HardExampleNO, DescriptorDim, CV_32FC1);
          sampleLabelMat = Mat::zeros(PosSamNO+NegSamNO+HardExampleNO, 1, CV_32FC1);
        }


        for(int i=0; i<DescriptorDim; i++)
		  sampleFeatureMat.at<float>(num,i) = descriptors[i];//��num�����������������еĵ�i��Ԫ��
        sampleLabelMat.at<float>(num,0) = 1;//���������Ϊ1������
     }
	




	//load negative samples and compute hog descriptor
     for(int num=0; num<NegSamNO && getline(finNeg,ImgName); num++)
     {
       cout<<"����"<<ImgName<<endl;
       Mat src = imread(ImgName);//��ȡͼ
	   resize(src,src,Size(15,30));
       vector<float> descriptors;//HOG����������
	   hog.compute(src,descriptors,Size(8,8));//����HOG�����ӣ���ⴰ���ƶ�����(8,8)
       cout<<"������ά����"<<descriptors.size()<<endl;


      //������õ�HOG�����Ӹ��Ƶ�������������sampleFeatureMat
       for(int i=0; i<DescriptorDim; i++)
         sampleFeatureMat.at<float>(num+PosSamNO,i) = descriptors[i];//��PosSamNO+num�����������������еĵ�i��Ԫ��
       sampleLabelMat.at<float>(num+PosSamNO,0) = -1;//���������Ϊ-1������
     }

    //����HardExample������
    if(HardExampleNO > 0)
    {
      ifstream finHardExample("D:\\VS2010_Projects\\JY_TrafficLight\\HOG_Traffic\\hardexample\\hardexample.txt");//HardExample���������ļ����б�
      //���ζ�ȡHardExample������ͼƬ������HOG������
      for(int num=0; num<HardExampleNO && getline(finHardExample,ImgName); num++)
      {
        cout<<"����"<<ImgName<<endl;
       // ImgName = ImgName;//����HardExample��������·����
        Mat src = imread(ImgName);//��ȡͼƬ
		resize(src,src,Size(15,30));
        vector<float> descriptors;//HOG����������
        hog.compute(src,descriptors,Size(8,8));//����HOG�����ӣ���ⴰ���ƶ�����(8,8)

        //������õ�HOG�����Ӹ��Ƶ�������������sampleFeatureMat
        for(int i=0; i<DescriptorDim; i++)
          sampleFeatureMat.at<float>(num+PosSamNO+NegSamNO,i) = descriptors[i];//��PosSamNO+num�����������������еĵ�i��Ԫ��
        sampleLabelMat.at<float>(num+PosSamNO+NegSamNO,0) = -1;//���������Ϊ-1������
      }
    }


    //ѵ��SVM������,������ֹ��������������1000�λ����С��FLT_EPSILONʱֹͣ����
    CvTermCriteria criteria = cvTermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 1000, FLT_EPSILON);
    //SVM������SVM����ΪC_SVC�����Ժ˺������ɳ�����C=0.01
    CvSVMParams param(CvSVM::C_SVC, CvSVM::LINEAR, 0, 1, 0, 0.01, 0, 0, 0, criteria);
    cout<<"��ʼѵ��SVM������"<<endl;
    svm.train(sampleFeatureMat, sampleLabelMat, Mat(), Mat(), param);//ѵ��������
    cout<<"ѵ�����"<<endl;
    svm.save("SVM_HOG.xml");//��ѵ���õ�SVMģ�ͱ���Ϊxml�ļ�

  }
  else //��TRAINΪfalse����XML�ļ���ȡѵ���õķ�����
  {
    svm.load("SVM_HOG_BenchMark.xml");//��XML�ļ���ȡѵ���õ�SVMģ��
  }


  //train SVM
  DescriptorDim = svm.get_var_count();
  int supportVectorNum = svm.get_support_vector_count();
  cout<<"֧������������"<<supportVectorNum<<endl;

  Mat alphaMat = Mat::zeros(1, supportVectorNum, CV_32FC1);//alpha���������ȵ���֧����������
  Mat supportVectorMat = Mat::zeros(supportVectorNum, DescriptorDim, CV_32FC1);//֧����������
  Mat resultMat = Mat::zeros(1, DescriptorDim, CV_32FC1);//alpha��������֧����������Ľ��

  //��֧�����������ݸ��Ƶ�supportVectorMat������
  for(int i=0; i<supportVectorNum; i++)
  {
    const float * pSVData = svm.get_support_vector(i);//���ص�i��֧������������ָ��
    for(int j=0; j<DescriptorDim; j++)
    {
      //cout<<pData[j]<<" ";
      supportVectorMat.at<float>(i,j) = pSVData[j];
    }
  }

  //��alpha���������ݸ��Ƶ�alphaMat��
  double * pAlphaData = svm.get_alpha_vector();//����SVM�ľ��ߺ����е�alpha����
  for(int i=0; i<supportVectorNum; i++)
  {
    alphaMat.at<float>(0,i) = pAlphaData[i];
  }

  resultMat = -1 * alphaMat * supportVectorMat;
  vector<float> myDetector;
  for(int i=0; i<DescriptorDim; i++)
  {
    myDetector.push_back(resultMat.at<float>(0,i));
  }




  //������ƫ����rho���õ������
  myDetector.push_back(svm.get_rho());
  cout<<"�����ά����"<<myDetector.size()<<endl;
 

  myHOG.setSVMDetector(myDetector);//����hog.cpp�е�setSVMDetector�����е�svmDetector��
  ofstream fout("HOGDetectorForOpenCV.txt");
  for(int i=0; i<myDetector.size(); i++)
  {
    fout<<myDetector[i]<<endl;
  }


}