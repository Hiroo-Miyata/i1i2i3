#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <complex.h>
#include "fft.h"

#define MAX_SIZE 1500

int server_socket_init(int);
int client_socket_init(int, char *);
void rec_and_play(int);

int main(int argc, char *argv[])
{
    if (argc == 2) /* server */
    {
        // 初期データ
        int arg1 = atoi(argv[1]); /* port番号 */
        int s = server_socket_init(arg1);
        rec_and_play(s);
    }
    else if (argc == 3) /* client */
    {
        int arg1 = atoi(argv[2]); /* port番号 */
        int s = client_socket_init(arg1, argv[1]);
        rec_and_play(s);
    }
    else
    {
        perror("Command Line Error");
        exit(1);
    }
}

int server_socket_init(int port)
{
    //サーバー基本設定
    int ss = socket(PF_INET, SOCK_STREAM, 0); /* ソケsット */
    struct sockaddr_in addr;                  /* address の情報 */
    addr.sin_family = AF_INET;                /* ͜IPv4 */
    addr.sin_port = htons(port);              /* port */
    addr.sin_addr.s_addr = INADDR_ANY;        /* IPは指定せず */
    int n = bind(ss, (struct sockaddr *)&addr, sizeof(addr));
    printf("%d", n);
    listen(ss, 10);

    //接続元情報
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    int s = accept(ss, (struct sockaddr *)&client_addr, &len);
    printf("\n connect is succeeded \n client IP address: %d \n", client_addr.sin_addr.s_addr);
    close(ss);

    return s;
}

int client_socket_init(int port, char *address)
{
    // サーバー基本設定
    int s = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;                  /* ͜IPv4 を使用する */
    int e = inet_aton(address, &addr.sin_addr); /* IP の指定*/
    if (e == 0)
    {
        perror("IP error");
        exit(1);
    }
    addr.sin_port = htons(port);                                  /* port の指定*/
    int ret = connect(s, (struct sockaddr *)&addr, sizeof(addr)); /* connect */

    return s;
}

void rec_and_play(int socket)
{
    char *cmdr = "rec -t raw -b 16 -c 1 -e s -r 44100 --buffer 1024 -";  /* コマンドライン：　録音 */
    char *cmdp = "play -t raw -b 16 -c 1 -e s -r 44100 --buffer 1024 -"; /* コマンドライン：　録音 */

    // 送受信変数
    FILE *fcmdr; /* 録音FILEパス */
    FILE *fcmdp;
    char send_data[MAX_SIZE];       /* 録音データ格納 */
    int send_data_index = MAX_SIZE; /* データの長さ */
    char recv_data[MAX_SIZE];       /* 送られたデータ格納 */
    int recv_data_index = MAX_SIZE; /* データの長さ */
    long n = 8192;
    sample_t *buf = calloc(sizeof(sample_t), n);
    complex double *X = calloc(sizeof(complex double), n);
    complex double *Y = calloc(sizeof(complex double), n);
    fcmdr = popen(cmdr, "r");
    fcmdp = popen(cmdp, "w");
    while (1)
    {
        //録音データを送る
        fread(send_data, 1, send_data_index, fcmdr);
        /* 複素数の配列に変換 */
        sample_to_complex(buf, X, n);
        /* FFT -> Y */
        fft(X, Y, n);
        send(socket, send_data, send_data_index, 0);

        //送られてたデータを再生
        read(socket, recv_data, recv_data_index);
        /* IFFT -> Z */
        ifft(Y, X, n);
        /* 標本の配列に変換 */
        complex_to_sample(X, buf, n);
        fwrite(recv_data, 1, recv_data_index, fcmdp);
    }
    pclose(fcmdr);
}