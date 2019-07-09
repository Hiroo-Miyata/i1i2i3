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
#include <signal.h> //sigatcion()
#include <errno.h>  //erron, EINTR

#define MSG_FAILURE -1
#define MAX_MSGSIZE 1024
#define MAX_BUFSIZE (MAX_MSGSIZE + 1)
#define MAX_TRIES 5
#define TIMEOUT_SECOND 2

int get_socket(const char *);
void sockaddr_init(const char *, unsigned short, struct sockaddr *);
int udp_send(int, const char *, int, struct sockaddr *);
int udp_receive(int, char *, int, struct sockaddr *);
void socket_close(int);
int input(char *, int);
void remove_lf(char *, int);
void catchAlarm(int);
int udp_try_receive(int, struct sockaddr *, struct sockaddr *, char *, int, char *);
int check_correct_server(struct sockaddr *, struct sockaddr *);

int intTries = 0;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "argument count mismatch error.\n");
        exit(EXIT_FAILURE);
    }

    const char *address = argv[1];
    unsigned short port = (unsigned short)atoi(argv[2]);
    struct sockaddr_in to, from;
    to.sin_family = AF_INET;         /* ͜IPv4 */
    to.sin_port = htons(port);       /* port */
    to.sin_addr.s_addr = INADDR_ANY; /* IPは指定せず */

    int s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    while (1)
    {
        char sendBuffer[MAX_BUFSIZE];
        char receiveBuffer[MAX_BUFSIZE];

        if (strcmp(sendBuffer, "quit\n") == 0)
        {
            socket_close(s);
            break;
        }

        int receivedSize = udp_try_receive(s, &to, &from, sendBuffer, MAX_BUFSIZE, receiveBuffer);

        if (check_correct_server(&to, &from) == -1)
        {
            continue;
        }

        remove_lf(receiveBuffer, receivedSize);
    }

    return EXIT_SUCCESS;
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

void socket_close(int server)
{
    if (close(server) < 0)
    {
        perror("close() failed.");
        exit(EXIT_FAILURE);
    }
}

void remove_lf(char *buffer, int bufferSize)
{
    buffer[bufferSize - 1] = '\0';
}

void catchAlarm(int ignored)
{
    intTries += 1;
}

int udp_try_receive(int sock, struct sockaddr *to, struct sockaddr *from, char *sendBuffer, int sendSize, char *receiveBuffer)
{

    int sendedSize = udp_send(sock, sendBuffer, sendSize, to);
    int receivedSize;
    while (1)
    {
        alarm(TIMEOUT_SECOND);
        receivedSize = udp_receive(sock, receiveBuffer, MAX_BUFSIZE, from);
        if (receivedSize == MSG_FAILURE)
        {
            if (errno == EINTR)
            {
                if (intTries <= MAX_TRIES)
                {
                    printf("timed out %d.\n", intTries);
                    sendedSize = udp_send(sock, sendBuffer, sendSize, to);
                    if (sendedSize == MSG_FAILURE)
                        break;
                    alarm(TIMEOUT_SECOND);
                    continue;
                }
                else
                {
                    printf("total timed out %d.\n", MAX_TRIES);
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                exit(EXIT_FAILURE);
            }
        }
        break;
    }
    alarm(0);

    return receivedSize;
}

int check_correct_server(struct sockaddr *sockaddr_1, struct sockaddr *sockaddr_2)
{
    if (((struct sockaddr_in *)sockaddr_1)->sin_addr.s_addr != ((struct sockaddr_in *)sockaddr_2)->sin_addr.s_addr)
    {
        fprintf(stderr, "reveiceid from unknown server.\n");
    }
    else if (ntohs(((struct sockaddr_in *)sockaddr_1)->sin_port) != ntohs(((struct sockaddr_in *)sockaddr_2)->sin_port))
    {
        fprintf(stderr, "reveiceid from unknown port.\n");
    }
    else
    {
        return EXIT_SUCCESS;
    }

    return MSG_FAILURE;
}