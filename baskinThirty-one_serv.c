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

char *EXIT_STRING = "exit";
char *START_STRING = "Connected to Game\n";

char *FIRST_STRING= "PUSH 입력 : ";
char *SECOND_STRING = "Wait 잠시만 기다려주세요 \n******************************************\n";
char *START = "게임을 시작하겠습니다.\n ㄴ숫자를 1~3 중에 입력해주세요\n******************************************\n";
char *END = "\n******************************************\n게임이 끝났습니다. 수고하셨습니다. ^_^ \n";
char *WIN = "\n******************************************\n이겼습니다. 축하드립니다 *_* \n";
char *LOSE = "\n******************************************\n졌습니다. 다음 기회를 노려보세요 ㅠ_ㅠ \n";
char *WRONG_TURN_STRING = " ※경고※ 당신의 차례가 아닙니다 \n";

int maxfdp1;
int num_chat = 0;
int clisock_list[MAX_SOCK];
int listen_sock;

int is_GameStart = 0;
int turn = 0;
int total = 0;
char input_error[30];
char test[30];
void rule_print();
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
        struct sockaddr_in cliaddr;
        char buf[MAXLINE];
        int i,j,nbyte,k;
        int accp_sock, clilen, addrlen;

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
        rule_print();
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
                        addClient(accp_sock, &cliaddr);
                        send(accp_sock, START_STRING, strlen(START_STRING), 0);
                        printf("%d번째 사용자 추가.\n", num_chat);

                        if(num_chat == 2) {
                                for(k = 0; k < num_chat; k++) {
                                        send(clisock_list[k], START,strlen(START), 0);
                                }
                                send(clisock_list[0], FIRST_STRING, strlen(FIRST_STRING), 0);
                                send(clisock_list[1], SECOND_STRING, strlen(SECOND_STRING), 0);
                                is_GameStart = 1;
                                printf("READY !!! \n");
                        }
                }
                for(i = 0; i<num_chat; i++) {
                        if(is_GameStart == 1)
                                game_play(i);
                }
        }
}

void rule_print(){
        printf(" ==============================================================================\n");
        printf(" < 베스킨 라빈스 31 게임 rule > \n 1. 1부터 시작해서 연속된 숫자로 부른다. \n 2. 건너뛰지 않고 숫자를 3개까지 이어서 해야한다. \n 3. 상대방이 마지막 부른 숫자에 이어서 똑같은 규칙대로 숫자를 이어 부른다. \n 4. 31을 먼저 부르는 사람이 진다. \n " );
printf(" ==============================================================================\n");
}

void game_play(int i){
        int k, j, nbyte, num;
        int present;
        char inputBuf[150];
        char resultBuf[150];
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
                if(num > 0 && num < 4) {
                        printf("*****************************************************\n");
                        printf(" %s 가 부른 숫자 리스트 :  ",player);
                        for( present=total+1; present<=total+num; present++){
                                printf(" %d", present);
                                if(present!=total+num)
                                        printf(",");
                        }
                        puts("");
                        total += num;
                        printf("현재까지 3 1 게임의 마지막 수 => %d 입니다\n", total);
                        if(total >= 31) {
                                for(k = 0 ; k<2; k++){
                                        if(i==k)
                                                send(clisock_list[k],LOSE,strlen(LOSE), 0);
                                        else
                                                send(clisock_list[k],WIN,strlen(WIN), 0);
                                }
                                printf("최종 : %s가 졌습니다.\n",player);
                                is_GameStart = 0;
                        }
                        else{
                                sprintf(inputBuf, "%s 님이 %d를 선택하셨습니다. 현재 숫자[%d]\n", player, num, total);
                                for(k = 0; k<2; k++)
                                        if(i==k){
                                                sprintf(resultBuf, "%s \n 차례를 기다리세요....", inputBuf);
                                                send(clisock_list[k], resultBuf,strlen(resultBuf), 0);
                                        }
                                        else{
                                                sprintf(resultBuf, "%s \n 당신의 차례입니다.....:)",inputBuf);
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
        printf("new client : %s\n", buf);
        clisock_list[num_chat] = s;
        num_chat++;
}
void removeClient(int i) {
        close(clisock_list[i]);
        if(i != num_chat-1) {
                clisock_list[i] = clisock_list[num_chat-1];
        }
        num_chat--;
        printf("채팅 참가자 1명 탈퇴. 현재 참가수 = %d\n", num_chat);
}

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

