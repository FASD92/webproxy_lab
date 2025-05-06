/*
  1. 리슨 소켓 생성
  2. 무한 루프 진입
  3. 클라이언트 연결 요청 수락
  4. 클라이언트 정보 출력
  5. echo(connfd)로 데이터 처리
  6. 연결 종료 후 반복
*/
#include "csapp.h"

void echo(int connfd);  // 실제 클라이언트와 통신하는 함수
                        // 인자로 받은 connfd 소켓을 통해 데이터를 수신하고 그대로 송신

int main(int argc, char ** argv)    //  argv[1]로 포트 번호를 받아오기 위해 인자를 받고 있음
{
    int listenfd, connfd;   //  listenfd: 클라이언트 연결 요청을 기다리는 수동 소켓
	                        //  connfd: 연결이 수락된 후 클라이언트와 통신할 능동 소켓
    socklen_t clientlen;    //  clientlen: 클라이언트 주소 구조체의 크기를 담는 변수
    struct sockaddr_storage clientaddr; //  클라이언트의 주소 정보가 저장될 구조체 (sockaddr_storage는 범용적인 구조체로 IPv4/IPv6 모두 지원)
    char client_hostname[MAXLINE], client_port[MAXLINE];    //  클라이언트의 호스트 이름과 포트 번호를 문자열로 저장하기 위한 버퍼

    if (argc != 2)  {   //  명령줄 인자 오류 체크
                        //  포트 번호가 제공되지 않으면 오류 메시지를 출력하고 종료
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    listenfd = Open_listenfd(argv[1]);  // 전달 받은 포트 번호 argv[1]로 서버측 리슨 소켓 생성
                                        // Open_listenfd()는 내부적으로 getaddrinfo(), socket(), bind(), listen()을 포함하는 안전 함수
                                        // 반환값은 수동 대기 소켓 식별자
    while (1) { // 무한 루프 : 서버는 계속해서 클라이언트 요청을 기다리고 처리함
        clientlen = sizeof(struct sockaddr_storage);    // accept 호출 전에 clientlen값을 세팅, 실제 연결되면 클라이언트의 주소 크기로 갱신
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);   // 클라이언트 연결 요청 수락, 해당 연결에 대한 전용 소켓을 connfd에 반환.
                                                                    // 연결 후 리슨 소켓은 대기 상태로 남고, confd는 통신 전용 소켓이 됨
        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE,
                    client_port, MAXLINE, 0);   // 클라이언트 주소 구조체를 문자열 형태의 hostname과 port로 변환 -> 디버깅 및 로그 출력을 위한 용도
        printf("이쪽으로 연결됐슴당 (%s, %s)\n", client_hostname, client_port);
        echo(connfd);   // 클라이언트로부터 받은 데이터를 그대로 다시 보내주는 에코 처리 함수
        Close(connfd);  // 클라이언트와의 통신이 끝났으므로 해당 연결 소켓을 닫는 close 함수
    }
    exit(0);    // 정상 종료
}