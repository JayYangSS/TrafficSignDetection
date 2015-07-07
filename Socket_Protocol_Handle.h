/**************************************************************************
�ļ���	��Socket_Protocol_Handle.h
����	��dly
�汾	��2.0
�޸�ʱ�䣺2014-08-24
����	�����ݡ��������ݴ���Э��-2.0��ʵ�ְ�Э���װ���ݰ�,�������ݰ�����
�ܵĴ����ʽ����ͷ���������� + ��������(1) + ʱ���(4) + ���ĳ���(4) + �������� + ��β���������루ͬ��ͷ��
***********************************************************************/
#ifndef __SOCKET_PROTOCOL_HANDLE_H
#define __SOCKET_PROTOCOL_HANDLE_H
#include "opencv2\opencv.hpp"
/*����*/
#define INDEX_NULL		0	//������
#define INDEX_STRING	1	//�ַ���
#define INDEX_ARRAY		2	//����
#define INDEX_IMAGE		3	//ͼ��
#define INDEX_USER		4	//�û��Զ���
/*������*/
#define SUB_INDEX_1		1	//
/*��������*/
#define ERR_FAULT		-1	//����Э�����ݳ���
#define ERR_OK			0	//�������
#define ERR_STICKING	1	//package sticking
#define ERR_BROKEN		2	//package broken


/*����Э���������ṹ��*/
typedef 
struct _DECODING_RESULT
{
	unsigned char index;	//��������
	unsigned char sub_index;//����������
	unsigned int timestamp;	//ʱ���
	union	//����ָ��
	{
		struct
		{
			char *p_string;		//�ַ���
			int length;
		};
		CvMat *p_mat;		//����mat
		IplImage *p_img;	//ͼ��
	};
	int total_byte_num;	//��Ч�����ܵ��ֽ���
}
DECODING_RESULT,SEND_OBJ;

extern DECODING_RESULT g_socket_decoding_result;
extern char g_err_info[128];
extern int SocketDecoder(DECODING_RESULT *p_decoding_result, unsigned char * p_rxbuf, int rx_num, char *err_info);
extern int SocketPackager(unsigned char *p_txbuf, int *tx_num, SEND_OBJ *p_send_obj);
extern int SocketPackString(unsigned char *const p_txbuf, int *const p_tx_num, char *const p_string, int length, unsigned char sub_index=0, unsigned int timestamp=0);
extern int SocketPackIplImage(unsigned char *const p_txbuf, int *const p_tx_num, IplImage *const p_img, unsigned char sub_index=0, unsigned int timestamp=0);
extern int SocketPackArray(unsigned char *const p_txbuf, int *const p_tx_num, CvMat * const p_cv_mat, unsigned char sub_index=0, unsigned int timestamp=0);
extern int ReleaseData_DecodingResultObj(DECODING_RESULT *p_obj);
#endif