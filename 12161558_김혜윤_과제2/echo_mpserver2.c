#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30	

//error msg 출력 후, 종료
void error_handling(char* message){
	fputs(message,stderr);
	fputc('\n',stderr);
	exit(1);
}

void read_childproc(int sig){
	pid_t pid;
	int status;
	pid = waitpid(-1, &status, WNOHANG); 
	printf("removed proc id : %d \n",pid);
}

int main(int argc, char *argv[]){
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	
	pid_t pid;
	struct sigaction act;
	socklen_t adr_sz;
	int str_len, state;
	char buf[BUF_SIZE];
	
	if(argc!=2){
		printf("Usage : %s <PORT>\n",argv[0]);
		exit(1);
	}
	
	act.sa_handler = read_childproc; //자식이 종료하면 read_childproc 호출
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	state = sigaction(SIGCHLD, &act, 0);
	
	serv_sock = socket(PF_INET,SOCK_STREAM,0);
	memset(&serv_adr,0,sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));
	
	if(bind(serv_sock,(struct sockaddr*)&serv_adr,sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock,5)==-1)
		error_handling("listen() error");
		
	while(1){
		adr_sz = sizeof(clnt_adr);
		clnt_sock = accept(serv_sock, (struct sockaddr*) &clnt_adr, &adr_sz);
		
		if(clnt_sock==-1)
			continue;
		else{
			printf("new client connected...\n");
			//clnt_adr의 sin_addr에 저장되어있는 ip주소는 네트워크 바이트 순서의 32비트 값이라 inet_ntoa로 Dotted-Decimal Notation의 주소값을 변환
			//clnt_adr의 sin_port에 저장되어있는 port 번호는 short형 데이터의 바이트 순서이므로 host로 변경 (htons())와 반대
			printf("connected client IP: %s port: %d\n",inet_ntoa(clnt_adr.sin_addr),ntohs(clnt_adr.sin_port)); 
		}
		
		pid = fork();//자식 프로세스 생성
		
		if(pid==-1){ //생성 실패
			close(clnt_sock);
			continue;
		}
		if(pid==0){ //자식 생성
			close(serv_sock);
			//client가 보낸 msg 받고
			while((str_len=read(clnt_sock,buf,BUF_SIZE))!=0)
				//그대로 다시 보내기
				write(clnt_sock,buf,str_len);
				
				close(clnt_sock);
				puts("client disconnected...");
				return 0;
		}
		else
			close(clnt_sock);
	}
	close(serv_sock);
	return 0;
}
		
		
			
		
		

