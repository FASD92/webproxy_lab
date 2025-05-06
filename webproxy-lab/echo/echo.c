/*
    1. 클라이언트 메시지 수신
    2. 서버에서 로그 출력
    3. 그대로 다시 클라이언트한테 전송
*/
#include "csapp.h"

void echo(int connfd){  //  클라이언트와의 통신을 처리하는 함수
                        //  인자로 받은 connfd는 서버에서 accept()로 획득한 클라이언트와 연결된 소켓
    size_t n;           //  읽은 바이트 수를 저장하는 변수 (size_t는 부호 없는 정수형)
    char buf[MAXLINE];  //  입출력을 위한 버퍼(한 줄씩)
    rio_t rio;          //  robust I/O용 구조체(내부적으로 connfd에 연결된 읽기 버퍼를 관리)

    Rio_readinitb(&rio, connfd);    // rio 구조체 초기화
                                    //  connfd를 내부 버퍼와 연결하여 rio_readlineb()로 줄 단위 처리를 가능하게 만듦
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0)    //  클라이언트로부터 한 줄씩 읽어들이는 루프
                                                            //  n이 0이 되면 EOF 종료
    {
        printf("서버 측에서 %d 바이트 받음ㅎㅎ\n", (int)n);
        Rio_writen(connfd, buf, n); //  받은 데이터를 클라이언트에게 그대로 다시 보냄
    }
    
}