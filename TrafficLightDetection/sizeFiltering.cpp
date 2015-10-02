#include"std_tlr.h"

//
// function "sizeFiltering":
// using the area property and the width/height ratio property 
// of the extracted rectangle to filter 
//
bool sizeFiltering(CvRect rect)
{
	//const int whRatioTh = 60;//�߿����ֵ
	const int whRatioTh =50;//�߿����ֵ
	//const int areaThH = 500;//�������ֵ
	const int areaThH = 1000;//�������ֵ
	const int areaThL =5;//�������ֵ
	//const int fillingRatioTh = ;

	int rectWidth = rect.width;
	int rectHeight = rect.height;
	int rectArea = rectWidth * rectHeight;
	int ratioWH = rectWidth/rectHeight;
	int ratioHW = rectHeight/rectWidth;	
	

	/*if( (ratioWH < whRatioTh) || (ratioHW < whRatioTh) || (rectArea > areaThH) || (rectArea< areaThL))
		return FALSE;
	else
		return	TRUE;*/
	//if(((ratioHW>=whRatioTh)&&(ratioWH>=whRatioTh)&&(rectArea>=areaThL)&&(rectArea<=areaThH))||((ratioWH>=180)&&(ratioWH<=510)&&(rectArea)>=48&&(rectArea<=1500)))
	if((rectArea>=areaThL)&&(rectArea<=areaThH)&&(ratioHW<2)&&(ratioHW>0.1))
		return TRUE;
	else
		return FALSE;

}