#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<sys/file.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<errno.h>
#include<signal.h>
#define MAXLINE 511
#define MAX_SOCK 1024


char *TEST = " \n \t\t\t : ";

char *EXIT_STRING = "exit";
char *CONNECT_STRING = "Hi , Connected to the game.\n";
char *START_STRING = "Let's start the game \n\n\t------------------------------------------------------\n";

char *INPUT_STRING= "\t\t\t Please enter a number \n↓----   ------------------------------------------------------ ";
char *WAIT_STRING = "\t\t\t\t  Wait \n\t------------------------------------------------------\n";


char *END = "\n 게임이 끝이 났습니다.  \n";
char *WIN = "게임에서 승리하셨습니다.\n";
char *LOSE = "게임에서 졌습니다. \n";
char *WRONG_TURN_STRING = " *WARNING※ \n\t\t\t :  It's not your turn \n\t------------------------------------------------------\n \n";

int maxfdp1;
int num_chat = 0;
int clisock_list[MAX_SOCK];
int listen_sock;

int is_GameStart = 0;
int turn = 0;
int trun = 1;
int total = 0;
char input_error[30];
char test[30];
void GameRule();
void game_play(int i);

void addClient(int s, struct sockaddr_in *newcliaddr);
void removeClient(int s);
int set_nonblock(int sockfd);
int is_nonblock(int sockfd);
int tcp_listen(int host, int port, int backlog);
void errquit(char *mesg) {
        perror(mesg);
        exit(1);
}

int main(int argc, char *argv[]) {
	char *player;
	char recv_buf[MAXLINE];
        struct sockaddr_in cliaddr;
        char buf[MAXLINE];
        int i,j,nbyte,k;
        int accp_sock, clilen, addrlen;
	char user_name[MAXLINE] = "tnddls";
	char user_name1[MAXLINE] = "gkdus";

        if(argc != 2) {
                printf("사용법 : %s port\n", argv[0]);
                exit(0);
        }
        listen_sock = tcp_listen(INADDR_ANY, atoi(argv[1]), 5);

        if(listen_sock == -1) {
                errquit("tcp_listen fail");
        }
        if(set_nonblock(listen_sock) == -1) {
                errquit("set_nonblock fail");
        }

        GameRule();


        while(1) {
                addrlen = sizeof(cliaddr);
                accp_sock = accept(listen_sock, (struct sockaddr *)&cliaddr,&addrlen);
                if(accp_sock == -1 && errno != EWOULDBLOCK) {
                        errquit("accept fail");
                }
                else if(accp_sock > 0) {
                        clisock_list[num_chat] = accp_sock;
                        if(is_nonblock(accp_sock) != 0 &&set_nonblock(accp_sock) < 0) {
                                errquit("set_nonblock fail");
                        }
			// 서버에 새로운 참가자 등장을 알림
			//nbyte = recv(clisock_list[num_chat],recv_buf,MAXLINE,0);
        		//player = strtok(recv_buf,":");
			//printf("aa : %s\n ",recv_buf);
        	        //printf("%d , %d,  %d, %d \n",nbyte ,strlen(recv_buf),clisock_list[i],i);
                        addClient(accp_sock, &cliaddr);
			if(num_chat == 1){
			printf("%s가 입장하였습니다..\n", user_name);
			}
			if(num_chat == 2){
				printf("%s가 입장하였습니다..\n", user_name1);
			}
			// 클라이언트에 메세지를 전달해줌
                        send(accp_sock, CONNECT_STRING, strlen(CONNECT_STRING), 0);

                        if(num_chat == 2) {
                                //for(k = 0; k < num_chat; k++) {
                                send(clisock_list[0], START_STRING,strlen(START_STRING), 0);
				send(clisock_list[1],TEST,strlen(TEST),0);
				send(clisock_list[1], START_STRING,strlen(START_STRING), 0);                                
//}
                                send(clisock_list[0], INPUT_STRING, strlen(INPUT_STRING), 0);
                                send(clisock_list[1], WAIT_STRING, strlen(WAIT_STRING), 0);
                                is_GameStart = 1;
                                printf("\n\t\t    게임 시작할 준비가 완료되었습니다.\n");
                        }
                }
                for(i = 0; i<num_chat; i++) {
                        if(is_GameStart == 1)
                                game_play(i);
                }
        }
}

void GameRule(){
puts("");
fprintf(stderr, "\033[32m");
puts(" ----------------------------------------------------------------------------");
puts("⎢\t\t\t < BaskinRobbins Thirty One Game Rule >              ⎜\n⎜\t\t\t\t\t\t\t\t\t     ⎜");
puts("⎢\t 1. 1부터 시작해서 숫자를 연속으로 부른다                            ⎜");
puts("⎢\t 2. 건너뛰지 않고 숫자를 3개까지 부를 수 있다                        ⎜");
puts("⎢\t 3. 상대방이 마지막 부른 숫자에 이어서 31을 부르는 사람이 진다       ⎜");
puts(" ----------------------------------------------------------------------------");
fprintf(stderr, "\033[97m");
}

void game_play(int i){ 
	char* tmp2 = NULL;
	char aa[150] = " ";
	int df[MAXLINE];
        int k, j, nbyte, num,f;
        int present;
	char numBuf[150];
        char inputBuf[150];
        char resultBuf[MAXLINE];
        char recv_buf[MAXLINE];
        char *player, *next_token;


		

        nbyte = recv(clisock_list[i], recv_buf, MAXLINE, 0);
        if(nbyte == 0) {
                removeClient(i);  return;
        }
        else if(nbyte == -1 && errno == EWOULDBLOCK) {
                return;
        }
        if(strstr(recv_buf, EXIT_STRING) != NULL) {
                removeClient(i);
                return;
        }
        recv_buf[nbyte] = 0;
        player = strtok(recv_buf,":");
        next_token = strtok(NULL, " ");
        num = atoi(next_token);

	if(turn == i){
//		printf("\n  aa : %s\n ",recv_buf);
//		printf("%d , %d,  %d, %d \n",nbyte ,strlen(recv_buf),clisock_list[i],i);
                if(num > 0 && num < 4) {
			fprintf(stderr, "\033[33m");
                        printf("\n\t ------------------------------------------------------------ \n");
                        printf("\n\t\t\t사용자%s 숫자 :  ",player);
                        for( present=total+1; present<=total+num; present++){
                                printf(" %d", present);
                                if(present!=total+num)
                                        printf(",");
                        }
                        puts("");
                        total += num;
                        printf("\t\t\t   ✓ 현재 마지막 숫자 ► %d \n", total);
                        if(total >= 31) {
                                for(k = 0 ; k<2; k++){
					fprintf(stderr, "\033[97m");
                                        if(i==k)
                                                send(clisock_list[k],LOSE,strlen(LOSE), 0);
                                        else
                                                send(clisock_list[k],WIN,strlen(WIN), 0);
                                }
				fprintf(stderr, "\033[97m");
				printf("\t ------------------------------------------------------------ \n");
                                printf("\t\t\t 결과 : %s가 졌습니다.\n",player);
                                is_GameStart = 0;
                        }
                        else{
                                
				//sprintf(inputBuf, "%s ▶︎ 숫자 %d개를 선택. \n\t\t\t :  현재 숫자[%d]\n", player, num, total);
				
			//	sprintf(inputBuf, "%s ▶︎ 숫자 %d개를 선택. \n ", player, num);
				for(f=total-num+1; f < total+1; f++){ 
					sprintf(numBuf,"%d. ", f);
					tmp2 = strcat(aa,numBuf);
				}
				sprintf(inputBuf, "%s ▶︎ 숫자 %d개를 선택. \n\t\t\t :   현재 숫자[ %s] \n ", player, num,tmp2);
			//	sprintf(numBuf, "%d",df); 
			//	send(clisock_list[k], numBuf, strlen(numBuf),0);
				
				printf("\n\t ------------------------------------------------------------");
                                for(k = 0; k<2; k++)
                                        if(i==k){
                                                sprintf(resultBuf, "%s \n\n\t------------------------------------------------------\n\t\t\t 차례를 기다리세요.\n\t------------------------------------------------------\n", inputBuf);
                                                send(clisock_list[k], resultBuf,strlen(resultBuf), 0);
                                        }
                                        else{
                                                sprintf(resultBuf, "%s \n\n\t------------------------------------------------------\n\t\t\t 숫자를 입력하세요.\n↓----   ------------------------------------------------------",inputBuf);
                                                send(clisock_list[k], resultBuf, strlen(resultBuf),0);
                                        }
                                        turn = (i+1)%2;
                        }
                }
                else{
                        sprintf(input_error, "%s가 %s\n", player,"잘못입력");
                        for(j = 0; j < num_chat; j++) {
                                send(clisock_list[j], input_error, strlen(input_error),0);
                        }
                }
        }
        else{
                send(clisock_list[i], WRONG_TURN_STRING, strlen(WRONG_TURN_STRING), 0);
        }
}

void addClient(int s, struct sockaddr_in *newcliaddr) {
        char buf[20];
        inet_ntop(AF_INET, &newcliaddr->sin_addr, buf, sizeof(buf));
        printf("\n\t\t새로운 참가자 -->  ");
	clisock_list[num_chat] = s;
        num_chat++;
}
void removeClient(int i) {
        close(clisock_list[i]);
        if(i != num_chat-1) {
                clisock_list[i] = clisock_list[num_chat-1];
        }
        num_chat--;
	printf("\n\t\t\t 한명이 게임을 나갔습니다. \n\n\t\t 현재 게임에 남아있는 참가자 수%d명입니다.\n\n",num_chat);
}


/*
- non-blocking 모드 : 통신 상대가 여럿이거나 여러 가지 작업을 병행하려면 nonblocking 또는 비동기 모드를 사용하여야 한다.
- 비동기 : 순서없이 신호를 보내고 받는다 
- 네트워크가 즉시 처리할 수 있으면 해당 결과를 리턴 ( 대신 신뢰성 떨어짐 )
- 리턴값을 대게 -1, false를 리턴
- 결과 확인을 위한 반복적인 작업(폴링)으로 CPU낭비를 초래

*/

int is_nonblock(int sockfd) {
        int val;
        val = fcntl(sockfd, F_GETFL, 0);
        if(val & O_NONBLOCK)
                return 0;
        return -1;
}

int set_nonblock(int sockfd) {
        int val;
        val = fcntl(sockfd, F_GETFL, 0);
        if(fcntl(sockfd, F_SETFL, val | O_NONBLOCK) == -1) {
                return -1;
        }
        return 0;
}

int tcp_listen(int host, int port, int backlog) {
        int sd;
        struct sockaddr_in servaddr;

        sd = socket(AF_INET, SOCK_STREAM, 0);
        if(sd == -1) {
                perror("socket fail");
                exit(1);
        }

        bzero((char *)&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htonl(host);
        servaddr.sin_port = htons(port);
        if(bind(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
                perror("bind fail");
                exit(1);
        }

        listen(sd, backlog);
        return sd;
}
