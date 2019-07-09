#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#define N 1024

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		printf("please input port (& adress)!!\n");
		return 1;
	}

	if (argc > 3)
	{
		printf("too much information of port and adress!!\n");
		return 1;
	}

	unsigned short data_r[N];
	unsigned short data_p[N];

	int i;
	for (i = 0; i < N; i++)
	{
		data_r[i] = 0;
		data_p[i] = 0;
	}

	if (argc == 2)
	{
		int s = socket(PF_INET, SOCK_STREAM, 0);
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(atoi(argv[1]));
		addr.sin_addr.s_addr = INADDR_ANY;
		bind(s, (struct sockaddr *)&addr, sizeof(addr));
		listen(s, 10);
		struct sockaddr_in client_addr;
		socklen_t len = sizeof(struct sockaddr_in);
		int ss = accept(s, (struct sockaddr *)&client_addr, &len);
		close(s);

		char *cmdline_r = "rec -t raw -b 16 -c 1 -e s -r 44100 - ";
		FILE *rec;
		rec = popen(cmdline_r, "r");

		// char *cmdline_p = "play -t raw -b 16 -c 1 -e s -r 44100 - ";
		// FILE *play;
		// play = popen(cmdline_p, "w");

		while (1)
		{
			fread(data_r, 1, N, rec);
			send(ss, data_r, N, 0);
			// recv(ss, data_p, 2 * N, 0);
			// fwrite(data_p, 2, N, play);
		}
	}

	if (argc == 3)
	{
		int s = socket(PF_INET, SOCK_STREAM, 0);
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(atoi(argv[1]));
		inet_aton(argv[2], &addr.sin_addr);
		connect(s, (struct sockaddr *)&addr, sizeof(addr));

		// char *cmdline_r = "rec -t raw -b 16 -c 1 -e s -r 44100 - ";
		// FILE *rec;
		// rec = popen(cmdline_r, "r");

		char *cmdline_p = "play -t raw -b 16 -c 1 -e s -r 44100 - ";
		FILE *play;
		play = popen(cmdline_p, "w");

		while (1)
		{
			recv(s, data_p, N, 0);
			fwrite(data_p, 1, N, play);
			// fread(data_r, 2, N, rec);
			// send(s, data_r, 2 * N, 0);
		}
	}

	return 0;
}