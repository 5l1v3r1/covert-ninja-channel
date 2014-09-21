#include "client_ops.h"

/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: client_ops.c - This source file holds functions for the client.
-- 
-- FUNCTIONS: 	int client_file_io();
--		void forge_packet_client(struct in_addr addr, unsigned int forged_src_port);
-- 
-- DATE: 2014/09/20
-- 
-- REVISIONS: (Date and Description)
-- 
-- DESIGNER: Luke Tao
-- 
-- PROGRAMMER: Luke Tao
-- 
-- NOTES: These 2 functions are used by the client, for client file I/O operations and packet forging. 
----------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
-- FUNCTION: client_file_io
-- 
-- DATE: 2014/09/20
-- 
-- REVISIONS: (Date and Description)
-- 
-- DESIGNER: Luke Tao
-- 
-- PROGRAMMER: Luke Tao
-- 
-- INTERFACE: int client_file_io()
-- 
-- RETURNS: 0 on successful file reading.
-- 
-- NOTES: This client function essentially opens a file for reading, reads 5 bytes of data, and then send them
--	  over the network. Note that the data is encrypted by subtracting 255 to each of the decimal ASCII-equivalent 
--	  characters (same way as decrypting, see convert_ip_to_string function in util.c). This function also uses
--	  timestamps where it calculates from the start of the packet forging, sending it over to the server,
-- 	  to the end of the sleep time. The timestamps are for testing purposes.
----------------------------------------------------------------------------------------------------------------------*/
int client_file_io()
{
	FILE * input;
	int file_size;
	int file_pos = 0;
	int read_bytes;
	struct sockaddr_in addr;
	struct timeval ttp;
	double tt1, tt2;
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

		sprintf(ip_addr, "%d.%d.%d.%d", MAX_DECIMAL - rbuffer[0], MAX_DECIMAL - rbuffer[1], 
			MAX_DECIMAL - rbuffer[2], MAX_DECIMAL - rbuffer[3]);

		inet_aton(ip_addr, &addr.sin_addr);
		addr.sin_port = rbuffer[4];

		/* Time start */
		gettimeofday(&ttp, NULL);
		tt1 = ttp.tv_sec + (ttp.tv_usec / 1000000.0);

		forge_packet_client(addr.sin_addr, addr.sin_port);
		usleep(rand() % 7000000 + 100000);

		/* Time finish */
		gettimeofday(&ttp, NULL);
		tt2 = ttp.tv_sec + (ttp.tv_usec / 1000000.0);

		printf("Time to send 1 packet: %.4lf seconds.\n", tt2-tt1);

		memset(rbuffer, 0, sizeof(rbuffer));
	}
	fclose(input);
	return 0;
}

/*--------------------------------------------------------------------------------------------------------------------
-- FUNCTION: forge_packet_client
-- 
-- DATE: 2014/09/20
-- 
-- REVISIONS: (Date and Description)
-- 
-- DESIGNER: Luke Tao
-- 
-- PROGRAMMER: Luke Tao
-- 
-- INTERFACE: void forge_packet_client(struct in_addr addr, unsigned int forged_src_port)
--				       struct in_addr addr 	    - forged source IP
--				       unsigned int forged_src_port - forged source port
--
-- 
-- RETURNS: void.
-- 
-- NOTES: This function passes in the forged source IP and port and embeds them into the packet. Afterwards, it
--	  performs a checksum for both the IP and TCP headers, then sends them to the server.
----------------------------------------------------------------------------------------------------------------------*/
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

	send_pkt.tcp.source = htons(forged_src_port * 128);
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
	printf("Sending data through src IP masked as %s\t = %s\n", inet_ntoa(addr), data);
	printf("Sending data through src port masked as %d = ASCII decimal: %d = %c\n",
		ntohs(send_pkt.tcp.source), ntohs(send_pkt.tcp.source) / 128, ntohs(send_pkt.tcp.source) / 128); 
		
	close_socket(send_socket);
	free(data);
}

