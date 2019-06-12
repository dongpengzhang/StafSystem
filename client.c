#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sqlite3.h>
#include <signal.h>
#include <time.h>


#define N 64
typedef struct
{
	int start; //登录状态
	char name[N]; //用户名
	char password[N]; //密码
	int no; //工号
	char work[N];//职位
	int age; //年龄
	int phone; //电话
	char date[N];//入职时间
	float salary;//工资
	char addr[N];//家庭住址

}staff_info;//员工信息结构体


typedef struct
{

	int type;//通信类型 
	int recvmsg;//通讯消息
	char data[2*N];//通讯数据
	char name[N];//用户名
	char password[N];//用户密码
	staff_info info;//员工信息
	char  history[N]; 

}MSG;//账号结构体


void do_admin_history(int sockfd,MSG *msg)//管理员查询历史函数
{
	msg->type = 10;//10为查询历史请求
	
	send(sockfd,msg,sizeof(MSG),0);
	while(1)
		{
			//接受服务器相应
			recv(sockfd,msg,sizeof(MSG),0);
			if(msg->recvmsg !=2){
				printf("%s\n",msg->data);
			}
			else{
				break;
			}
		}
		msg->recvmsg=0;
	
}

void do_admin_deluser(int sockfd,MSG *msg)//管理员删除函数
{
	msg->type = 9;//9为管理员删除请求

	printf("请输入要删除的用户工号：\n");
	scanf("%d",&msg->info.no);
	getchar();
	printf("请输入用户名：");
	scanf("%s",msg->info.name);
	getchar();

	//发送查询请求
	send(sockfd,msg,sizeof(MSG),0);
	//接受服务器相应
	recv(sockfd,msg,sizeof(MSG),0);
	printf("%s\n",msg->data);//成员删除成功
}

void do_admin_adduser(int sockfd,MSG *msg)//管理员添加函数
{
	char n ;
	msg->type = 8;//8为管理员添加请求
	printf("***************热烈欢迎新员工***************\n");
h:
	printf("请输入工号：\n");
	scanf("%d",&msg->info.no);
	getchar();
	printf("您输入的工号是：%d\n",msg->info.no);
	printf("工号信息一旦录入无法更改，请确认您所输入的是否正确！(Y/N)");
	scanf("%c",&n);
	getchar();
	if(n == 'N' || n == 'n')
	{
		goto h;
	}
	printf("请输入用户名：");
	scanf("%s",msg->info.name);
	getchar();
	printf("请输入用户密码：");
	scanf("%s",msg->info.password);
	getchar();
	printf("请输入年龄：");
	scanf("%d",&msg->info.age);
	getchar();
	printf("请输入电话：");
	scanf("%d",&msg->info.phone);
	getchar();
	printf("请输入家庭住址：");
	scanf("%s",msg->info.addr);
	getchar();
	printf("请输入职位：");
	scanf("%s",msg->info.work);
	getchar();
	
	printf("请输入职日期(不能输入空格)：");
	scanf("%s",msg->info.date);
	getchar();
	
	printf("请输入工资：");
	scanf("%f",&msg->info.salary);
	getchar();
i:
	printf("是否为管理员：(Y/N)");
	scanf("%c",&n);
	getchar();
	if(n == 'Y' || n == 'y')
	{
		msg->info.start = 1;
	}else if(n == 'N' || n == 'n')
	{
		msg->info.start = 2;
	}else
	{
		printf("输入错误\n");
		goto i;
	}
	
	//发送查询请求
	send(sockfd,msg,sizeof(MSG),0);
	//接受服务器相应
	recv(sockfd,msg,sizeof(MSG),0);
	printf("%s\n",msg->data);//成员添加成功，是否继续添加
	scanf("%c",&n);
	getchar();
	if(n == 'Y' || n == 'y')//如果是就跳转到头继续添加
	{
		goto h;
	}
}

void do_user_modification(int sockfd,MSG *msg)//普通用户修改函数
{
	int n;
	char data[N];//中转数组
	msg->type = 7;//7为普通成员修改请求
	while(1)
	{
	printf("请输入你要修改的工号\n");
	scanf("%d",&msg->info.no);
	getchar();
	printf("*******************请输入要修改的选项********************\n");
	printf("****** 1：家庭住址   2：电话   3：密码   4:退出    ******\n");
	printf("*********************************************************\n");
	printf("请输入您的选择（数字）>>");
	scanf("%d",&n);
	getchar();	
		switch(n)
		{
			case 1:
				printf("请输入家庭住址\n");
				scanf("%s",data);
				getchar();
				sprintf(msg->data,"家庭住址='%s'",data);
				break;
			case 2:
				printf("请输入电话\n");
				scanf("%s",data);
				getchar();
				sprintf(msg->data,"电话=%s",data);
				break;
			case 3:
				printf("请输入密码\n");
				scanf("%s",data);
				getchar();
				sprintf(msg->data,"密码='%s'",data);
				break;
			case 4:
				return;
				break;
			default:
				printf("您输入有误，请输入数字\n");
				break;
		}
	//发送查询请求
	send(sockfd,msg,sizeof(MSG),0);
	//接受服务器相应
	recv(sockfd,msg,sizeof(MSG),0);
	printf("%s\n",msg->data);
	}
	
}

void do_admin_modification(int sockfd,MSG *msg)//管理员修改函数
{
	int n;
	char data[N];//中转数组
	msg->type = 6;//6为管理员修改请求
	while(1)
	{
	printf("请输入你要修改的工号\n");
	scanf("%d",&msg->info.no);
	getchar();
	printf("*******************请输入要修改的选项********************\n");
	printf("******	1：姓名	  2：年龄	3：家庭住址   4：电话  ******\n");
	printf("******	5：职位	  6：工资   7：入职年月   8：密码  ******\n");
	printf("******	9：退出				                       ******\n");
	printf("*********************************************************\n");
	printf("请输入您的选择（数字）>>");
	scanf("%d",&n);
	getchar();	
		switch(n)
		{
		 	case 1:
				printf("请输入用户名\n");
				scanf("%s",data);
				getchar();
				sprintf(msg->data,"用户名='%s'",data);
				break;
		 	case 2:
				printf("请输入年龄\n");
				scanf("%s",data);
				getchar();
				sprintf(msg->data,"年龄=%s",data);
				break;
			case 3:
				printf("请输入家庭住址\n");
				scanf("%s",data);
				getchar();
				sprintf(msg->data,"家庭住址='%s'",data);
				break;
			case 4:
				printf("请输入电话\n");
				scanf("%s",data);
				getchar();
				sprintf(msg->data,"电话=%s",data);
				break;
			case 5:
				printf("请输入职位\n");
				scanf("%s",data);
				getchar();
				sprintf(msg->data,"职位='%s'",data);
				break;
			case 6:
				printf("请输入工资\n");
				scanf("%s",data);
				getchar();
				sprintf(msg->data,"工资=%s",data);
				break;
			case 7:
				printf("请输入入职日期\n");
				scanf("%s",data);
				getchar();
				sprintf(msg->data,"入职日期='%s'",data);
				break;
			case 8:
				printf("请输入密码\n");
				scanf("%s",data);
				getchar();
				sprintf(msg->data,"密码='%s'",data);
				break;
			case 9:
				return;
				break;
			default:
				printf("您输入有误，请输入数字\n");
				break;
		}
	//发送查询请求
	send(sockfd,msg,sizeof(MSG),0);
	//接受服务器相应
	recv(sockfd,msg,sizeof(MSG),0);
	printf("%s\n",msg->data);
	}
}

void do_admin_query_all(int sockfd,MSG *msg)//管理员查询所有函数
{
	msg->type = 4;//4为管理员查询所有请求
	
	//发送查询请求
		send(sockfd,msg,sizeof(MSG),0);
		while(1)
		{
			//接受服务器相应
			recv(sockfd,msg,sizeof(MSG),0);
			if(msg->recvmsg !=2){
				printf("%s\n",msg->data);
			}
			else{
				break;
			}
		}
		msg->recvmsg=0;	
}

void do_admin_query_name(int sockfd,MSG *msg)//管理员按人名查询函数
{
	msg->type = 3;//3为管理员按人名查询请求
	
	printf("请输入你要查询的人名:");
	scanf("%s",msg->name);
	getchar();

	//发送查询请求
		send(sockfd,msg,sizeof(MSG),0);
		while(1)
		{
			//接受服务器相应
			recv(sockfd,msg,sizeof(MSG),0);
			if(msg->recvmsg !=2){
				printf("%s\n",msg->data);
			}
			else{
				break;
			}
		}
		msg->recvmsg=0;	
	
}

void do_user_query(int sockfd,MSG *msg)//普通用户查询函数
{
	msg->type = 5;//5为普通用户查询请求

    //发送查询请求
	send(sockfd,msg,sizeof(MSG),0);
	while(1)
	{
		//接受服务器相应
		recv(sockfd,msg,sizeof(MSG),0);
		if(msg->recvmsg !=2){
			printf("%s\n",msg->data);
		}
		else{
			break;
		}
	}
	msg->recvmsg=0;

	
}

void do_admin_query(int sockfd,MSG *msg)//管理员查询函数
{
	int a;
	while(1)
	{
		printf("*************************************************************\n");
		printf("**********  1：按人名查询  	2：查找所有		3：退出	 ********\n");
		printf("*************************************************************\n");
		printf("请输入您的选择（数字）>>");
		scanf("%d",&a);
		getchar();

		switch(a)
		{
			case 1:
				do_admin_query_name(sockfd,msg);
				break;
			case 2:
				do_admin_query_all(sockfd,msg);
				break;
			case 3:
				return ;
				break;
			default:
				printf("您输入有误，请输入数字\n");
				break;
		}
	  	
	}
	
}

void admin_menu(int sockfd,MSG *msg)//管理员菜单
{
	int n;

	while(1)
	{
		printf("*************************************************************\n");
		printf("* 1：查询  2：修改 3：添加用户  4：删除用户  5：查询历史记录*\n");
		printf("* 6：退出													*\n");
		printf("*************************************************************\n");
		printf("请输入您的选择（数字）>>");
		scanf("%d",&n);
		getchar();

		switch(n)
		{
			case 1:
				do_admin_query(sockfd,msg);//管理员查询函数
				break;
			case 2:
				do_admin_modification(sockfd,msg);//管理员修改函数
				break;
			case 3:
				do_admin_adduser(sockfd,msg);//管理员添加函数
				break;
			case 4:
				do_admin_deluser(sockfd,msg);//管理员删除函数
				break;
			case 5:
				do_admin_history(sockfd,msg);//管理员查询历史函数
				break;
			case 6:
				//msg->msgtype = QUIT;
				//send(sockfd, msg, sizeof(MSG), 0);
				close(sockfd);
				exit(0);
			default:
				printf("您输入有误，请重新输入！\n");
		}
	}
}


void user_menu(int sockfd,MSG *msg)
{
	int n;
	while(1)
	{
		printf("*************************************************************\n");
		printf("*************  1：查询  	2：修改		3：退出	 *************\n");
		printf("*************************************************************\n");
		printf("请输入您的选择（数字）>>");
		scanf("%d",&n);
		getchar();

		switch(n)
		{
			case 1:
				do_user_query(sockfd,msg);//普通用户查询函数
				break;
			case 2:
				do_user_modification(sockfd,msg);//普通用户修改函数
				break;
			case 3:
				//msg->msgtype = QUIT;
				send(sockfd, msg, sizeof(MSG), 0);
				close(sockfd);
				exit(0);
			default:
				printf("您输入有误，请输入数字\n");
				break;
		}
	}

}
//普通用户菜单

//判断登录信息的函数
int admin_or_user_login(int sockfd, MSG *msg)
{
	memset(msg->name,0,N);//将type里n个字节替换成0
	printf("请输入用户名:");
	scanf("%s",msg->name);
	getchar();

	memset(msg->password,0,N);//将type里n个字节替换成0
	printf("请输入密码:");
	scanf("%s",msg->password);
	getchar();

	//发送登录请求
	send(sockfd,msg,sizeof(MSG),0);
	//接受服务器相应
	recv(sockfd,msg,sizeof(MSG),0);

	if(msg->recvmsg == 1)//通讯消息为1，说明查到
	{
		strcpy(msg->info.name , msg->name);//记录登录者
		if(msg->type == 1)//说明是管理员模式
		{
			printf("进入管理员模式\n");
			admin_menu(sockfd,msg);//进入管理员菜单
		}
		else if(msg->type == 2)//说明是普通用户模式
		{
			printf("进入普通用户模式\n");
			user_menu(sockfd,msg);//进入普通模式菜单
		}
	}
	else
	{
		printf("登录失败\n");
		return -1;
	}
	return 0 ;

}


int main(int argc, const char *argv[])
{
	int sockfd;

	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd < 0)
	{
		perror("sockfd fail");
		exit(-1);
	}
	printf("sockfd OK\n");
	//创建流式套接字
	
	if(argc < 3)
	{
		printf("请输入%s<IP><RORT>.\n",argv[0]);
		exit(-1);
	}//判断输入正确性

	struct sockaddr_in clientaddr;
	socklen_t addrlen = sizeof(clientaddr);

	clientaddr.sin_family = AF_INET;
	clientaddr.sin_port = htons(atoi(argv[2]));
	clientaddr.sin_addr.s_addr = inet_addr(argv[1]);
	//填充结构体
	
	if(connect(sockfd,(struct sockaddr*)&clientaddr,addrlen)<0)
	{
		perror("connet fail");
		exit(-1);
	}
	printf("connet OK\n");
	//建立链接
	
	int n = 0;
	char buf[N]={0};
	MSG msg;

	while(1)
	{
		printf("*******************************************************************\n");
		printf("********  1:管理员模式    2:普通用户模式    3:退出********\n");
		printf("*******************************************************************\n");
		printf("请输入您的选择（数字）>>");
		scanf("%d",&n);
		getchar();

		switch(n)
		{
			case 1:
				msg.type = 1;//1为管理员模式登录
				break;
			case 2:
				msg.type = 2;//2为普通用户模式登录
				break;
			case 3:
				close(sockfd);
				exit(0);
			default:
				printf("您的输入有误,请重新输入");
		}
//==================这里将各种模式加以区分============================
		
		admin_or_user_login(sockfd,&msg);//判断登录信息的函数

	}



	return 0;
}
