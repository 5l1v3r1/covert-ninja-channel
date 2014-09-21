#ifndef COVERT_H
#define COVERT_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/prctl.h>
#include <sys/types.h>

#define USER_ROOT 	0
#define DEST_PORT 	8654
#define WINDOW_SIZE 	4068
#define DATA_SIZE 	5
#define UNREAD 		0
#define READ 		1
#define BUFFER_SIZE 	80
#define SEND_SOCKET 	0
#define RECV_SOCKET 	1
#define TCP_BUFFER 	10000
#define MASK 		"kthread2"
#define MAX_DECIMAL 	255

struct channel_info {
	int server;
	char desthost[BUFFER_SIZE];
	unsigned int dest_port;
	unsigned int dest_host;
	unsigned int w_size;
	char filename[BUFFER_SIZE];
} channel_info;

int recv_sock; /* recv socket used in server and signal handler functions */

void start_covert_channel();

#endif
