#include"std_tlr.h"
extern Size Win_vertical,block_vertical,blockStride_vertical,cell_vertical;

bool BoxDetectTL(Mat src_test,HOGDescriptor &myHOG ,bool HORZ)
{
	vector<Rect> found;
	if(HORZ)
		myHOG.detectMultiScale(src_test, found,-0.8,Size(4,4), Size(0,0), 1.05, 2);//�ĳ�-0.75�󣬶�10.23�Ųɼ��ĺ���ͨ�Ƽ��Ч����
	                                                                                //��releaseģʽ�£����ĸ�����ʹ��Size(5,5)û�����⣬debugģʽ�²��У��ĳ�Size(4,4)�ź�ʹ
	else//��ֱ���ʹ��
		myHOG.detectMultiScale(src_test, found,0.7,Size(1,1), Size(0,0),1.05, 2);//��ͼƬ���ж�߶����˼��
#if ISDEBUG_TL
    cout<<"�ҵ��ľ��ο������"<<found.size()<<endl;
#endif
  //�ҳ�����û��Ƕ�׵ľ��ο�r,������found_filtered��,�����Ƕ�׵Ļ�,��ȡ���������Ǹ����ο����found_filtered��
	for(int i=0; i < found.size(); i++)
  {
    Rect r = found[i];
    int j=0;
    for(; j < found.size(); j++)
      if(j != i && (r & found[j]) == r)
        break;
    if( j == found.size())
      return true;
  }
	return false;
 }

 
  /*
  //�����ο���Ϊhog_vertical�����ľ��ο��ʵ�������Ҫ��΢��Щ,����������Ҫ��һЩ����
  for(int i=0; i<found_filtered.size(); i++)
  {
    Rect r = found_filtered[i];

    r.x += cvRound(r.width*0.1);
    r.width = cvRound(r.width*0.8);
    r.y += cvRound(r.height*0.07);
    r.height = cvRound(r.height*0.8);
   // rectangle(src_test, r.tl(), r.br(), Scalar(0,255,0), 1);
	rectangle(src_test, r.tl(), r.br(), Scalar(0,255,0), 2);//tl()�������Ͻǵ�����,br()�������½ǵ�����
  }

	imshow("src",src_test);
	waitKey(5);//ע�⣺imshow֮������waitKey�������޷���ʾͼ��*/