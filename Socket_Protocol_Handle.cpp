/**************************************************************************
�ļ���	��Socket_Protocol_Handle.cpp
����	��dly
�汾	��2.0
�޸�ʱ�䣺2014-08-24//��������ϰ�����
����	�����ݡ��������ݴ���Э��-2.0��ʵ�ְ�Э���װ���ݰ�,�������ݰ�����
�ܵĴ����ʽ����ͷ���������� + ��������(1) + ʱ���(4) + ���ĳ���(4) + �������� + ��β���������루ͬ��ͷ��
***********************************************************************/
#include "Socket_Protocol_Handle.h"	
char g_err_info[128];	//�������������Ϣ
DECODING_RESULT g_socket_decoding_result;
SEND_OBJ g_package_obj;
/**************************�������ݰ�***************************/

/*
����	��������������
����	��&1���������ָ��	&2�����뻺��ָ��  &3�����泤��
����ֵ	��������ɱ�־��0��ʾ������ɣ�1��ʾ���ڻ������ݴ�����,-1��������
��ע	��������Ϊ���ݷ���ռ䣬��������ͷſռ�
*/
int SocketDecoder(DECODING_RESULT *const p_decoding_result, unsigned char *const p_rxbuf, int rx_num, char *const err_info)
{
	if(rx_num < 20)
	{
		strcpy(err_info, "rxbuf is two short!\n");
		p_decoding_result->index = INDEX_NULL;
		return ERR_FAULT;
	}
	if(strcmp((char *)p_rxbuf, "STRING")==0)//����ͷ�Ƿ�Ϊ�ַ���
	{
		//��ȡ���ݳ���
		unsigned int *p_len = (unsigned int *)(p_rxbuf+sizeof("STRING")+1+4);
		p_decoding_result->total_byte_num = sizeof("STRING")*2+9+*p_len;
		if(rx_num < p_decoding_result->total_byte_num)	//У�����ݳ���
		{
			//�������ݳ��ȴ�����
			strcpy(err_info, "STRING:Broken!");
			p_decoding_result->index = INDEX_NULL;
			return ERR_BROKEN;
		}
		else if(strcmp((char *)(p_rxbuf+sizeof("STRING")+9+*p_len), "STRING")!=0)	//����β
		{
			//��β������
			strcpy(err_info, "error in STRING message end!\n");
			p_decoding_result->index = INDEX_NULL;
			return ERR_FAULT;
		}
		else if(*(p_rxbuf+sizeof("STRING")+9+*p_len-1)!='\0')	//����ַ���'\0'
		{
			strcpy(err_info, "error in string message data\n");
			p_decoding_result->index = INDEX_NULL;
			return ERR_FAULT;
		}
		else	//��ȷ
		{
			p_decoding_result->index = INDEX_STRING;
			p_decoding_result->sub_index = *(p_rxbuf+sizeof("STRING"));
			p_decoding_result->timestamp = *(unsigned int *)(p_rxbuf+sizeof("STRING")+1);
			p_decoding_result->length = *(unsigned int *)(p_rxbuf+sizeof("STRING")+5);
			p_decoding_result->p_string = new char [p_decoding_result->length];	//��̬����ռ�
			memcpy(p_decoding_result->p_string, (p_rxbuf+sizeof("STRING")+9), p_decoding_result->length);
			strcpy(err_info, "string ok!\n");
			if(rx_num > p_decoding_result->total_byte_num)
				return ERR_STICKING;
			else
				return ERR_OK;
		}
	}
	else if(strcmp((char *)p_rxbuf ,"ARRAY")==0)//����
	{
		//��ȡ���ݳ���
		unsigned int *p_len = (unsigned int *)(p_rxbuf+sizeof("ARRAY")+5);
		p_decoding_result->total_byte_num = sizeof("ARRAY")*2+9+*p_len;
		if(rx_num < p_decoding_result->total_byte_num)	//���ճ���С��ָ�����ȱ���
		{
			//���ݳ��ȴ�����
			strcpy(err_info,"ARRAY:Broken!");
			p_decoding_result->index = INDEX_NULL;
			return ERR_BROKEN;
		}
		else if(strcmp((char *)(p_rxbuf+sizeof("ARRAY")+9+*p_len), "ARRAY")!=0)	//��β
		{
			//��β������
			strcpy(err_info,"error in ARRAY end\n");
			p_decoding_result->index = INDEX_NULL;
			return ERR_FAULT;
		}
		else
		{
			unsigned char *p_data = (unsigned char *)(p_rxbuf+sizeof("IMAGE")+9);
			unsigned int rows = *(unsigned int *)p_data;
			unsigned int  cols = *(unsigned int *)(p_data+4);
			int type = *(int *)(p_data+8);
			CvMat *& p_mat = p_decoding_result->p_mat;
			p_mat = cvCreateMat(rows, cols, type);
			if((unsigned int)(*p_len - 12) != p_mat->step*p_mat->rows)
			{
				cvReleaseMat(&p_mat);
				strcpy(err_info,"error in ARRAY property\n");		
				p_decoding_result->index = INDEX_NULL;
				return ERR_FAULT;
			}
			else
			{
				memcpy(p_mat->data.ptr, (p_data+12), p_mat->step*p_mat->rows);	//ע������widthStep
				p_decoding_result->index = INDEX_ARRAY;
				p_decoding_result->sub_index = *(p_rxbuf+sizeof("ARRAY"));
				p_decoding_result->timestamp = *(unsigned int *)(p_rxbuf+sizeof("ARRAY")+1);
				strcpy(err_info,"ARRAY ok!\n");
				if(rx_num > p_decoding_result->total_byte_num)
					return ERR_STICKING;
				else
					return ERR_OK;
			}

		}
	}
	else if(strcmp((char *)p_rxbuf, "IMAGE")==0)//ͼ��
	{
		//��ȡ���ݳ���
		unsigned int *p_len = (unsigned int *)(p_rxbuf+sizeof("IMAGE")+5);
		p_decoding_result->total_byte_num = sizeof("IMAGE")*2+9+*p_len;
		if(rx_num < p_decoding_result->total_byte_num)	//���ճ���С��ָ�����ȱ���
		{
			//���ݳ��ȴ�����
			p_decoding_result->index = INDEX_NULL;
			strcpy(err_info,"image:Broken!");
			return ERR_BROKEN;
		}
		else if(strcmp((char *)(p_rxbuf+sizeof("IMAGE")+9+*p_len), "IMAGE")!=0)	//��β
		{
			//��β������
			strcpy(err_info,"error in image end\n");
			p_decoding_result->index = INDEX_NULL;
			return ERR_FAULT;
		}
		else
		{
			unsigned char *p_data = (unsigned char *)(p_rxbuf+sizeof("IMAGE")+9);
			unsigned int width = *(unsigned int *)p_data;
			unsigned int  height = *(unsigned int *)(p_data+4);
			unsigned char channels = *(unsigned char *)(p_data+8);
			unsigned char depth = *(unsigned char *)(p_data+9);
			IplImage *&p_img = p_decoding_result->p_img;
			p_img = cvCreateImage(cvSize(width, height), depth, channels); 
			if((unsigned int)(*p_len - 10) != p_img->widthStep*p_img->height)	//���ж�IplImage�е�����
			{
				cvReleaseImage(&p_img);
				strcpy(err_info,"error in image property\n");
				p_decoding_result->index = INDEX_NULL;			
				return ERR_FAULT;
			}
			else
			{
				memcpy(p_img->imageData, (p_data+10), p_img->widthStep*p_img->height);	//ע������widthStep
				p_decoding_result->index = INDEX_IMAGE;
				p_decoding_result->sub_index = *(p_rxbuf+sizeof("IMAGE"));
				p_decoding_result->timestamp = *(unsigned int *)(p_rxbuf+sizeof("IMAGE")+1);
				strcpy(err_info,"image ok!\n");
				if(rx_num > p_decoding_result->total_byte_num)
					return ERR_STICKING;
				else
					return ERR_OK;
			}
		}
	}	
	else if(strcmp((char *)p_rxbuf, "USER")==0)//�û�
	{
		return ERR_OK;
	}
	else//��������
	{
		strcpy(err_info, "error message!\n");
		p_decoding_result->index = INDEX_NULL;
		return ERR_FAULT;
	}
}

/*
�ͷ�DECODING_RESULT�ṹ��ָ���ڵ�����ָ��,�����Խṹ��ͷ������
*/
int ReleaseData_DecodingResultObj(DECODING_RESULT *p_obj)
{
	if(p_obj->index == INDEX_STRING)
	{
		if(p_obj->p_string!=NULL)
		{
			delete [] p_obj->p_string;	
		}
	}
	else if(p_obj->index == INDEX_IMAGE)
	{
		if(p_obj->p_img!=NULL)
		{
			cvReleaseImage(&p_obj->p_img);
		}
	}
	else if(p_obj->index == INDEX_ARRAY)
	{
		if(p_obj->p_mat!=NULL)
		{
			cvReleaseMat(&p_obj->p_mat);
		}
	}
	return ERR_OK;
}

/*
����	����װָ�����͵�����
����	��&1�����ͻ���ָ��	&2������ָ��	&3���������ݵĽṹ��ָ��
����ֵ	����װ�ɹ���־
��ע	�����ͻ����СҪ�㹻
*/
int SocketPackager(unsigned char *p_txbuf, int *p_tx_num, SEND_OBJ *p_pack_obj)
{
	if(p_pack_obj->index == INDEX_STRING)	//��װ�ַ���
	{
		int i = 0;
		strcpy((char *)p_txbuf, "STRING");	//��ͷ
		i += 7;
		p_txbuf[i] = p_pack_obj->sub_index;	//������
		i += 1;
		*(unsigned int *)(p_txbuf+i) = p_pack_obj->timestamp;	//ʱ���
		i += 4;
		*(unsigned int *)(p_txbuf+i) = p_pack_obj->length;		//���ݳ���
		i += 4;
		memcpy(p_txbuf+i, p_pack_obj->p_string, p_pack_obj->length);	//copy����
		i += p_pack_obj->length;
		strcpy((char *)(p_txbuf+i), "STRING");	//��β
		i += 7;
		*p_tx_num = i;	//���ͻ��泤��
		return ERR_OK;
	}
	else if(p_pack_obj->index == INDEX_ARRAY)
	{
		int i = 0;
		strcpy((char *)p_txbuf, "ARRAY");	//��ͷ
		i += 6;
		p_txbuf[i] = p_pack_obj->sub_index;	//������
		i += 1;
		*(unsigned int *)(p_txbuf+i) = p_pack_obj->timestamp;	//ʱ���
		i += 4;
		int mat_bytes_num = p_pack_obj->p_mat->step * p_pack_obj->p_mat->rows;	//mat��ռ���ֽ���
		//		printf("%d,%d,%d",p_pack_obj->p_mat->step, p_pack_obj->p_mat->rows, p_pack_obj->p_mat->cols);
		*(unsigned int *)(p_txbuf+i) = mat_bytes_num + 12;		//���ݳ��ȣ�����step����
		i += 4;
		*(unsigned int *)(p_txbuf+i) = p_pack_obj->p_mat->rows;	//����
		i += 4;
		*(unsigned int *)(p_txbuf+i) = p_pack_obj->p_mat->cols;	//����
		i += 4;
		*(int *)(p_txbuf+i) = p_pack_obj->p_mat->type;	//��������
		i += 4;
		memcpy(p_txbuf+i, p_pack_obj->p_mat->data.ptr, mat_bytes_num);	//copy����
		i += mat_bytes_num;
		strcpy((char *)(p_txbuf+i), "ARRAY");	//��β
		i += 6;
		*p_tx_num = i;	//���ͻ��泤��
		return ERR_OK;

	}
	else if(p_pack_obj->index == INDEX_IMAGE)
	{
		int i=0, j=0;
		unsigned int *len;
		IplImage * &p_src_img = p_pack_obj->p_img;	
		int image_bytes_num = p_src_img->widthStep*p_src_img->height;	//IplImageͼ������ռ���ֽ���
		//IMAGE
		strcpy((char *)p_txbuf, "IMAGE");	//��ͷ
		i += 6;
		p_txbuf[i] = p_pack_obj->sub_index;	//������
		i += 1;
		*(unsigned int *)(p_txbuf+i) = p_pack_obj->timestamp;	//ʱ���
		i += 4;
		len = (unsigned int*)(p_txbuf + i);
		*len = image_bytes_num+4+4+1+1;
		i = i+4;	//����
		*(unsigned int*)(p_txbuf + i) = p_src_img->width;
		i = i+4;
		*(unsigned int*)(p_txbuf + i) = p_src_img->height;
		i = i+4;
		*(unsigned int*)(p_txbuf + i) = p_src_img->nChannels;
		i = i+1;
		*(unsigned int*)(p_txbuf + i) = p_src_img->depth;
		i = i+1;
		memcpy(&p_txbuf[i], p_src_img->imageData, image_bytes_num);
		i = i+ image_bytes_num;
		strcpy((char *)(p_txbuf+i), "IMAGE");	//��β
		i += 6;
		*p_tx_num = i;	//���ͻ��泤��
		return ERR_OK;
	}
	return ERR_FAULT;
}
/*
����	����װ�ַ��������ͻ���
����	��&1�����ͻ���ָ��	&2������ָ��	&3:���ͽṹ�壬&4�����͵��ַ��� &5���ַ������� &6�������� &7��ʱ���
����ֵ	����װ�ɹ���־
��ע	�����ͻ����СҪ�㹻
*/
int SocketPackString(unsigned char *const p_txbuf, int *const p_tx_num, char *const p_string, int length, unsigned char sub_index, unsigned int timestamp)
{
	SEND_OBJ package_obj;
	package_obj.index = INDEX_STRING;
	package_obj.sub_index = sub_index;
	package_obj.timestamp = timestamp;
	package_obj.p_string = p_string;
	package_obj.length = length;
	SocketPackager(p_txbuf, p_tx_num, &package_obj);
	return ERR_OK;
}
/*
����	����װIplImage�����ͻ���
����	��&1�����ͻ���ָ��	&2������ָ�� &3�����ͽṹ�� &4�����͵�ͼ��
����ֵ	����װ�ɹ���־
��ע	�����ͻ����СҪ�㹻
*/
int SocketPackIplImage(unsigned char *const p_txbuf, int *const p_tx_num, IplImage *const p_img, unsigned char sub_index, unsigned int timestamp)
{
	SEND_OBJ package_obj;
	package_obj.index = INDEX_IMAGE;
	package_obj.sub_index = sub_index;
	package_obj.timestamp = timestamp;
	package_obj.p_img = p_img;
	SocketPackager(p_txbuf, p_tx_num, &package_obj);
	return ERR_OK;
}
/*
����	����װCvMat�����ͻ���
����	��&1�����ͻ���ָ��	&2������ָ��	&3:���ͽṹ�壬&4��CvMatָ�� &5�������� &6��ʱ���
����ֵ	����װ�ɹ���־
��ע	�����ͻ����СҪ�㹻
*/
int SocketPackArray(unsigned char *const p_txbuf, int *const p_tx_num, CvMat * const p_cv_mat, unsigned char sub_index, unsigned int timestamp)
{
	SEND_OBJ package_obj;
	package_obj.index = INDEX_ARRAY;
	package_obj.sub_index = sub_index;
	package_obj.timestamp = timestamp;
	package_obj.p_mat = p_cv_mat;
	SocketPackager(p_txbuf, p_tx_num, &package_obj);
	return ERR_OK;	
}

/*************************************************************************************************************************************************************
////****ʹ�÷���
*************************************************************************************************************************************************************/
/*
/////////////////���ͱ���//////////////////
while(1)
{
	Sleep(4);
	int tx_num=0;
	SocketPackString((unsigned char *)txbuf, &tx_num, "i am dly!", sizeof("i am dly!"));
	bytesSent = send(socket, txbuf, tx_num, 0);
	if( bytesSent == SOCKET_ERROR)
	{
		printf("\tCommunicationThread\tsend error %d\n", 
			WSAGetLastError());
		closesocket(socket);
		return 1;
	}
	SocketPackIplImage((unsigned char *)txbuf, &tx_num, p_src_img);
	bytesSent = send(socket, txbuf, tx_num, 0);
	//		Sleep(5);
	if( bytesSent == SOCKET_ERROR)
	{
		printf("\tCommunicationThread\tsend error %d\n", 
			WSAGetLastError());
		closesocket(socket);
		return 1;
	}
	CvMat *p_mat = cvCreateMat(5,3, CV_32FC1);
	static int x=0;
	for(int j=0; j<p_mat->rows; j++)
	{
		for(int i=0; i<p_mat->cols; i++)
		{
			*(float *)CV_MAT_ELEM_PTR(*p_mat, j, i) = x++;
		}
	}
	SocketPackArray((unsigned char *)txbuf, &tx_num, p_mat);
	cvReleaseMat(&p_mat);
	bytesSent = send(socket, txbuf, tx_num, 0);
	//		Sleep(1);
	if( bytesSent == SOCKET_ERROR)
	{
		printf("\tCommunicationThread\tsend error %d\n", 
			WSAGetLastError());
		closesocket(socket);
		return 1;
	}
}
/////////////////////////////���ձ���///////////////////////////////
while(1)
{
	// ��������
	int bytesRecv = recv(socket, // socket
		rxbuf, // ���ջ���
		RECV_BUF_SIZE, // �����С
		0);// ��־
	printf("recv_byte=%d\n",bytesRecv);
	if (bytesRecv == 0)// ��������ʧ�ܣ������Ѿ��ر�
	{
		printf("Connection closing...\n");
		closesocket(socket);
		return 1;
	}
	else if (bytesRecv == SOCKET_ERROR)// ��������ʧ�ܣ�socket����
	{
		printf("recv failed: %d\n", WSAGetLastError());
		closesocket(socket);
		return 1;
	}
	else if(bytesRecv>0)
	{
		int result = ERR_STICKING;
		int rx_num = bytesRecv;
		char *p_buf = rxbuf;
		g_socket_decoding_result.total_byte_num = 0;	
		while(result == ERR_STICKING || result == ERR_BROKEN)
		{
			if(result == ERR_STICKING)	
			{
				result = SocketDecoder(&g_socket_decoding_result, (unsigned char*)p_buf, rx_num, g_err_info);
				if(result == ERR_FAULT)printf(g_err_info);
				if(result != ERR_OK && result != ERR_STICKING)	//���û�н��ն���Ч�����ݰ���������ѭ��
					continue;
				p_buf += g_socket_decoding_result.total_byte_num;
				rx_num -= g_socket_decoding_result.total_byte_num;
				if (g_socket_decoding_result.index == INDEX_STRING)
				{
#ifdef DEBUG_DISPLAY_STRING
					printf(g_socket_decoding_result.p_string);
#endif
				}
				else if(g_socket_decoding_result.index == INDEX_ARRAY)
				{
					CvMat *&p_mat = g_socket_decoding_result.p_mat;
#ifdef DEBUG_DISPLAY_ARRAY
					printf("sub_index=%d,timestamp=%d\n", g_socket_decoding_result.sub_index, g_socket_decoding_result.timestamp);
#endif
					for(int j=0; j<p_mat->rows; j++)
					{
						for(int i=0; i<p_mat->cols; i++)
						{
							float element = *(float *)CV_MAT_ELEM_PTR(*p_mat, j, i);
#ifdef DEBUG_DISPLAY_ARRAY
							printf("%f	", element);
#endif
						}
#ifdef DEBUG_DISPLAY_ARRAY
						printf("\n");
#endif
					}
				}
				else if(g_socket_decoding_result.index == INDEX_IMAGE)
				{
#ifdef DEBUG_DISPLAY_IMAGE
					cvShowImage("rx_image",g_socket_decoding_result.p_img);
					cvWaitKey(1);
#endif
				}
				ReleaseData_DecodingResultObj(&g_socket_decoding_result);	//�ͷſռ�
			}
			else if(result == ERR_BROKEN)
			{
				if(rx_num > RECV_BUF_SIZE)
				{
					printf("recv_buf_size is not enough!\n");
				}
				else
				{
					bytesRecv = recv(socket, // socket
						p_buf+rx_num, // ��ƫ�ƵĽ��ջ���
						RECV_BUF_SIZE-rx_num, // ���ջ����С	//�������½��ջ���
						0);// ��־
					printf("recv_byte=%d\n",bytesRecv);
					if(bytesRecv>0)
					{
						rx_num += bytesRecv;
						result = ERR_STICKING;
					}
					else
					{
						result = ERR_FAULT;
						printf("socket error!\n");
					}
				}
			}
		}
	}
}
*/

