#ifndef __SOCKET_SERVER_H
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <vector>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")	//socket

/*���ͱ�־λ*/
extern bool gb_filled;
/*���͵�����*/
extern char sendMsg[2000000];
//extern float g_data;	
extern int SocketInit(void);
int packData(vector<double> data);
#endif