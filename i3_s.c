#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#define MSG_FAILURE -1

#define MAX_MSGSIZE 1024
#define MAX_BUFSIZE (MAX_MSGSIZE + 1)

int udp_send(int, const char *, int, struct sockaddr *);
int udp_receive(int, char *, int, struct sockaddr *);
void sockaddr_init(const char *, unsigned short, struct sockaddr *);

int main(int argc, char *argv[])
{
    const char *address = "";
    unsigned short port = (unsigned short)atoi(argv[1]); /* port */
    struct sockaddr to, from;                            /* address の情報 */
    sockaddr_init(address, port, &to);

    int s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (bind(s, &to, sizeof(to)) < 0)
    {
        perror("bind() failed.");
        exit(EXIT_FAILURE);
    }

    char *cmdr = "rec -t raw -b 16 -c 1 -e s -r 44100 -"; /* コマンドライン：　録音 */

    // 送受信変数
    FILE *fcmdr; /* 録音FILEパス */
    char sendBuffer[MAX_BUFSIZE];
    char receiveBuffer[MAX_BUFSIZE];
    fcmdr = popen(cmdr, "r");

    while (1)
    {
        // int recvMsgSize = udp_receive(s, receiveBuffer, MAX_BUFSIZE, &from);
        // if (recvMsgSize == MSG_FAILURE)
        //     continue;
        // write(1, receiveBuffer, MAX_MSGSIZE);
        fread(sendBuffer, 1, MAX_MSGSIZE, fcmdr);
        int sendMsgSize = udp_send(s, sendBuffer, MAX_MSGSIZE, &from);
        if (sendMsgSize == MSG_FAILURE)
            continue;
    }
}
void sockaddr_init(const char *address, unsigned short port, struct sockaddr *sockaddr)
{

    struct sockaddr_in sockaddr_in;
    sockaddr_in.sin_family = AF_INET;

    if (inet_aton(address, &sockaddr_in.sin_addr) == 0)
    {
        if (strcmp(address, "") == 0)
        {
            sockaddr_in.sin_addr.s_addr = htonl(INADDR_ANY);
        }
        else
        {
            fprintf(stderr, "Invalid IP Address.\n");
            exit(EXIT_FAILURE);
        }
    }

    if (port == 0)
    {
        fprintf(stderr, "invalid port number.\n");
        exit(EXIT_FAILURE);
    }
    sockaddr_in.sin_port = htons(port);

    *sockaddr = *((struct sockaddr *)&sockaddr_in);
}

int udp_send(int sock, const char *data, int size, struct sockaddr *sockaddr)
{
    int sendSize;
    sendSize = sendto(sock, data, size, 0, sockaddr, sizeof(*sockaddr));
    if (sendSize != size)
    {
        perror("sendto() failed.");
        return MSG_FAILURE;
    }
    return sendSize;
}

int udp_receive(int sock, char *buffer, int size, struct sockaddr *sockaddr)
{
    unsigned int sockaddrLen = sizeof(*sockaddr);
    int receivedSize = recvfrom(sock, buffer, MAX_BUFSIZE, 0, sockaddr, &sockaddrLen);
    if (receivedSize < 0)
    {
        perror("recvfrom() failed.");
        return MSG_FAILURE;
    }

    return receivedSize;
}