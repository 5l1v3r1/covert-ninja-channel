#include "server_ops.h"

/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: server_ops.c - This source file holds functions for the server.
-- 
-- FUNCTIONS: 	void decrypt_packet_server();
--		int server_file_io(char * recv_buffer);
-- 
-- DATE: 2014/09/20
-- 
-- REVISIONS: (Date and Description)
-- 
-- DESIGNER: Luke Tao
-- 
-- PROGRAMMER: Luke Tao
-- 
-- NOTES: These 2 functions are used by the server, for server file I/O operations and packet decryption.
----------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
-- FUNCTION: decrypt_packet_server
-- 
-- DATE: 2014/09/20
-- 
-- REVISIONS: (Date and Description)
-- 
-- DESIGNER: Luke Tao
-- 
-- PROGRAMMER: Luke Tao
-- 
-- INTERFACE: void decrypt_packet_server()
-- 
-- RETURNS: void.
-- 
-- NOTES: This server essentially reads incoming packets, decrypts both the source IP and port for each packet, and then 
--	  writes them to a file via server_file_io() function.
----------------------------------------------------------------------------------------------------------------------*/
void decrypt_packet_server()
{
	struct recv_tcp
   	{
		struct ip ip;
		struct tcphdr tcp;
		char buffer[TCP_BUFFER];
   	} recv_pkt;	

   	int recv_len;
   	char * data;
  	char src_port_data[2];

	signal(SIGINT, sig_proc);
   	recv_sock = create_raw_socket(RECV_SOCKET);

   	while(1)
   	{
   		if ((recv_len = read(recv_sock, (struct recv_tcp *)&recv_pkt, TCP_BUFFER)) < 0) {
   			perror("Cannot read socket. Are you root?\n");
   			break;
		}

		/* Check to see if the packet has the SYN/ACK flag set and is the same window size as the client */
		if((recv_pkt.tcp.syn == 1) && (ntohs(recv_pkt.tcp.window) == channel_info.w_size))
		{
			data = convert_ip_to_string(recv_pkt.ip.ip_src);
			printf("Receiving Data from Forged Src IP: %s\n", data);
			printf("Receiving Data from Src Port: %c\n", ntohs(recv_pkt.tcp.source) / 128);
			
			sprintf(src_port_data, "%c", ntohs(recv_pkt.tcp.source) / 128);

			strcat(data, src_port_data); /* Join src IP string with the src port string */

			if(server_file_io(data) < 0)
			{
				perror("Cannot write to file\n");
				exit(1);
			}
			free(data);
		}
   	}
}

/*--------------------------------------------------------------------------------------------------------------------
-- FUNCTION: server_file_io
-- 
-- DATE: 2014/09/20
-- 
-- REVISIONS: (Date and Description)
-- 
-- DESIGNER: Luke Tao
-- 
-- PROGRAMMER: Luke Tao
-- 
-- INTERFACE: int server_file_io(char * recv_buffer)
-- 
-- RETURNS: 0 on successful file writing, -1 if there's a problem opening a file.
-- 
-- NOTES: This server function essentially opens a file, writes the receiving data into the file, then closes it.
----------------------------------------------------------------------------------------------------------------------*/
int server_file_io(char * recv_buffer)
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
