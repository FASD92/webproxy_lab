/*
1. 명령줄 인자로 호스트와 포트 입력 받기  
2. 서버에 연결 요청 (Open_clientfd)  
3. 표준 입력으로부터 한 줄 입력  
4. 서버로 전송  
5. 서버 응답 받기  
6. 응답 출력  
7. 반복
*/

#include "csapp.h"  // CSAPP에서 제공하는 헬퍼 함수들을 포함하는 헤더 파일
// Open_clientfd, Rio_readinitb, Rio_readlineb, Rio_writen 등등 사용하기 위함임

int main(int argc, char **argv) // argc: 명령줄 인자의 개수(예: ./echoclient localhost 12345라면 argc==3)
// argv : 명령줄 인자들을 담고 있는 문자열 배열(예: argv[0] = "./echoclient")
{
    int clientfd;   //클라이언트측 소켓 식별자, 즉 서버와 통신하는 부분.
    char *host, *port, buf[MAXLINE];    // host: 접속할 서버 주소, port: 접속할 포트 번호, buf: 입출력을 위한 버퍼
    rio_t rio;  //  csapp.h에서 정의된 robust I/O 구조체
                //  Rio_readinitb()를 통해 초기화하고 이후 rio_readlineb()로 줄 단위 입력을 받음

    if (argc != 3)  {   // 명령줄 인자가 3개가 아니면
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);  // 프로그램명+호스트+포트 오류 메시지 출력 후
        exit(0);    // 종료해버림
    }
    host = argv[1]; //  명령줄 인자에서 host와
    port = argv[2]; //  port 추출, 당연히 둘 다 문자열이겠지.

    clientfd = Open_clientfd(host, port);   //  내부적으로 getaddrinfo(), socket(), connect()를 묶어둔 안전 래퍼
                                            //  서버의 주소와 포트를 사용해 TCP 연결을 맺고, 그 소켓 식별자를 반환
                                            // 실패 시 프로그램은 종료되도록 내부 예외 처리 포함임
    rio_readinitb(&rio, clientfd);          // Robust I/O를 위해 소켓 식별자(clientfd)로 rio 구조체를 초기화

    while (Fgets(buf, MAXLINE, stdin) != NULL ) {   //표준 입력으로부터 한 줄 읽어와 buf에 저장(최대 MAXLINE, 아마 8092?)
        Rio_writen(clientfd, buf, strlen(buf));     // 서버와 연결된 소켓에 데이터를 보냄. robust한 쓰기 보장
        rio_readlineb(&rio, buf, MAXLINE);          // 한 줄(개행 문자까지)을 robust하게 읽기
        Fputs(buf, stdout);                         // 받은 응답을 표준 출력에 출력
    }
    Close(clientfd);    //루프 종료 후, 소켓 닫고 연결 종료
    exit(0);            // 정상 종료
}