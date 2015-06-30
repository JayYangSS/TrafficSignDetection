#include"traffic.h"
#include <sys/stat.h>


#define NUM_SPEED 5
#define IMG_NEW_DIM 20

void LoadImg2HOG_Mat(string &rowPath,int file_ID,float Label,Mat &sampleFeatureMat,Mat &sampleLabelMat);
void LoadImg2HOG_Neg(string &rowPath,float Label,Mat &sampleFeatureMat,Mat &sampleLabelMat);
static bool fileExists(const char * fileName);
string convertInt(int number, char * prefix);

void hogSVMTrain(MySVM &svm,bool TRAIN)
{
	
    int DescriptorDim;
   // MySVM svm;
    int WinStride;
    String posPath="D:\\JY\\JY_TrainingSamples\\GTSRB_Final_Training_Images\\GTSRB\\Final_Training\\Images";
	String negPath="D:\\JY\\JY_TrainingSamples\\negetive1\\negetive1.txt";

  if(TRAIN)
  {
    string ImgName;
    Mat SpeedFeature1,SpeedFeature2,SpeedFeature3,SpeedFeature4,SpeedFeature5,negFeatureMat;
    Mat SpeedLabel1,SpeedLabel2,SpeedLabel3,SpeedLabel4,SpeedLabel5,negLabelMat;
	

    //load positive samples and compute hog descriptor
    LoadImg2HOG_Mat(posPath,1,1.0,SpeedFeature1,SpeedLabel1);
	LoadImg2HOG_Mat(posPath,2,2.0,SpeedFeature2,SpeedLabel2);
	LoadImg2HOG_Mat(posPath,3,3.0,SpeedFeature3,SpeedLabel3);
	LoadImg2HOG_Mat(posPath,4,4.0,SpeedFeature4,SpeedLabel4);
	LoadImg2HOG_Mat(posPath,5,5.0,SpeedFeature5,SpeedLabel5);


	//load negative samples and compute hog descriptor
    LoadImg2HOG_Neg(negPath,-1.0,negFeatureMat,negLabelMat);

   SpeedFeature1.push_back(SpeedFeature2);
   SpeedFeature1.push_back(SpeedFeature3);
   SpeedFeature1.push_back(SpeedFeature4);
   SpeedFeature1.push_back(SpeedFeature5);
   SpeedFeature1.push_back(negFeatureMat);

   SpeedLabel1.push_back(SpeedLabel2);
   SpeedLabel1.push_back(SpeedLabel3);
   SpeedLabel1.push_back(SpeedLabel4);
   SpeedLabel1.push_back(SpeedLabel5);
   SpeedLabel1.push_back(negLabelMat);

    //ѵ��SVM������,������ֹ��������������1000�λ����С��FLT_EPSILONʱֹͣ����
    CvTermCriteria criteria = cvTermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 1000, FLT_EPSILON);
    //SVM������SVM����ΪC_SVC�����Ժ˺������ɳ�����C=0.01
    CvSVMParams param(CvSVM::C_SVC, CvSVM::POLY,2, 1, 0, 1, 0, 0, 0, criteria);//ʹ�ö���ʽ��׼ȷ�ʱ���������Ҫ��
    cout<<"��ʼѵ��SVM������"<<endl;
    svm.train(SpeedFeature1, SpeedLabel1, Mat(), Mat(), param);//ѵ��������
    cout<<"ѵ�����"<<endl;
    svm.save("src//SVM_SpeedSign.xml");//��ѵ���õ�SVMģ�ͱ���Ϊxml�ļ�
  }
  else //��TRAINΪfalse����XML�ļ���ȡѵ���õķ�����
  {
    svm.load("src//SVM_SpeedSign.xml");//��XML�ļ���ȡѵ���õ�SVMģ��
  }

}




string convertInt(int number, char * prefix)
{
	stringstream ss;//create a stringstream
	ss << prefix << number;//add number to the stream
	return ss.str();//return a string with the contents of the stream
}

static bool fileExists(const char * fileName)
{
	struct stat stFileInfo;
	const int intStat = stat(fileName, &stFileInfo);
	return (intStat == 0);
}



//���ζ�ȡ������ͼƬ����������HOG�����ͱ�ǩ�ֱ����sampleFeatureMat��sampleLabelMat��
void LoadImg2HOG_Mat(string &rowPath,int file_ID,float Label,Mat &sampleFeatureMat,Mat &sampleLabelMat)
{
	int DescriptorDim;
	HOGDescriptor hog(Size(IMG_NEW_DIM,IMG_NEW_DIM),Size(10,10),Size(5,5),Size(5,5),9,1,-1.0,0,0.2,true,30);
	String trainingSample[NUM_SPEED] = {"01","02","03","04","05"}; 
	int num=0;
	Mat croppedImg,img;
	Mat resizedImg(IMG_NEW_DIM,IMG_NEW_DIM,CV_8UC3);
	Mat tempFeatureMat;
	Mat tempLabelMat;
	//read every directory 

	string numFolder= "000"+trainingSample[file_ID];
	string folder=rowPath+"\\"+numFolder;
	string csvFile=folder+"\\"+"GT-"+numFolder+".csv";



		ifstream file(csvFile.c_str());
		string line;
		int numeroLigne = 0;
		//read every single image
		while(getline(file,line)&&numeroLigne<=1000)
		{
			cout<<"test"<<endl;
			numeroLigne++;
			if(numeroLigne==1)continue;
			replace(line.begin(),line.end(),';',' ');

			istringstream iss(line);
			string rawInfo[8];
			string cell;
			int k=0;

			string imagePath;
			int RoiX1 ,  RoiY1, RoiX2, RoiY2, ClassId; 

			while (iss >> cell)
			{
				rawInfo[k] = cell;
				k++;				
			}

			imagePath =  folder + "\\"  +  rawInfo[0] ;
			RoiX1 = atoi(rawInfo[3].c_str());
			RoiY1 = atoi(rawInfo[4].c_str());
			RoiX2 = atoi(rawInfo[5].c_str());
			RoiY2 = atoi(rawInfo[6].c_str());
			ClassId = atoi(rawInfo[7].c_str());

			img = imread( imagePath.c_str() , CV_LOAD_IMAGE_COLOR );
			Rect ROI(RoiX1, RoiY1, RoiX2 - RoiX1, RoiY2 - RoiY1);
			croppedImg= img(ROI).clone();
			resize(croppedImg ,resizedImg ,resizedImg.size());
		

			vector<float> descriptors;//HOG descriptor
			hog.compute(resizedImg,descriptors,Size(8,8));//block stride(8,8)
			cout<<"������ά����"<<descriptors.size()<<endl;

			DescriptorDim = descriptors.size();//HOG�����ӵ�ά��
			//initialize the first mat
			if( 0 == num )
			{
				sampleFeatureMat.create(1,DescriptorDim,CV_32FC1);  
				sampleLabelMat.create(1,1,CV_32FC1); 
				for(int i=0;i<DescriptorDim;i++)
					sampleFeatureMat.at<float>(num,i) = descriptors[i];//��num�����������������еĵ�i��Ԫ��
				 sampleLabelMat.at<float>(0,0) = Label;//���������Ϊ-1������
			}else{
				tempFeatureMat=Mat::zeros(1,DescriptorDim,CV_32FC1);
				tempLabelMat=Mat::zeros(1,1,CV_32FC1);

				
				for(int i=0;i<DescriptorDim;i++)
					tempFeatureMat.at<float>(0,i) = descriptors[i];//��num�����������������еĵ�i��Ԫ��
				tempLabelMat.at<float>(0,0) = Label;//���������Ϊ-1������

				//concatenate the feature and label mat of every single image into one Mat
				
				sampleFeatureMat.push_back(tempFeatureMat);
				sampleLabelMat.push_back(tempLabelMat);
			}
			num++;
			cout<<num<<endl;
		}
		file.close();
}



//���ζ�ȡ������ͼƬ����������HOG�����ͱ�ǩ�ֱ����sampleFeatureMat��sampleLabelMat��
void LoadImg2HOG_Neg(string &rowPath,float Label,Mat &sampleFeatureMat,Mat &sampleLabelMat)
{
	int DescriptorDim; 
	ifstream finNeg(rowPath);//������ͼƬ���ļ����б�
	string ImgName;//ͼƬ��(����·��)
	HOGDescriptor hog(Size(IMG_NEW_DIM,IMG_NEW_DIM),Size(10,10),Size(5,5),Size(5,5),9,1,-1.0,0,0.2,true,30);
	

    for(int num=0; num<NegSamNO && getline(finNeg,ImgName); num++)
    {
      cout<<"����"<<ImgName<<endl;
      Mat src = imread(ImgName);//��ȡͼƬ
	  resize(src,src,Size(IMG_NEW_DIM,IMG_NEW_DIM));
      vector<float> descriptors;//HOG����������
	  hog.compute(src,descriptors,Size(8,8));//����HOG�����ӣ���ⴰ���ƶ�����(8,8)
      cout<<"������ά����"<<descriptors.size()<<endl;
	  DescriptorDim = descriptors.size();//HOG�����ӵ�ά��


	  if(num==0)
	  {
		  sampleFeatureMat = Mat::zeros(NegSamNO, DescriptorDim, CV_32FC1);
		  sampleLabelMat = Mat::zeros(NegSamNO, 1, CV_32FC1);
	  }
	  

	  for(int i=0; i<DescriptorDim; i++)
			sampleFeatureMat.at<float>(num,i) = descriptors[i];//��PosSamNO+num�����������������еĵ�i��Ԫ��
		sampleLabelMat.at<float>(num,0) = -1;//���������Ϊ-1������
	}
	finNeg.close();

}