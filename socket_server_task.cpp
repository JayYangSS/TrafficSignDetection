#include "socket_server_task.h"
/* ���� */
#define DEFAULT_PORT "10004" // �˿�
#define MAX_REQUEST 1024 // �������ݵĻ����С
#define BUF_SIZE 4096 // �������ݵĻ����С
/*���ͱ�־λ*/
bool gb_filled = false;
/*���͵�����*/
//float g_data = 0;	
CvMat *g_mat;
/*������տͻ��������߳�*/
extern DWORD WINAPI Thread_AcceptHand(LPVOID lpParameter);
/*
����������߳�
����ͼ��͵�����
*/
DWORD WINAPI Thread_Task_Send(LPVOID lpParameter);

SOCKET ListenSocket = INVALID_SOCKET;// ����socket
SOCKET ClientSocket = INVALID_SOCKET;// ����socket
/*
��ʼ������
*/
int SocketInit(void)
{
	WSADATA wsaData;
	struct addrinfo *result = NULL,
		hints;
	int iResult;// ���淵�ؽ��
	// ��ʼ��Winsock����֤Ws2_32.dll�Ѿ�����
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed: %d\n", iResult);
		return 0;
	}
	// ��ַ
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// ��ȡ������ַ����֤����Э����õ�
	iResult = getaddrinfo(NULL, // ����
		DEFAULT_PORT, // �˿�
		&hints, // ʹ�õ�����Э�飬�������͵�
		&result);// ���
	if ( iResult != 0 )
	{
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 0;
	}

	// ����socket�����ڼ���
	ListenSocket = socket(
		result->ai_family, // ����Э�飬AF_INET��IPv4
		result->ai_socktype, // ���ͣ�SOCK_STREAM
		result->ai_protocol);// ͨ��Э�飬TCP
	if (ListenSocket == INVALID_SOCKET)
	{
		printf("socket failed: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 0;
	}
	// �󶨵��˿�
	iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 0;
	}
	printf("bind\n");

	freeaddrinfo(result);// reuslt����ʹ��

	// ��ʼ����
	iResult = listen(ListenSocket, SOMAXCONN);
	printf("start listen......\n");
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 0;
	}
	LPVOID lpParameter = NULL;
	/*�����߳̽�������*/
	CreateThread(
		NULL,
		0,
		Thread_AcceptHand, // �̺߳���
		(LPVOID)lpParameter, // ��socket��Ϊ����
		0,
		NULL);
}
/*������տͻ��������߳�*/
DWORD WINAPI Thread_AcceptHand(LPVOID lpParameter)
{
	while(1)
	{
		// ���տͻ��˵����ӣ�accept������ȴ���ֱ�����ӽ���
		printf("ready to accept\n");
		ClientSocket = accept(ListenSocket, NULL, NULL);
		// accept�������أ�˵���Ѿ��пͻ�������
		// ��������socket
		printf("accept a connetion\n");
		if (ClientSocket == INVALID_SOCKET)
		{
			printf("accept failed: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			break;// �ȴ����Ӵ����˳�ѭ��
		}
		// Ϊÿһ�����Ӵ���һ�����ݷ��͵Ľ����̣߳�
		// ʹ������ֿ����������������ͻ��˵�����
		if(!CreateThread(
			NULL,
			0,
			Thread_Task_Send, // �̺߳���
			(LPVOID)ClientSocket, // ��socket��Ϊ����
			0,
			NULL))
		{
			printf("Create Thread error (%d)", GetLastError());
			break;
		}
	}
	// ѭ���˳����ͷ�DLL��
	WSACleanup();
	return 0;
}
/*
����������߳�
����ͼ��͵�����
*/
DWORD WINAPI Thread_Task_Send(LPVOID lpParameter)
{
	DWORD dwTid = GetCurrentThreadId();
	// ��ò���sokcet
	SOCKET socket = (SOCKET)lpParameter;
	// Ϊ�������ݷ���ռ�
	int iResult;
	int bytesSent;// ���ڱ���send�ķ���ֵ��ʵ�ʷ��͵����ݵĴ�С
//	IplImage *p_src_img = cvLoadImage("lena.jpg", CV_LOAD_IMAGE_COLOR);
	//	IplImage *p_src_img = cvLoadImage("tiggo.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	char *txbuf = new char [2000000];
//	CvMat *p_mat = cvCreateMat(1,1, CV_32FC1);
	while(1)
	{
		int tx_num=0;
		while(!gb_filled)	//�ȴ����ݵ�װ��
		{
			Sleep(10);		}
//		SocketPackIplImage((unsigned char *)txbuf, &tx_num, p_src_img);
//		bytesSent = send(socket, txbuf, tx_num, 0);
//		Sleep(10);
		
//		*(float *)CV_MAT_ELEM_PTR(*p_mat, 0, 0) = g_data;
		SocketPackArray((unsigned char *)txbuf, &tx_num, g_mat);

		bytesSent = send(socket, txbuf, tx_num, 0);
		Sleep(10);
		if( bytesSent == SOCKET_ERROR)
		{
			printf("\Thread_Task_Send\tsend error %d\n", 
				WSAGetLastError());
			closesocket(socket);
			return 1;
		}
		gb_filled = false;	//����װ�����
	}
//	cvReleaseMat(&p_mat);
	// �ͷŽ������ݻ��棬�ر�socket
	delete [] txbuf;
	closesocket(socket);
	return 0;
}