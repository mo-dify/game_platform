#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include<sys/epoll.h>
#include"main.h"



int main(int argc,char* argv[])
{
	ME me;
	bzero(&me,0);
	PME p=&me;
	int sfd;
	int usfd;
	int ret;
	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex,NULL);
	srand((int)time(0));//初始化时钟
	int i=8080;
//	int efd, i, nfound; 
//	struct epoll_event ev, evs[2];

	socklen_t	len;
	struct sockaddr_in sin;
	bzero(&sin,0);
	pthread_t tid;
	//创建udp套接字******************************************
	usfd=socket(AF_INET,SOCK_DGRAM,0);
	if(usfd == -1)
	{
		perror("socket");
		return -1;
	}
	p->usfd=usfd;
	printf("usfd===%d\n",usfd);

	//创建tcp套接字***************************************************************
	sfd = socket(AF_INET, SOCK_STREAM,0);
	if(sfd == -1)
	{
		perror("socket");
		return -1;
	}
	p->sfd=sfd;
	printf("sfd===%d\n",sfd);

	//2,连接
	sin.sin_family = AF_INET;
	sin.sin_port = htons(8080);
	inet_pton(AF_INET,"127.0.0.1",&sin.sin_addr.s_addr);
//	inet_pton(AF_INET,"192.168.43.121",&sin.sin_addr.s_addr);
//	inet_pton(AF_INET,"192.168.43.185",&sin.sin_addr.s_addr);
	ret = connect(sfd,(const struct sockaddr*)&sin,(socklen_t)sizeof(sin));
	if(ret == -1)
	{
		perror("connect");
		return -1;
	}
	//心跳线程
#if 1
	pthread_create(&tid,NULL,heart_beat,p);
	pthread_detach(&tid);
#endif
	//3进入菜单选项
	enter(p);
	//6,关闭
	close(sfd);

}



