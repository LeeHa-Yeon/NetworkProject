#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<arpa/inet.h>

#define MAXLINE 1000
#define NAME_LEN 20

// 클라이언트의 종료요청 문자열
char *EXIT_STRING = "exit";

// 소켓 생성 및 서버 연결, 생성된 소켓 리턴
int tcp_connect(int af, char *servip, unsigned short port);

// 소켓함수 오류 출력 후 종료
void errquit(char *msg){
	perror(msg);
	exit(1);
}

// argc : 메인함수에 전달되는 정보의 개수
// argv : 메인함수에 전달되는 실질적인 정보, 문자열의 배열 , 첫번째 문자열은 프로그램의 실행경로로 항상 고정되어있음
int main(int argc, char *argv[]){

	char bufall[MAXLINE+NAME_LEN], *bufmsg; // 이름 , 메시지 부분
	int maxfdp1, s, namelen; // 최대 소켓 디스크립터, 이름의 길이
	fd_set read_fds; // 읽기를 감지할 fd_set 구조체

	// 실행파일 ip port name ->  총 4개 실행되어야함
	if(argc!= 4){
		printf("사용법: %s server_ip port name \n", argv[0]);
		exit(0);
	}

	// sprintf는 buffer라는 메모리 공간에 출력하는 출력함수
	sprintf(bufall,"사용자 '%s':",argv[3]);
	namelen = strlen(bufall);
	bufmsg = bufall + namelen;

	// atoi  문자열을 정수로 변환
	s = tcp_connect(AF_INET,argv[1],atoi(argv[2]));
	if(s == -1) {
		errquit("tcp_connect fail");
	}

	// MARK : 이거 없어도 됨
	puts("\n\t      <   BaskinRobbins project 2017301063 이하연 >\n\n      ");
	puts("\t                      서버에 접속 성공\n\n\n");
	
	

	maxfdp1 = s+1; // 최대 소켓번호 + 1
	// 소켓에 메시지를 받기 위해서 모든 비트를 삭제 해둠 , 초기화
	FD_ZERO(&read_fds);
	
	while(1){
		FD_SET(0,&read_fds); // 키보드 입력용 파일기술자(0) 세트
		FD_SET(s,&read_fds); // 서버와 연결된 소켓번호(s) 세트
		// 소켓 I/O 감지
		// 두번째인자(즉,읽기변화감지용), 세네번째인자(즉,쓰기및예외발생에 해당하는것들)
		if(select(maxfdp1, &read_fds, NULL, NULL, NULL)<0){
			errquit("select fail");
		}
	
		// 서버가 보내오는 메시지를 수신해서 출력
		if(FD_ISSET(s,&read_fds)){
			int nbyte;
			if((nbyte = recv(s,bufmsg,MAXLINE,0))>0){
				bufmsg[nbyte] = 0;
				// 메시지 출력 구간
				printf("\t\t\t 메세지  : %s\n",bufmsg);
				// 글자색을 노랑33으로 변경
				fprintf(stderr, "\033[33m");
			}
		}
		// 키보드 입력 데이터를 서버로 전송
		if(FD_ISSET(0,&read_fds)){
			if(fgets(bufmsg,MAXLINE, stdin)){
				if(send(s,bufall,namelen+strlen(bufmsg),0)<0){
					puts("Error: write error on socket");
				}
				if(strstr(bufmsg,EXIT_STRING)!=NULL){
					fprintf(stderr, "\033[32m");
					puts("GOOD BYE");
					close(s);
					exit(0);
				}
			}
		}
	}
}

int tcp_connect(int af, char *servip, unsigned short port){
	struct sockaddr_in servaddr; // 소켓 주소 구조체
	int s; // 함수 내에서 사용할 소켓

	// 소켓 생성
	if((s=socket(af,SOCK_STREAM,0))<0){
		return -1;
	}

	bzero((char*)&servaddr,sizeof(servaddr)); //채팅 서버의 소켓 주소 구조체 servaddr 초기화
	servaddr.sin_family = af; // af 값을 소켓 주소의 어드레스 패밀리값에 반영
	inet_pton(AF_INET,servip,&servaddr.sin_addr); // IPv4형식 주소를 바이너리 형태로 변경
	servaddr.sin_port = htons(port); // port값을 네트워크 byte order로 변경 후 소켓 주소의 포트값에 반영 
	
	// 연결 요청
	if(connect(s,(struct sockaddr *)&servaddr,sizeof(servaddr))<0){
		return -1;
	}
	return s;
}
