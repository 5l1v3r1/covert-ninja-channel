#include "socket_ops.h"


int create_raw_socket(int sock_proto)
{
	int sock_fd;

	if((sock_proto == SEND_SOCKET) && (sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
	{
		perror("Send Socket cannot be created. Are you root?");
		exit(1);
	}
	else if ((sock_proto == RECV_SOCKET) && (sock_fd = socket(AF_INET, SOCK_RAW, 6)) < 0)
	{
		perror ("Receive Socket cannot be created. Are you root?");
		exit(1);
	}
	return sock_fd;
}
int close_socket(int sock_d)
{
	close(sock_d);
	return 0;
}
void sig_proc()
{
	close_socket(recv_sock);
	printf("\nServer Socket closed.\n");
	exit(0);
}
