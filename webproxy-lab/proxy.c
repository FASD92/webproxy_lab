/*
1단계: 기본 시퀀셜 프록시 (Sequential Proxy)
2단계: 동시성 처리 (Concurrency)
3단계: 캐시 구현 (Web Object Cache)
*/

#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
int parse_uri(char *uri, char *hostname, char *port, char *path);
void handle_request(int connfd);
void read_requesthdrs(rio_t *rp);
void *thread(void *vargp);  // <-- 추가

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

int main(int argc, char **argv)
{
  int listenfd;
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;
  char hostname[MAXLINE], port[MAXLINE];
  pthread_t tid;


  if (argc != 2) {
      fprintf(stderr, "usage: %s <port>\n", argv[0]);
      exit(1);
  }

  listenfd = Open_listenfd(argv[1]);

  while (1) {
      clientlen = sizeof(clientaddr);

      int *connfdp = Malloc(sizeof(int));
      *connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen);

      Pthread_create(&tid, NULL, thread, connfdp);

      Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
      printf("Accepted connection from (%s, %s)\n", hostname, port);
  }
}

int parse_uri(char *uri, char *hostname, char *port, char *path)
{
    char *hostbegin, *pathbegin, *portpos;

    // 1. "http://" 문자열을 건너뛴 후 호스트 이름이 시작하는 위치를 찾음
    // 예: "http://www.example.com:8080/index.html" → hostbegin = "www.example.com:8080/index.html"
    hostbegin = uri + 7;  // "http://"는 7글자

    // 2. '/' 문자를 기준으로 경로(path)의 시작점을 찾음
    // 예: hostbegin = "www.example.com:8080/index.html" → pathbegin = "/index.html"
    pathbegin = strchr(hostbegin, '/');

    if (pathbegin) {
        // pathbegin이 존재하면 path에 복사하고
        // 그 위치를 NULL로 바꿔 hostbegin에서 hostname과 port만 남김
        strcpy(path, pathbegin);  // path = "/index.html"
        *pathbegin = '\0';        // hostbegin = "www.example.com:8080"
    } else {
        // '/'가 없다면 path는 "/" (기본 루트)
        strcpy(path, "/");
    }

    // 3. ':' 문자를 기준으로 포트번호가 있는지 확인
    // 예: hostbegin = "www.example.com:8080"
    portpos = strchr(hostbegin, ':');

    if (portpos) {
        // ':'가 있다면 hostname과 port 분리
        *portpos = '\0';            // hostbegin = "www.example.com"
        strcpy(hostname, hostbegin); // hostname = "www.example.com"
        strcpy(port, portpos + 1);   // port = "8080"
    } else {
        // ':'가 없다면 포트는 기본값 "80"으로 설정
        strcpy(hostname, hostbegin); // hostname = "www.example.com"
        strcpy(port, "80");
    }

    return 0;  // 성공적으로 파싱했음을 의미
}

void handle_request(int connfd)
{
  char buf[MAXLINE];
  rio_t rio;
  char method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char hostname[MAXLINE], port[MAXLINE], path[MAXLINE];

  // 1. request line 읽기
  Rio_readinitb(&rio, connfd);
  if (!Rio_readlineb(&rio, buf, MAXLINE))
  {
    return;
  }
  sscanf(buf, "%s %s %s", method, uri, version);

  // 2. 메서드 확인
  if (strcasecmp(method, "GET")) {
    clienterror(connfd, method, "501", "Not Implemented", "Tiny does not implement this method");
    return;
  }

  // 3. 요청 헤더 읽기
  // 클라이언트의 요청 헤더를 무시하면서 모두 읽어 들임
  read_requesthdrs(&rio);

  // 4. URI 파싱(프록시용 hostname/port/path 추출)
  parse_uri(uri, hostname, port, path);

  int serverfd = Open_clientfd(hostname, port);
  if (serverfd < 0)
  {
    fprintf(stderr, "연결 실패\n");
    return;
  }

  // 요청 라인
sprintf(buf, "GET %s HTTP/1.0\r\n", path);
Rio_writen(serverfd, buf, strlen(buf));

// Host 헤더
sprintf(buf, "Host: %s\r\n", hostname);
Rio_writen(serverfd, buf, strlen(buf));

// 기본 헤더들
sprintf(buf, "Connection: close\r\n");
Rio_writen(serverfd, buf, strlen(buf));

sprintf(buf, "Proxy-Connection: close\r\n");
Rio_writen(serverfd, buf, strlen(buf));

sprintf(buf, "%s", user_agent_hdr);  // 이미 proxy.c에 있음
Rio_writen(serverfd, buf, strlen(buf));

// 빈 줄
sprintf(buf, "\r\n");
Rio_writen(serverfd, buf, strlen(buf));

// 서버의 응답을 읽어 클라이언트에 전달
rio_t rio_server;
Rio_readinitb(&rio_server, serverfd);

ssize_t n;
while ((n = Rio_readlineb(&rio_server, buf, MAXLINE)) > 0) {
  Rio_writen(connfd, buf, n);
}
Close(serverfd);  // 서버 소켓 종료
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
    char buf[MAXLINE], body[MAXBUF];
    int len = 0;

    len += snprintf(body + len, MAXBUF - len, "<html><title>Tiny Error</title>");
    len += snprintf(body + len, MAXBUF - len, "<body bgcolor=\"ffffff\">\r\n");
    len += snprintf(body + len, MAXBUF - len, "%s: %s\r\n", errnum, shortmsg);
    len += snprintf(body + len, MAXBUF - len, "<p>%s: %s\r\n", longmsg, cause);
    len += snprintf(body + len, MAXBUF - len, "<hr><em>프록시 서버임당</em>\r\n");

    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", len);
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, len);
}

void read_requesthdrs(rio_t *rp)
{
    char buf[MAXLINE];
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
    while (strcmp(buf, "\r\n")) {
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
}

void *thread(void *vargp)
{
    int connfd = *((int *)vargp);
    Pthread_detach(pthread_self());
    Free(vargp);
    handle_request(connfd);
    Close(connfd);
    return NULL;
}