/* ************************************************************************
 *       Filename:  main.h
 *    Description:  
 *        Version:  
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  YOUR NAME (), 
 *        Company:  
 * ************************************************************************/

#ifndef   _MAIN_H_
#define   _MAIN_H_
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netinet/tcp.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include"main.h"
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/epoll.h>
#include<strings.h>
#include<time.h>

#define N 16
//服务器：存放读取到的客户信息的数据
typedef struct client{
	char id[64];//帐号
	char name[64];//姓名
	char pwd[108];//密码
	char score[100];
	struct client *pre;
	struct client *next;
}CLIENT,*PCLIENT;

//服务器/客户端通讯协议
typedef struct head{
	unsigned short type;
	unsigned short length;//为什么有这个长度，是为了节省带宽，用多少，取多少
}HEAD,*PHEAD;
//聊天/游戏信息结构体
typedef struct chat{
	char name[64];
	char id[100];
	char say[1024];
	int x;
	int y;
	int flag;
}CHAT,*PCHAT;

//服务器：在线用户信息构体
typedef struct node{
	int match_cfd;//游戏对手的cfd
	int num;//心跳计数
	int cfd;//套接字
	struct sockaddr_in sin;//包含ip信息的结构体
	char id[100];//帐号
	char name[100];//姓名
	struct node * next;
}CLT,*PCLT;

typedef union body{
	CLT olc;
	CHAT chat;	
}BODY,*PBODY;



//客户端：棋子坐标结构体
struct zuobiao{
	int x[N*N];
	int y[N*N];
}weizhi[N*N];//每一个棋子的位置
//客户端：存放该客户端当前登录客户的信息的结构体
typedef struct me{
	char name[100];
	char id[100];
	char score[100];
	int sfd;
	int usfd;
}ME,*PME; 


extern int keep_alive(int cfd);
//服务器应用**************************************************************
//创建客户链表
extern PCLIENT create_link_list_allclient(const char *path);
//保存客户信息
extern void save_client(const PCLIENT head);
//客户端发生错误
extern int client_error(int *v,PCLT *ochead);
//客户端事件解析
extern void client_do(int  v,PCLT *ochead,PCLIENT* chead);
//
extern void * beat_check(void* ochead);





//客户端应用*************************************************************
extern void client_enter(PME p);
extern void client_enroll(PME p);
extern int  enter(PME p);
extern int send_fun(PME p);
extern void recv_fun(PME p);
extern void * heart_beat(void* pp);

//五子棋相关函数
extern void welcome();
extern void initqipan();
extern int zouqihang();
extern int zouqilie();
extern void save(int p);
extern int showqi(int i);
extern int panduan(int p);
extern void heqi();
extern void over();
extern int duplicate_check(int i);
extern int duplicate_check1(int i);









#endif
