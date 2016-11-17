/* ************************************************************************
 *       Filename:  server_fun.c
 *    Description:  
 *        Version:  1.0
 *        Created:  2016年09月17日 11时09分01秒
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  YOUR NAME (), 
 *        Company:  
 * ************************************************************************/
#include"main.h"

pthread_mutex_t mutex;//锁

/***************************************************************
 *	
 *			函数功能：保存链表信息到文档中
 *	
 *			参数：head 需要保存链表的头指针
 *
 *			返回值：无
 *
 * ************************************************************/
 void save_client(const PCLIENT head)
{
	FILE *fp;
	PCLIENT p=head;
	fp =fopen("./client.txt","w");
	while(p)
	{
		fprintf(fp ,"%s\t%s\t%s\t%s\n",	p->id,p->pwd,p->name,p->score);
		p=p->next;
	}
	fclose(fp);
}

/***************************************************************
 *
 *			函数功能：读取文档内容，保存到链表中
 *	
 *			参数：path 需要读取文档的路径
 *
 *			返回值：head 保存有信息的链表的头指针
 *
 * ************************************************************/
PCLIENT create_link_list_allclient(const char* path)
{	
	struct client *head =NULL;
	struct client *p;
	char buf[1024];
	FILE *fp;
	int ret;

	if(path==NULL)
		return NULL;
	fp =fopen(path,"r");
	if(fp==NULL){
		sprintf(buf,"open %s",path);
		perror(buf);
		return	 NULL;
	}
	while(1)
	{
		p=malloc(sizeof(struct client));
		if(p==NULL){
			perror("malloc student");
			return NULL;
		}
		ret =fscanf(fp ,"%s %s %s %s",p->id,p->pwd,p->name,p->score);
		if(ret <=0){
			perror("read client");
			free(p);
			break;
		}
		if(head==NULL){
			p->next=NULL;
			p->pre=NULL;
			head=p;
		}else{
			p->next=head;
			head->pre=p;
			p->pre=NULL;
			head=p;
		}
	}
	fclose(fp);
	return head;
}

/***************************************************************
 *
 *			函数功能：	失败
 *	
 *			参数：		v     发生错误的套接字
 *						head  保存有所有套接字的链表
 *
 *			返回值：无
 *
 * ************************************************************/
int client_error(int *v ,PCLT *ochead)
{
	int cfd = *v;
	struct sockaddr_in cin;
	int len =sizeof(cin);
	char cip[16];
	int i;
	PCLT p,q;
	if(errno == ETIMEDOUT){
		printf("keepalive failed!\n");
	}
	getpeername(cfd,(struct sockaddr*)&cin,(socklen_t*)&len);
	printf("clinet port(%d) ",ntohs(cin.sin_port));
	inet_ntop(AF_INET, (const void*)&cin.sin_addr.s_addr,cip,16);
	printf("ip(%s) exit\n",cip);
	close(cfd);
	pthread_mutex_lock(&mutex);
	p = *ochead;
	while(p)
	{
		if(p->cfd == cfd)break;
		q = p;
		p= p->next;
	}
	if(p)
	{
		if(p== *ochead){
			*ochead = (*ochead)->next;
		}else
			q->next = p->next;
		free(p);
	}
	pthread_mutex_unlock(&mutex);
}
/***************************************************************
 *
 *			函数功能：	处理发生的事件事件
 *	
 *			参数：		v		发生事件的套接字
 *						ochead	二级指针，指向包含所有在线客户端信息的结构体链表的头指针
 *						chead	二级指针，指向包含有所有客户端信息的结构体链表的头指针

 *
 *			返回值：	无
 *
 * ************************************************************/
void client_do(int  v, PCLT *ochead,PCLIENT *chead)
{
	int cfd = v;//发生事件的套接字
	printf("发生事件cfd==%d\n",cfd);
	int ret;
	PCLT p;//发送给所有人的时候用
	HEAD h;//协议头
	BODY b;//协议包
	PCLIENT c=NULL;

	memset(&b,0,sizeof(b));
	memset(&h,0,sizeof(h));
	//读取头部
	ret = read(cfd,&h,4);
	printf("h.type==================%d\n",h.type);
	//注销或错误判断**************************************************
	if(h.type==1000)
	{
		printf("注销或错误\n");
		client_error(&cfd,ochead);
		printf("从error中出来了!!!!!!!");
		return;
	}
	//注册判断***********************************************************************
	if(h.type == 1001 && h.length>0)
	{
		printf("注册协议\n");
		ret = read(cfd,&b,h.length);
		if(ret<=0){
			client_error(&cfd,ochead);
			return ;
		}
		//判断帐号是否已经存在
		pthread_mutex_lock(&mutex);
		c=*chead;
		while(c)
		{
			if(!strcmp(c->id,b.chat.id))
			{	
				//告诉客户端注册失败
				h.type=10011;
				h.length=0;	
				write(cfd,&h,sizeof(h));
				pthread_mutex_unlock(&mutex);
				return;
			}
			else
				c = c->next;
		}
		//如果用户不存在，添加新的帐号信息到文本中
		if(c==NULL)
		{
			c=*chead;
			c = malloc(sizeof(struct client));
			strcpy(c->id,b.chat.id);
			strcpy(c->pwd,b.chat.say);
			strcpy(c->name,b.chat.name);
			strcpy(c->score,"0");
			//添加新的帐号到链表中
			if(*chead)
			{
				c->next=*chead;
				(*chead)->pre=c;
				c->pre = NULL;
				*chead = c;
			}
			else
			{
				c->pre=NULL;
				c->next=NULL;
				*chead = c;
			}
			//保存链表到文本
			save_client(*chead);
			//告诉客户端注册成功
			h.type=10010;
			h.length=0;	
			write(cfd,&h,sizeof(h));
			pthread_mutex_unlock(&mutex);
			return ;
		}
		return ;
	}
	//登录判断***********************************************************
	if(h.type == 1002 && h.length>0)
	{
		printf("登录协议\n");
		ret = read(cfd,&b,h.length);
		if(ret<=0){
			client_error(&cfd,ochead);
			return ;
		}
		printf("b.chat.name=%s\n",b.chat.id);
		printf("b.chat.say=%s\n",b.chat.say);	
		pthread_mutex_lock(&mutex);
		//判断帐号是否存在
		c = *chead;
		while(c->next)
		{
			if(!strcmp(c->id,b.chat.id))
				break;
			else
				c = c->next;
		}
		//判断密码是否正确
		if(!(strcmp(c->pwd,b.chat.say)))
		{
			h.type=10020;
			strcpy(b.chat.name,c->name);
			strcpy(b.chat.say,c->score);
			//添加登录用户信息到在线链表中
			p = *ochead;
			while(p)
			{
				if(p->cfd == cfd)
				{
					strcpy(p->id,c->id);
					strcpy(p->name,c->name);
					b.olc.sin=p->sin;
				}
				p = p->next;
			}
			//遍历打印在线链表，看量表是否正确
			p = *ochead;
			while(p)
			{
				printf("%s	%s	%d\n",p->id,p->name,p->cfd);
				printf("clinet port(%d)\n",ntohs(p->sin.sin_port));
		
				p=p->next;
			}
			//告诉客户端登录成功
			h.length=sizeof(b.chat.name)+sizeof(b.chat.say)+sizeof(b.olc.sin);		
			write(cfd,&h,sizeof(h));
			write(cfd,&b,h.length);
			pthread_mutex_unlock(&mutex);
			return ;
		}
		else
		{	
			//告诉客户端登陆失败
			pthread_mutex_unlock(&mutex);
			h.type=10021;
			h.length=0;
			write(cfd,&h,sizeof(h));
			return ;
		}
	}
		//2,解析聊天数据*******************************************************
		if(h.type == 1004 && h.length>0)
		{

			ret = read(cfd,&b,h.length);
			if(ret<=0)
			{
				client_error(&cfd,ochead);
				return;
			}
			//3,遍历链表，发送信息到客户端
			pthread_mutex_lock(&mutex);
			p = *ochead;
			while(p)
			{
				if(p->cfd != cfd)
				{
					printf("p->cfd=%d\n",p->cfd);
					write(p->cfd,&h,sizeof(h));
					write(p->cfd,&b,sizeof(b));
				}
				p = p->next;
			}
			pthread_mutex_unlock(&mutex);
			return ;
		}
		//游戏协议**************************************************	
		if(h.type == 1005)
		{
			printf("游戏协议\n");
			static int pcfd[2];
			static int i=0;
			i++;
			if(i==1)
			{
				pcfd[0]=cfd;//有一个玩家进入游戏室等
				return ;
			}
			if(i==2)
			{
				pcfd[1]=cfd;//有两个玩家进入到游戏室等待
//				printf("pcfd[1]==========%d\n",pcfd[1]);
//				printf("pcfd[0]==========%d\n",pcfd[0]);
				h.type=10050;//游戏先手协议
				write(pcfd[0],&h,sizeof(h));
				memset(&h,0,sizeof(h));

				//将玩家2信息发送给玩家1,并保存玩家1的cfd到玩家2的结构体中
				p = *ochead;
				while(p)
				{
					if(p->cfd == pcfd[1])//找到玩家2的信息
					{
						p->match_cfd=pcfd[0];
						b.olc=*p;//信息包括端口号和ip
						write(pcfd[0],&b,sizeof(b));//将玩家2的信息发给玩家1
					}
					p = p->next;
				}
				bzero(&b,0);

				h.type=10051;//游戏后手协议
				write(pcfd[1],&h,sizeof(h));

				//将玩家2信息发送给玩家1,并保存玩家1的cfd到玩家2的结构体中
				p = *ochead;
				while(p)
				{
					if(p->cfd == pcfd[0])//找到玩家1的信息
					{
						p->match_cfd=pcfd[1];
						b.olc=*p;//信息包含端口号和ip
						write(pcfd[1],&b,sizeof(b));//将玩家1的信息发给玩家2
					}
					p = p->next;
				}
				i=0;//状态量置0
				return;
			}
		}
		//查询协议解析*************************************************
		if(h.type == 1006 && h.length>0)
		{
			printf("查询协议\n");
			ret = read(cfd,&b,h.length);
			if(ret<=0){
				client_error(&cfd,ochead);
				return ;
			}
			//遍历在线链表
			pthread_mutex_lock(&mutex);
			c=*chead;
			while(c)
			{
				if(!strcmp(c->id,b.chat.id))
				{	
					h.type=1006;
					strcpy(b.chat.name,c->name);
					strcpy(b.chat.say,c->score);
					h.length=sizeof(b.chat.name)+sizeof(b.chat.say);
					write(cfd,&h,sizeof(h));
					write(cfd,&b,h.length);
					break;
				}
				else
					c = c->next;
			}
			pthread_mutex_unlock(&mutex);
			//如果查询id不存在
			if(c==NULL)
			{
				h.type=10060;
				h.length=0;	
				write(cfd,&h,sizeof(h));
				return ;
			}
			return;
		}
		//获取所有在线玩家协议***********************************************
		if(h.type ==1007)
		{
			printf("查看在线协议\n");
			//遍历在线链表，并发送给申请客户端
			pthread_mutex_lock(&mutex);
			p=*ochead;
			while(p)
			{	
				h.type=1007;
				strcpy(b.chat.name,p->name);
				strcpy(b.chat.id,p->id);
				h.length=sizeof(b.chat.name)+sizeof(b.chat.id);
				write(cfd,&h,sizeof(h));
				write(cfd,&b,h.length);
				p = p->next;
			}
			pthread_mutex_unlock(&mutex);
			//告诉客户端发送完毕
			h.type=10070;
			h.length=0;	
			write(cfd,&h,sizeof(h));
			return;
		}

		//在线客户端心跳检测************************************************************
		//正在完善，TO BE CONTINUE........
#if 1
		if(h.type ==1008)
		{
			printf("心跳协议\n");
			//遍历在线链表，并发送给申请客户端
			pthread_mutex_lock(&mutex);
			p=*ochead;
			while(p)
			{	
				if(p->cfd==cfd)
				{
					(p->num)+=1;
					printf("心跳计数num==%d\n",p->num);
					break;
				}
				p = p->next;
			}
			pthread_mutex_unlock(&mutex);
			//告诉客户端发送完毕
			return;
		}
#endif
		//加分协议解析*************************************************
		if(h.type == 1009)
		{
			printf("加分协议件\n");
			int i;
			if(ret<=0){
				client_error(&cfd,ochead);
				return ;
			}
			//遍历在线链表
			pthread_mutex_lock(&mutex);
			c=*chead;
			while(c)
			{
				if(!strcmp(c->id,b.chat.id))
				{	
					i=atoi(c->score);//转换成int型
					i+=10;
					sprintf(c->score,"%d",i);//重新写道从c->score
					save_client(*chead);//将改变的分数保存到文本中
					break;
				}
				else
					c = c->next;
			}
			pthread_mutex_unlock(&mutex);
			//如果查询id不存在
			return;
		}

		printf("第  %d  行\n",__LINE__);
		return;
}


int beat_error(int v ,PCLT *ochead)
{
	int cfd = v;
	struct sockaddr_in cin;
	int len =sizeof(cin);
	char cip[16];
	int i;
	PCLT p,q;
	if(errno == ETIMEDOUT){
		printf("keepalive failed!\n");
	}
	getpeername(cfd,(struct sockaddr*)&cin,(socklen_t*)&len);
	printf("clinet port(%d) ",ntohs(cin.sin_port));
	inet_ntop(AF_INET, (const void*)&cin.sin_addr.s_addr,cip,16);
	printf("ip(%s) exit\n",cip);
	close(cfd);
	p = *ochead;
	while(p)
	{
		if(p->cfd == cfd)break;
		q = p;
		p= p->next;
	}
	if(p)
	{
		if(p== *ochead){
			*ochead = (*ochead)->next;
		}else
			q->next = p->next;
		free(p);
	}
}

/***************************************************************
 *	
 *			函数功能：心跳检测线程
 *	
 *			参数：ochea 二级指针，指向在线链表头指针
 *
 *			返回值：无
 *
 * ************************************************************/
void * beat_check(void *ochead)
{
	PCLT* head=(PCLT*)ochead;
	PCLT p=NULL;
	PCLT q=NULL;
	HEAD h;//协议头
	while(1)
	{
		sleep(10);
		pthread_mutex_lock(&mutex);
		p=*head;
		while(p)
		{	
			if(p->num > 0)
			{
				p->num=0;
				printf("\n\n心跳检测成功，num==%d\n",p->num);
			}
			else if(p->num==0)
			{
				if(p->match_cfd!=0)//如果该掉玩家正在游戏则告诉另一名玩家进行托管
				{

					printf("发送托管命令\n");
					
					printf("p->match_cfd%d\n",p->match_cfd);
					h.type=10053;
					write(p->match_cfd,&h,sizeof(h));
				}
				beat_error(p->cfd ,ochead);
			}
			p = p->next;
		}
		pthread_mutex_unlock(&mutex);
	}
}






