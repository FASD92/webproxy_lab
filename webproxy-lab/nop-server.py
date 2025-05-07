#!/usr/bin/python3

# nop-server.py - This is a server that we use to create head-of-line
#                 blocking for the concurrency test. It accepts a
#                 connection, and then spins forever.
#
# usage: nop-server.py <port>                
# 프록시 서버의 동시성 구현이 제대로 되어 있는지를 검증할 수 있다.
# 다른 클라이언트의 요청이 정상적으로 처리된다면 -> 프록시가 동시성 지원을 잘 하고 있음
# 다른 클라이언트도 대기한다면 -> 프록시가 직렬 처리만 하고 있음
import socket
import sys

#create an INET, STREAMing socket
serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
serversocket.bind(('', int(sys.argv[1])))
serversocket.listen(5)

while 1:
  channel, details = serversocket.accept()
  while 1:
    continue
