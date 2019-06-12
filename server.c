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


sqlite3 *db; //创建庫指针

#define N 64

char *errmsg = NULL; //创建出错庫指针

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

	int type;//账号类型 
	int recvmsg;//通讯信号
	char data[2*N];//通讯数据
	char name[N];//用户名
	char password[N];//用户密码
	staff_info info;//员工信息
	char history[N];//历史信息

}MSG;//账号结构体

void do_addr_history(int acceptfd,MSG msg)//向历史数据库插入数据
{
	int found;//承接do_searchword返回值
	time_t te;//承接秒数
	struct tm* tm_p;//承接时间
	char sendtm[N] = {0};//作为sqlite3的中间数组
	char work[N] = {0};//作为寻找失败的返回数组


	time(&te);//获得秒数
	tm_p = localtime(&te);
	//获得历史时间
	sprintf(sendtm,"insert into history values('%s','%d-%d-%d %d:%d:%d','%s');",msg.info.name,tm_p->tm_year+1900,tm_p->tm_mon+1,tm_p->tm_mday,tm_p->tm_hour,tm_p->tm_min,tm_p->tm_sec,msg.history);
	if(sqlite3_exec(db,sendtm,NULL,NULL,&errmsg) != 0)
	{
		fprintf(stderr,"历史插入失败:%s\n",errmsg);
		return;
	}
	//将历史插入数据库
	
}


void do_admin_history( int acceptfd,MSG msg)//管理员查询历史函数
{
	char select[N] = {0} ;
	//创建查找中专数组
	char **resultp = NULL;
	int nrow;
	int ncolumn;
	int i ;

	sprintf(select,"select * from history;");
    //查询用户是否存在
    sqlite3_get_table(db,select,&resultp,&nrow,&ncolumn,&errmsg);

	for(i=0; i<=nrow ; i++){
		
		sprintf(select,"%s  %s  %s",resultp[i*ncolumn+0],resultp[i*ncolumn+1],resultp[i*ncolumn+2]);
		strcpy(msg.data,select);//将找到的信息
		printf("resultp = %s\n",select);
		send(acceptfd,&msg,sizeof(msg),0);//传回客户端
		usleep(1000);
	}
	msg.recvmsg = 2;//recvmsg设置为2 作为传递结束标志
	send(acceptfd,&msg,sizeof(msg),0);//传回客户端
	
}

void to_admin_deluser(int acceptfd,MSG msg)//管理员删除函数
{
	char mod[sizeof(msg)] = {0};//数据库修改中间数组
	sprintf(mod,"delete from user where 用户名='%s' and 工号=%d;",msg.info.name,msg.info.no);
	printf("mod = %s\n",mod);

	if(sqlite3_exec(db,mod,NULL,NULL,&errmsg)!=0)
	{
		fprintf(stderr,"sqlite3_exec failed :%s\n",sqlite3_errmsg(db));
		strcpy(msg.data,"成员删除失败\n");
		send(acceptfd,&msg,sizeof(msg),0);//传回客户端
		
		sprintf(msg.data,"%s成员工号为：%d的成员\n",msg.info.name,msg.info.no);
		do_addr_history(acceptfd,msg);//将删除信息存入历史
		
		return ;
	}else{
		sprintf(msg.history,"成员删除成功，删除成员工号为：%d\n",msg.info.no);
		send(acceptfd,&msg,sizeof(msg),0);//传回客户端
	}
}

void to_admin_adduser(int acceptfd,MSG msg)//管理员添加成员函数
{
	char mod[sizeof(msg)] = {0};//数据库修改中间数组
	sprintf(mod,"insert into user values('%d','%s','%s',%d,'%s',%d,%d,'%s',%f,'%s');",msg.info.start,msg.info.name,msg.info.password,msg.info.no,msg.info.work,msg.info.age,msg.info.phone,msg.info.date,msg.info.salary,msg.info.addr);
	printf("mod = %s\n",mod);
	if(sqlite3_exec(db,mod,NULL,NULL,&errmsg)!=0)
	{
		fprintf(stderr,"sqlite3_exec failed :%s\n",sqlite3_errmsg(db));
		strcpy(msg.data,"成员添加失败，是否从新添加员工:(Y/N)");
		send(acceptfd,&msg,sizeof(msg),0);//传回客户端		
		return ;
	}else{
		strcpy(msg.data,"成员添加成功，是否继续添加员工:(Y/N)");
		send(acceptfd,&msg,sizeof(msg),0);//传回客户端
		
		sprintf(msg.history,"添加新成员%s\n",msg.info.name);
		do_addr_history(acceptfd,msg);//将添加信息存入历史
	}
}

void to_user_modification(int acceptfd,MSG msg)//普通成员修改函数
{
	char mod[sizeof(msg)] = {0};//数据库修改中间数组
	char no[N] = {0};
	sprintf(no,"工号=%d",msg.info.no);//将工号转化
	sprintf(mod,"update user set %s where %s;",msg.data,no);
	printf("mod = %s\n",mod);
	if(sqlite3_exec(db,mod,NULL,NULL,&errmsg)!=0)
	{
		fprintf(stderr,"sqlite3_exec failed :%s\n",sqlite3_errmsg(db));
		return ;
	}else{
		sprintf(msg.history,"%s修改工号为%d的成员%s\n",msg.info.name,msg.info.no,msg.data);
		do_addr_history(acceptfd,msg);//将修改信息存入历史
	
		strcpy(msg.data,"数据库修改成功");
		send(acceptfd,&msg,sizeof(msg),0);//传回客户端
		
	}
}

void to_admin_modification(int acceptfd,MSG msg)//管理员修改函数
{
	char mod[sizeof(msg)] = {0};//数据库修改中间数组
	char no[N] = {0};
	sprintf(no,"工号=%d",msg.info.no);//将工号转化
	sprintf(mod,"update user set %s where %s;",msg.data,no);
	printf("mod = %s\n",mod);
	if(sqlite3_exec(db,mod,NULL,NULL,&errmsg)!=0)
	{
		fprintf(stderr,"sqlite3_exec failed :%s\n",sqlite3_errmsg(db));
		return ;
	}else{
		sprintf(msg.history,"%s修改工号为%d的成员%s\n",msg.info.name,msg.info.no,msg.data);
		do_addr_history(acceptfd,msg);//将修改信息存入历史
	
		strcpy(msg.data,"数据库修改成功");
		send(acceptfd,&msg,sizeof(msg),0);//传回客户端
		
	}
}

void to_user_query(int acceptfd,MSG msg)//普通用户查询函数
{
	char select[sizeof(msg)] = {0} ;
	//创建查找中专数组
	char **resultp = NULL;
	int nrow;
	int ncolumn;
	int i ;
	printf("%d %s %s %d\n",msg.type,msg.name,msg.password,msg.recvmsg);

	sprintf(select,"select * from user where 用户名='%s' and 密码='%s';",msg.name,msg.password);
    //查询用户是否存在
    sqlite3_get_table(db,select,&resultp,&nrow,&ncolumn,&errmsg);

	

	for(i=0; i<=nrow ; i++){
		sprintf(select,"%s %s %s %s %s %s %s %s %s %s",resultp[i*10+0],resultp[i*10+1],resultp[i*10+2],resultp[i*10+3],resultp[i*10+4],resultp[i*10+5],resultp[i*10+6],resultp[i*10+7],resultp[i*10+8],resultp[i*10+9]);
		strcpy(msg.data,select);//将找到的信息
		printf("resultp = %s\n",select);
		send(acceptfd,&msg,sizeof(msg),0);//传回客户端
		usleep(1000);
	}
	msg.recvmsg = 2;//recvmsg设置为2 作为传递结束标志
	send(acceptfd,&msg,sizeof(msg),0);//传回客户端

	sprintf(msg.history,"%s成员查询了自己\n",msg.name);
	do_addr_history(acceptfd,msg);//将修改信息存入历史
}

void to_admin_query_all(int acceptfd,MSG msg)//管理员全部查询函数
{
	char select[sizeof(msg)] = {0} ;
	//创建查找中专数组
	char **resultp = NULL;
	int nrow;
	int ncolumn;
	int i ;
	printf("%d %s %s %d\n",msg.type,msg.name,msg.password,msg.recvmsg);

	sprintf(select,"select * from user");
    //查询用户是否存在
    sqlite3_get_table(db,select,&resultp,&nrow,&ncolumn,&errmsg);
	
	for(i=0; i<=nrow ; i++){
		sprintf(select,"%s %s %s %s %s %s %s %s %s %s",resultp[i*10+0],resultp[i*10+1],resultp[i*10+2],resultp[i*10+3],resultp[i*10+4],resultp[i*10+5],resultp[i*10+6],resultp[i*10+7],resultp[i*10+8],resultp[i*10+9]);
		strcpy(msg.data,select);//将找到的信息
		printf("resultp = %s\n",msg.data);
		send(acceptfd,&msg,sizeof(MSG),0);//传回客户端
		usleep(1000);
	}
	msg.recvmsg = 2;//recvmsg设置为2 作为传递结束标志
	send(acceptfd,&msg,sizeof(msg),0);//传回客户端	

	sprintf(msg.history,"%s成员查询全部成员\n",msg.info.name);
	do_addr_history(acceptfd,msg);//将修改信息存入历史
	
}

void to_admin_query_name(int acceptfd,MSG msg)//管理员按人名查询函数
{
	char select[sizeof(msg)] = {0};
	//创建查找中专数组
	char **resultp = NULL;
	int nrow;
	int ncolumn;
	int i ;
	printf("%d %s %s %d\n",msg.type,msg.name,msg.password,msg.recvmsg);

	sprintf(select,"select * from user where 用户名='%s';",msg.name);
		//查询用户是否存在
	sqlite3_get_table(db,select,&resultp,&nrow,&ncolumn,&errmsg);

	
	for(i=0; i<=nrow ; i++){
		sprintf(select,"%s %s %s %s %s %s %s %s %s %s",resultp[i*10+0],resultp[i*10+1],resultp[i*10+2],resultp[i*10+3],resultp[i*10+4],resultp[i*10+5],resultp[i*10+6],resultp[i*10+7],resultp[i*10+8],resultp[i*10+9]);
		strcpy(msg.data,select);//将找到的信息
		printf("resultp = %s\n",select);
		send(acceptfd,&msg,sizeof(msg),0);//传回客户端
		usleep(1000);
	}

	
	msg.recvmsg = 2;//recvmsg设置为2 作为传递结束标志
	send(acceptfd,&msg,sizeof(msg),0);//传回客户端

	sprintf(msg.history,"%s成员查询了%s\n",msg.info.name,msg.name);
	do_addr_history(acceptfd,msg);//将修改信息存入历史
	
}

void to_login_request(int acceptfd, MSG msg)//登录请求
{
	char select[sizeof(msg)] = {0} ;
	//创建查找中专数组
	char **resultp = NULL;
	int nrow;
	int ncolumn;
	printf("%d %s %s %d",msg.type,msg.name,msg.password,msg.recvmsg);
	

	sprintf(select,"select * from user where 登录状态='%d'and 用户名='%s' and 密码='%s';",msg.type,msg.name,msg.password);
    //查询用户是否存在
    sqlite3_get_table(db,select,&resultp,&nrow,&ncolumn,&errmsg);
	//获得查询信息

	if(nrow == 1)
	{
	//如果找到了就返回1
	    printf("nrow = %d",nrow);
		printf("login OK\n");
		msg.recvmsg = 1;
		send(acceptfd,&msg,sizeof(MSG),0);
		return;
	}
	else
	{
	//如果没有找到就返回0
		printf("nrow = %d",nrow);
		fprintf(stderr,"没有找到用户:%s\n",errmsg);
		msg.recvmsg = 0;
		send(acceptfd,&msg,sizeof(MSG),0);
		return;
	}
	
}

int main(int argc, const char *argv[])
{
	if(sqlite3_open("Staf.db",&db) != 0)//打开数据庫
	{
		fprintf(stderr,"sqlite3_open_fail :%s\n",sqlite3_errmsg(db));
		return -1;
	}

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

	struct sockaddr_in serveraddr;
	socklen_t addrlen = sizeof(serveraddr);

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(atoi(argv[2]));
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
	//填充结构体
	if(bind(sockfd,(struct sockaddr *)&serveraddr,addrlen)<0)
	{
		perror("bind fail");
		exit(-1);
	}
	printf("bind OK\n");
	//建立链接bind
	if(listen(sockfd,5) < 0)
	{
		perror("listen fail");
		exit(-1);
	}
	printf("listen OK\n");
	//监听
	int acceptfd;
	int recvbytes;
	MSG msg;
	while(1)
	{
		acceptfd = accept(sockfd,NULL,NULL);//接受链接请求
		if(acceptfd < 0)
		{
			perror("accept fail");
			exit(-1);
		}
		printf("accept OK\n");

		if(fork() == 0)//创建线程
		{
			close(sockfd);//在子线程中关闭连接套接字
			while(recvbytes = recv(acceptfd,&msg,sizeof(MSG),0)> 0){
				
				switch(msg.type)
				{
					case 1:
					case 2://1和2为登录请求
						to_login_request(acceptfd,msg);
						break;
					case 3://3为管理员按人名查询请求
						to_admin_query_name(acceptfd,msg);
						break;
					case 4://4为管理员全部查询请求
						to_admin_query_all(acceptfd,msg);
						break;
					case 5://5为普通用户查询请求
						to_user_query(acceptfd,msg);
						break;
					case 6://6为管理员修改请求
						to_admin_modification(acceptfd,msg);
						break;
					case 7://7为普通成员修改请求
						to_user_modification(acceptfd,msg);
						break;
					case 8://8为管理员添加成员请求
						to_admin_adduser(acceptfd,msg);
						break;
					case 9://9为管理员删除请求
						to_admin_deluser(acceptfd,msg);
						break;
					case 10://10为管理员查询历史请求
						do_admin_history(acceptfd,msg);
						break;
				}
			}
		}
	}
	return 0;
}

