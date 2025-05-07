#! /usr/bin/perl -w
use strict;
use Digest::MD5;
#
# port-for-user.pl - Return a port number, p, for a given user, with a
#     low probability of collisions. The port p is always even, so that
#     users can use p and p+1 for testing with proxy and the Tiny web
#     server.
#     p는 프록시 서버가 사용, p+1은 Tiny 서버가 사용
#     usage: ./port-for-user.pl [optional user name]
#

"""
 예시 사용법
$ ./port-for-user.pl choeuseog
choeuseog: 21466

$ ./port-for-user.pl
your_username: 18822
"""
my $maxport = 65536;
my $minport = 1024;


# hashname - compute an even port number from a hash of the argument
sub hashname {
    my $name = shift;
    my $port;
    my $hash = Digest::MD5::md5_hex($name);

    # 이름을 MD5 해시한 뒤, 하위 32비트(8자리 hex)만 사용
    # take only the last 32 bits => last 8 hex digits
    $hash = substr($hash, -8);
    $hash = hex($hash);

    #   1024 ~ 65535 범위에서 포트 번호를 계산하고
    #   & 0xfffffffe로 짝수 포트를 만든다.
    $port = $hash % ($maxport - $minport) + $minport;
    $port = $port & 0xfffffffe;
    print "$name: $port\n";
}


# If called with no command line arg, then hash the userid, otherwise
# hash the command line argument(s).
#   아무 인자 없이 실행하면, 현재 로그인한 유저 이름을 기준으로 포트 생성
if($#ARGV == -1) {
    my ($username) = getpwuid($<);
    hashname($username);
#   인자를 주면 해당 인자들을 각각 포트로 변환
} else {
    foreach(@ARGV) {
        hashname($_);
    }
}