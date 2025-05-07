/* Modified tiny.c with internal CGI handler instead of fork/exec */
// 내부 함수 호출 버전
#include "csapp.h"

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
void adder(int fd, char *cgiargs);

int main(int argc, char **argv)
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        doit(connfd);
        Close(connfd);
    }
}

void doit(int fd)
{
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, fd);
    if (!Rio_readlineb(&rio, buf, MAXLINE)) return;
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    if (strcasecmp(method, "GET")) {
        clienterror(fd, method, "501", "Not Implemented", "Tiny does not implement this method");
        return;
    }
    read_requesthdrs(&rio);

    is_static = parse_uri(uri, filename, cgiargs);
    if (stat(filename, &sbuf) < 0) {
        clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
        return;
    }

    if (is_static) {
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
            clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
            return;
        }
        serve_static(fd, filename, sbuf.st_size);
    } else {
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
            clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program");
            return;
        }
        serve_dynamic(fd, filename, cgiargs);
    }
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

int parse_uri(char *uri, char *filename, char *cgiargs)
{
    char *ptr;
    if (!strstr(uri, "cgi-bin")) {
        strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcat(filename, uri);
        if (uri[strlen(uri) - 1] == '/')
            strcat(filename, "home.html");
        return 1;
    } else {
        ptr = index(uri, '?');
        if (ptr) {
            strcpy(cgiargs, ptr + 1);
            *ptr = '\0';
        } else {
            strcpy(cgiargs, "");
        }
        strcpy(filename, ".");
        strcat(filename, uri);
        return 0;
    }
}

void serve_static(int fd, char *filename, int filesize)
{
    int srcfd;
    char *srcbuf;
    char filetype[MAXLINE], buf[MAXBUF];

    get_filetype(filename, filetype);

    int n = 0;
    n += snprintf(buf + n, MAXBUF - n, "HTTP/1.0 200 OK\r\n");
    n += snprintf(buf + n, MAXBUF - n, "Server: Tiny Web Server\r\n");
    n += snprintf(buf + n, MAXBUF - n, "Connection: close\r\n");
    n += snprintf(buf + n, MAXBUF - n, "Content-length: %d\r\n", filesize);
    n += snprintf(buf + n, MAXBUF - n, "Content-type: %s\r\n\r\n", filetype);
    Rio_writen(fd, buf, strlen(buf));

    srcfd = Open(filename, O_RDONLY, 0);
    srcbuf = malloc(filesize);
    Rio_readn(srcfd, srcbuf, filesize);
    Close(srcfd);
    Rio_writen(fd, srcbuf, filesize);
    free(srcbuf);
}

void get_filetype(char *filename, char *filetype)
{
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
        strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else
        strcpy(filetype, "text/plain");
}

/* Internal dynamic CGI handler (no fork/exec) */
void serve_dynamic(int fd, char *filename, char *cgiargs)
{
    if (strstr(filename, "adder")) {
        adder(fd, cgiargs);
    } else {
        char buf[MAXLINE];
        sprintf(buf, "HTTP/1.0 501 Not Implemented\r\n");
        Rio_writen(fd, buf, strlen(buf));
        sprintf(buf, "Content-type: text/html\r\n\r\n");
        Rio_writen(fd, buf, strlen(buf));
        sprintf(buf, "<html><body><h1>501 Not Implemented</h1></body></html>\r\n");
        Rio_writen(fd, buf, strlen(buf));
    }
}

void adder(int fd, char *cgiargs)
{
    char buf[MAXLINE];
    char *query_string = cgiargs;
    int n1 = 0, n2 = 0;
    char *p;
    char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];

    strcpy(buf, query_string);
    p = strchr(buf, '&');
    if (p) {
        *p = '\0';
        strcpy(arg1, buf);
        strcpy(arg2, p + 1);
        n1 = atoi(strchr(arg1, '=') + 1);
        n2 = atoi(strchr(arg2, '=') + 1);
    }

    sprintf(content, "QUERY_STRING=%s\r\n<p>", query_string);
    sprintf(content + strlen(content), "Welcome to add.com: ");
    sprintf(content + strlen(content), "THE Internet addition portal.\r\n<p>");
    sprintf(content + strlen(content), "The answer is: %d + %d = %d\r\n<p>", n1, n2, n1 + n2);
    sprintf(content + strlen(content), "Thanks for visiting!\r\n");

    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(content));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, content, strlen(content));
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
    char buf[MAXLINE], body[MAXBUF];
    int len = 0;

    len += snprintf(body + len, MAXBUF - len, "<html><title>Tiny Error</title>");
    len += snprintf(body + len, MAXBUF - len, "<body bgcolor=\"ffffff\">\r\n");
    len += snprintf(body + len, MAXBUF - len, "%s: %s\r\n", errnum, shortmsg);
    len += snprintf(body + len, MAXBUF - len, "<p>%s: %s\r\n", longmsg, cause);
    len += snprintf(body + len, MAXBUF - len, "<hr><em>The Tiny Web server</em>\r\n");

    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", len);
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, len);
}
