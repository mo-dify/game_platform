/* ************************************************************************
 *       Filename:  client_fun.c
 *    Description:  
 *        Version:  1.0
 *        Created:  2016年09月18日 15时25分34秒
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  YOUR NAME (), 
 *        Company:  
 * ************************************************************************/

#include"main.h"

pthread_mutex_t mutex;
/***************************************************************
 *
 *			函数功能： 产生随机数
 *	
 *			参数：无
 *
 *			返回值：j 随机数
 *
 * ************************************************************/
int randxy()
{
	int j;	
	j=1+(int)(14.0*rand()/(RAND_MAX+1.0));
	return j;
}
/***************************************************************
 *
 *			函数功能： 五子棋游戏进程，发送自己的棋子坐标，接收对手的棋子坐标，判断输赢
 *	
 *			参数：pm  包含玩家信息的结构体指针
 *
 *			返回值：无
 *
 * ************************************************************/
void game_wuziqi(PME pm)
{
	system("clear");
	int sfd = pm->sfd;
	int usfd = pm->usfd;
	char buf[100]="";
	char match_name[100]="";
	char match_id[100]="";
	int ret=1;
	HEAD h;
	BODY b;
	
	bzero(&b,0);
	h.type= 1005;
	write(sfd,&h,sizeof(h));
	printf("\33[7B\33[20C正在寻找对手，请稍候！！！！！！！\n");	
	read(sfd,&h,sizeof(h));
	printf("h.type====%d\n",h.type);
	//读取对手的信息
	read(sfd,&b,sizeof(b));
	//udp链接
	ret = connect(usfd,(const struct sockaddr*)&(b.olc.sin),(socklen_t)sizeof(b.olc.sin));
	strcpy(match_name,b.olc.name);
	strcpy(match_id,b.olc.id);
	socklen_t  len=sizeof(b.olc.sin);

	//	printf("h.type=======%d\n",h.type);
	system("clear");
	if(h.type==10050)//先手一方
	{
		int p=0;
		int c=0;
		printf("对手姓名：\33[32m %s\33[0m      对手id：\33[32m %s\33[0m\n",match_name,match_id);
		initqipan();
		//先手一方应先写 后读
		for(p=1;p<=N*N;p++)
		{
			//先获取 自己 棋子坐标
			printf("您是\33[33m黄旗\33[0m请您下棋！！\n\n");
//			while(ret==0)
			{
				weizhi[p].x[p]=zouqihang();			//输入棋子的横坐标
				weizhi[p].y[p]=zouqilie();			//输入棋子的纵坐标
				duplicate_check(p);		//判断新棋子是否已经存在，存在返回-1
			}
			b.chat.x = weizhi[p].x[p];
			b.chat.y = weizhi[p].y[p];
//			ret=0;
			save(p);//保存棋盘信息
			system("clear");
			printf("对手姓名：\33[32m %s\33[0m      对手id：\33[32m %s\33[0m\n",match_name,match_id);
			showqi(p);				//打印棋盘
			c=panduan(p);			//判断输赢玩家1赢返回1,玩家2赢返回2
			if((c==1))				//玩家1获胜
			{
				b.chat.flag=1;		//输赢状态量，1为赢
				strcpy(b.chat.id,pm->id);
				write(usfd,&b,sizeof(b));//告诉玩家2,玩家1获胜
				h.type=1009;
				write(sfd,&h,sizeof(h));//告诉服务器,玩家1获得一场比赛的胜利
				c=atoi(pm->score);
				c+=10;
				sprintf(pm->score,"%d",c);
				printf("恭喜，\33[32m您\33[0m获得了本剧比赛的胜利！！！！！获得\33[32m10\33[0m积分奖励\n");
				printf("按任意键返回！！！！！");
				getchar();
				return;
			}
			send(usfd,&b,sizeof(b),0);//发送棋子信息到玩家2	
			printf("请等待对手下棋！\n");
			//获取 对手 棋子坐标
			recvfrom(usfd,&b,sizeof(b),0,(struct sockaddr*)&(b.olc.sin),(socklen_t*)&len);//接受玩家2的棋子信息
			p+=1;
			weizhi[p].x[p]=b.chat.x;
			weizhi[p].y[p]=b.chat.y;
			save(p);
			system("clear");
			printf("对手姓名：\33[32m %s\33[0m      对手id：\33[32m %s\33[0m\n",match_name,match_id);
			showqi(p);
			c=panduan(p);
			if(c==2)
			{
				printf("很遗憾，\33[32m对手\33[0m获得了本剧比赛的胜利！！！！\n");
				printf("按任意键返回！！！！！");
				getchar();
				return;	
			}

		}
	}
	//后手一方应先读 后写
	if(h.type==10051)//后手一方
	{
		int p=0;
		int c=0;
		welcome();
		printf("对手姓名：\33[32m %s\33[0m      对手id：\33[32m %s\33[0m\n",match_name,match_id);
		initqipan();
		for(p=1;p<=N*N;p++)
		{
	
			//先获取 对手 棋子坐标
			printf("请稍等对手下棋！！\n");
			recvfrom(usfd,&b,sizeof(b),0,(struct sockaddr*)&(b.olc.sin),(socklen_t*)&len);
			weizhi[p].x[p]=b.chat.x;
			weizhi[p].y[p]=b.chat.y;
			save(p);
			system("clear");
			printf("对手姓名：\33[32m %s\33[0m      对手id：\33[32m %s\33[0m\n",match_name,match_id);
			showqi(p);
			c=panduan(p);
			if(c==1)
			{
				printf("很遗憾，\33[32m对手\33[0m获得了本剧比赛的胜利！！！\n");
				printf("按任意键返回！！！！！");
				getchar();
				return;	
			}
			p+=1;
			//后获取 自己 棋子坐标
			printf("您是\33[33m白旗\33[0m请您下棋！！\n\n");
//			while(ret==0)
			{
				weizhi[p].x[p]=zouqihang();
				weizhi[p].y[p]=zouqilie();
				duplicate_check(p);
			}
			b.chat.x = weizhi[p].x[p];
			b.chat.y = weizhi[p].y[p];
//			ret=0;
			save(p);
			system("clear");
			printf("对手姓名：\33[32m %s\33[0m      对手id：\33[32m %s\33[0m\n",match_name,match_id);
			showqi(p);
			c=panduan(p);
			if(c==2)
			{
				b.chat.flag=1;
				strcpy(b.chat.id,pm->id);
				write(usfd,&b,sizeof(b));
				h.type=1009;
				write(sfd,&h,sizeof(h));
				c=atoi(pm->score);
				c+=10;
				sprintf(pm->score,"%d",c);
				printf("恭喜您，\33[32m你\33[0m获得了本剧比赛的胜利！！！！！\n");
				printf("按任意键返回！！！！！");
				getchar();
				return;
			}
			send(usfd,&b,sizeof(b),0);
		}
	}
}


/***************************************************************
 *
 *			函数功能：斗地主游戏，还没做
 *	
 *			参数：	无
 *	
 *			返回值： 无
 *
 * ************************************************************/
void game_doudizhu()
{
	system("clear");
	printf("\33[7B\33[20C正在开发，敬请期待！\n");
	sleep(1);
	return;
}
/***************************************************************
 *
 *			函数功能： 选择游戏界面
 *	
 *			参数：	pm  包含玩家信息的结构体指针
 *
 *			返回值： 无
 *
 * ************************************************************/


int option_game(PME p)
{
	char ch;
	while(1)
	{
		system("clear");
		printf("\33[7B\33[20C***********************************************\n");
		printf("\33[20C*                                             *\n");
		printf("\33[20C*       玩家：\33[32m %s\33[0m      分数：\33[32m %s\33[0m            *\n",p->name,p->score);
		printf("\33[20C*                                             *\n");
		printf("\33[20C*            操作选项                         *\n");
		printf("\33[20C*                                             *\n");
		printf("\33[20C*            1、五子棋                        *\n");
		printf("\33[20C*                                             *\n");
		printf("\33[20C*            2、斗地主                        *\n");
		printf("\33[20C*                                             *\n");
		printf("\33[20C*            3、返回                          *\n");
		printf("\33[20C*                                             *\n");
		printf("\33[20C***********************************************\n");
		printf("\n\n\n请输入登录选项：\n");

		ch = getchar();
		while(getchar()!='\n');
		switch(ch)
		{
			case '1':
				game_wuziqi(p);
				getchar();
				break;
			case '2':
				game_doudizhu(p);
				return;
			case  '3':
				return;
		}
	}
	return;
}


/***************************************************************
 *
 *			函数功能：   发送世界聊天信息
 *	
 *			参数：		p 包含玩家信息的结构体指针
 *
 *			返回值：	0 继续聊天
 *						1 退出世界聊天
 *
 * ************************************************************/

int send_fun(PME p)
{
	int sfd = p->sfd;
	char buf[1024]="";
	int ret;
	HEAD h;
	BODY b;
	memset(&h,0,sizeof(h));
	memset(&b,0,sizeof(b));
	ret = read(0,buf,1024);
	if(!strncmp(buf,"-r",2))
		return 1;
	strncpy(b.chat.say,buf,ret);
	strcpy(b.chat.name,p->name);
	h.type=1004;
	h.length = sizeof(b.chat.name)+sizeof(b.chat.say);
	write(sfd,&h,sizeof(h));
	write(sfd,&b,h.length);
	return 0;
}



/***************************************************************
 *
 *			函数功能：接收世界聊天信息
 *	
 *			参数： p 包含玩家信息的结构体指针
 *
 *			返回值：	无
 *
 * ************************************************************/

void recv_fun(PME p)
{
	int sfd = p->sfd;
	char buf[1024]="";
	int ret;
	HEAD h;
	BODY b;

	memset(&h,0,sizeof(h));
	memset(&b,0,sizeof(b));
	ret = read(sfd,&h,4);
	if(ret <=0){
		perror("read sfd");
		return;
	}
	//	printf("h.type=%d\n",h.type);
	//	printf("h.length=%d\n",h.length);
	if(h.type==1004 &&h.length>0){
		ret = read(sfd,&b,sizeof(b));
		if(ret <=0){
			perror("read sfd");
			return;
		}
		printf("\r\33[32m%s\33[0m : %s",b.chat.name,b.chat.say);
		fflush(stdout);
	}
	
	return ;
}



/***************************************************************
 *
 *			函数功能：	世界聊天，用epoll函数来做
 *	
 *			参数：	p 包含玩家信息的结构体指针
 *
 *			返回值： 无
 *
 * ************************************************************/



int chat_all(PME p)
{
	int sfd;
	int ret;	
	int efd, i, nfound; 
	char buf[1024]="";
	struct epoll_event ev, evs[2];

	efd = epoll_create(10);
	ev.events = EPOLLET|EPOLLIN;
	ev.data.fd = p->sfd;
	epoll_ctl(efd,EPOLL_CTL_ADD,p->sfd,&ev);

	ev.events = EPOLLET|EPOLLIN;
	ev.data.fd = 0;
	epoll_ctl(efd,EPOLL_CTL_ADD,0,&ev);
	system("clear");
	while(1){
	
		printf("\33[33m%s\33[0m: ",p->name);
		fflush(stdout);
		nfound = epoll_wait(efd, evs, 2, -1);
		for(i = 0; i<nfound; i++)
		{
			if(evs[i].data.fd == 0)
			{
				ret =send_fun(p);
				if(ret==1)
					return;
			}
			else		
				recv_fun(p);
			
		}
	}


}


/***************************************************************
 *
 *			函数功能：	查找玩家
 *	
 *			参数：	p 包含玩家信息的结构体指针
 *
 *			返回值： 无
 *
 * ************************************************************/
void search_client(PME p)
{
	system("clear");
	int sfd = p->sfd;
	char buf[1024]="";
	int ret;
	HEAD h;
	BODY b;
	memset(&h,0,sizeof(h));
	memset(&b,0,sizeof(b));
	printf("请输入要查找的玩家id:");
	scanf("%s",b.chat.id);
	getchar();
//	printf("%s\n",b.chat.id);
	h.type=1006;
	h.length = sizeof(b.chat.id);
	write(sfd,&h,sizeof(h));
	write(sfd,&b,h.length);
	read(sfd,&h,4);

	if(h.type==1006)
	{
		read(sfd,&b,sizeof(b));
		printf("\n\33[33m查询结果\33[0m：id：\33[32m%s\33[0m	姓名：\33[32m%s\33[0m	分数:\33[32m%s\33[0m\n",b.chat.id,b.chat.name,b.chat.say);
		printf("\n输入任意键退出！\n");
		getchar();
	}
	else{
		printf("查无此人！！！！！！\n");
		printf("\n输入任意键退出！\n");
		getchar();
	}
	return ;
}

/***************************************************************
 *
 *			函数功能：	查看在线玩家
 *	
 *			参数：	p 包含玩家信息的结构体指针
 *
 *			返回值： 无
 *
 * ************************************************************/

#if 1
void get_client_online(PME p)
{
	int sfd = p->sfd;
	int ret;
	HEAD h;
	BODY b;
	system("clear");
	memset(&h,0,sizeof(h));
	memset(&b,0,sizeof(b));
	h.type=1007;
	write(sfd,&h,sizeof(h));
	printf("\33[33m在线玩家列表\33[0m：\n\n");
	while(1)
	{
		read(sfd,&h,4);
		if(h.type==10070)
			break;
		read(sfd,&b,h.length);
		printf("id：\33[32m%s\33[0m	姓名：\33[32m%s\33[0m\n",b.chat.id,b.chat.name);
	}
	printf("\n输入任意键退出！\n");
	getchar();

	return ;
}

#endif


/***************************************************************
 *
 *			函数功能：  二级菜单界面
 *	
 *			参数：		p 包含玩家信息的结构体指针
 *
 *			返回值：    无
 *
 * ************************************************************/


int  select_option(PME p)
{

	char ch;
	while(1)
	{
		system("clear");
		printf("\33[7B\33[20C***********************************************\n");
		printf("\33[20C*                                             *\n");
		printf("\33[20C*       玩家：\33[32m %s\33[0m      分数：\33[32m %s\33[0m            *\n",p->name,p->score);
		printf("\33[20C*                                             *\n");
		printf("\33[20C*            操作选项                         *\n");
		printf("\33[20C*                                             *\n");
		printf("\33[20C*            1、游戏                          *\n");
		printf("\33[20C*                                             *\n");
		printf("\33[20C*            2、世界聊天                      *\n");
		printf("\33[20C*                                             *\n");
		printf("\33[20C*            3、查找玩家                      *\n");
		printf("\33[20C*                                             *\n");
		printf("\33[20C*            4、在线玩家                      *\n");
		printf("\33[20C*                                             *\n");
		printf("\33[20C*            5、注销                          *\n");
		printf("\33[20C*                                             *\n");
		printf("\33[20C***********************************************\n");
		printf("\n\n\n请输入登录选项：\n");

		ch = getchar();
		while(getchar()!='\n');
		switch(ch)
		{
			case '1':
				option_game(p);
				break;
			case '2':
				chat_all(p);
				break;
			case  '3':
				search_client(p);
				break;
			case  '4':
				get_client_online(p);
				break;
			case  '5':
				exit(0);
		}
	}
	return;

}


/***************************************************************
 *
 *			函数功能：   登录验证，并保存登录成功后玩家信息
 *	
 *			参数：		 p 保存玩家信息的结构体指针
 *
 *			返回值：     无
 *
 * ************************************************************/



void client_enter(PME p)
{
	int sfd = p->sfd;
	char id[100]="";
	char pwd[100]="";
	char buf[100]="";
	int ret;
	HEAD h;
	BODY b;
	//输入登录账户密码
	while(1)
	{	
		system("clear");
		printf("\33[7B\33[20C请输入帐号密码:\n");
		printf("\n\n\n");

		printf("\33[20C帐号:  ");
		scanf("%s",id);
		printf("\n\n\n");

		printf("\33[20C密码:  ");
		scanf("%s",pwd);
		getchar();
		//发送登录信息
		strcpy(b.chat.id,id);
		strcpy(b.chat.say,pwd);
		h.type=1002;
		h.length = sizeof(b.chat.id)+sizeof(b.chat.say);
		write(sfd,&h,sizeof(h));
		write(sfd,&b,h.length);
		//读取服务器判断
		read(sfd,&h,4);

		//成功
		if(h.type==10020)
		{
			system("clear");
			printf("登录成功！\n");
			read(sfd,&b,sizeof(b));
			
			strcpy(p->id,b.chat.id);
			printf("p->id==%s\n",p->id);
			strcpy(p->name,b.chat.name);
			strcpy(p->score,b.chat.say);
			//udp 绑定
			ret = bind(p->usfd,(const struct sockaddr*)&(b.olc.sin),(socklen_t)sizeof(b.olc.sin));
			select_option(p);

			break;
		}
		//失败
		system("clear");
		printf("帐号密码不正确！\n");
		sleep(1);	
	}
	return;
}
/***************************************************************
 *
 *			函数功能：  注册新用户
 *	
 *			参数：		p 保存玩家信息的结构体指针
 *
 *			返回值：	无
 *
 * ************************************************************/


void client_enroll(PME p)
{

	int sfd = p->sfd;
	char id[100]="";
	char pwd[100]="";
	char buf[100]="";
	char name[100]="";
	int ret;
	HEAD h;
	BODY b;
	//输入登录账户密码
	while(1)
	{	
		system("clear");
		printf("\33[7B\33[20C请输注册信息:\n");
		printf("\n\n\n");
		printf("\33[20C帐号:    ");
		scanf("%s",id);
		printf("\n\n\n");
		printf("\33[20C密码:    ");
		scanf("%s",pwd);
		printf("\n\n\n");	
		printf("\33[20C姓名：   ");
		scanf("%s",name);
		getchar();

		strcpy(b.chat.id,id);
		strcpy(b.chat.say,pwd);
		strcpy(b.chat.name,name);

		h.type=1001;
		h.length = sizeof(b.chat.id)+sizeof(b.chat.say)+sizeof(b.chat.name);

		write(sfd,&h,sizeof(h));
		write(sfd,&b,h.length);

		ret = read(sfd,&h,4);

		if(h.type==10010){
			printf("注册成功,请重新登录！\n");
			sleep(1);
			client_enter(p);
			break;
		}
		if(h.type==10011){
			printf("帐号已存在，请重新输入！\n");
			sleep(1);
			client_enroll(p);
			return;
		}
	}
}

/***************************************************************
 *
 *			函数功能：	登录、注册、退出界面
 *	
 *			参数：		p 保存玩家信息的结构体指针
 *
 *			返回值：	无
 *
 * ************************************************************/

int  enter(PME p)
{

	char ch;
	while(1)
	{
		system("clear");
		printf("\33[7B\33[20C***********************************************\n");
		printf("\33[20C*                                             *\n");
		printf("\33[20C*          游戏对战平台                       *\n");
		printf("\33[20C*                                             *\n");
		printf("\33[20C*            1、登录                          *\n");
		printf("\33[20C*                                             *\n");
		printf("\33[20C*            2、注册                          *\n");
		printf("\33[20C*                                             *\n");
		printf("\33[20C*            3、退出                          *\n");
		printf("\33[20C*                                             *\n");
		printf("\33[20C***********************************************\n");
		printf("\n\n\n请输入登录选项：\n");
		ch = getchar();
		while(getchar()!='\n');
		switch(ch)
		{
			case '1':
				client_enter(p);
				break;
			case '2':
				client_enroll(p);
				break;
			case  '3':
				exit(0);
		}
	}
	return;

}

/***************************************************************
 *
 *			函数功能：	心跳包
 *	
 *			参数：		p 保存玩家信息的结构体指针
 *
 *			返回值：	无
 *
 * ************************************************************/
#if 1
void * heart_beat(void* pp)
{
	PME p=(PME)pp;
	int sfd = p->sfd;
	int ret;
	HEAD h;
	BODY b;
	memset(&h,0,sizeof(h));
	while(1)
	{
		sleep(5);
		h.type=1008;
		write(sfd,&h,sizeof(h));
	}
	return ;
}
#endif


