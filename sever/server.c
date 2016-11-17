
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include "main.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include<sys/epoll.h>
/*
 server.c
 */
//char fds[1024]={1,1,1};
//int max_fd = 2;




int main()
{
	int sfd;
	int cfd;
	int ret;
	char buf[1024]="";
	char cip[16]="";
	pthread_t tid;//心跳检测线程
	PCLIENT c_head=NULL;//所有用户的结构体的指针
	PCLIENT *chead=&c_head;//所有用户信息的链表头指针,二级指针一定要初始化

	PCLT oc_head=NULL;//在线用户链表头指针
	PCLT *ochead=&oc_head;//在线用户头指针的二级指针
	PCLT o=NULL;
	int efd,nfound,i;
	struct epoll_event ev,evs[20];

	struct sockaddr_in sin;
	struct sockaddr_in cin;

	socklen_t	len = sizeof(cin);
	pthread_mutex_t mutex;//创建锁
	pthread_mutex_init(&mutex,NULL);//初始化锁
	c_head=create_link_list_allclient("./client.txt");//


	//1,创建套接字
	sfd = socket(AF_INET, SOCK_STREAM,0);
	if(sfd == -1)
	{
		perror("socket");
		return -1;
	}
	//2,绑定
	//设定提供服务的ip和port
	sin.sin_family = AF_INET;
	sin.sin_port = htons(8080);
	inet_pton(AF_INET,"127.0.0.1",&sin.sin_addr.s_addr);
//	inet_pton(AF_INET,"192.168.43.121",&sin.sin_addr.s_addr);
//	inet_pton(AF_INET,"192.168.43.185",&sin.sin_addr.s_addr);
	ret = bind(sfd,(const struct sockaddr *)&sin, (socklen_t)sizeof(sin));
	if(ret == -1)
	{
		perror("bind");
		return -1;
	}

	//3,监听
	ret = listen(sfd, 30);
	if(ret == -1)
	{
		perror("listen");
		return -1;
	}

	//4,接收
	efd = epoll_create(1000);
	ev.events = EPOLLET|EPOLLIN;
	ev.data.fd = sfd;
	epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &ev);

	//开启心跳检测线程
	ret = pthread_create(&tid,NULL,beat_check,(void*)ochead);
	if(ret <0)
	{
		perror("pthread_create client");
		exit(-1);
	}
	pthread_detach(tid);
	//事件循环
	while(1)
	{
		printf("等待事件发生.................\n");
		nfound = epoll_wait(efd, evs , 20, -1);
		printf("！！！！！！！！！！！！！！！！！！！！！！！！！！！！！发生事件\n");
		for(i=0;i<nfound;i++)
		{
			if(evs[i].data.fd == sfd)
			{
				cfd = accept(sfd, (struct sockaddr *)&cin,(socklen_t*)&len);
				//accept会把接收的结构体大小写入len
				if(cfd <0){
					perror("accept");
					close(sfd);
					return -1;
				}
				//增加新cfd到在线链表
				o = malloc(sizeof(CLT));
				memset(o,0,sizeof(CLT));
				(o->num)+=1;
				printf("%d\n",o->num);
				o->cfd = cfd;
				o->sin=cin;

				//显示客户端ip和port
				printf("clinet port(%d)\n",ntohs(o->sin.sin_port));
				inet_ntop(AF_INET, (const void*)&o->sin.sin_addr.s_addr,cip,16);
				printf("client ip(%s)\n",cip);
				//添加链表
				pthread_mutex_lock(&mutex);
				o->next = oc_head;
				oc_head = o;
				pthread_mutex_unlock(&mutex);
				//将新链接的cfd
				ev.events = EPOLLET|EPOLLIN;
				ev.data.fd = cfd;
				epoll_ctl(efd, EPOLL_CTL_ADD, cfd, &ev);
			}
			else if(evs[i].events&EPOLLIN)
			{
				cfd = evs[i].data.fd;
				client_do(cfd,ochead,chead);
			}
		}
	}//end while(1) 

	//6,关闭
	close(sfd);

}
