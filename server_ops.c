#include "server_ops.h"

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

		if((recv_pkt.tcp.syn == 1) && (ntohs(recv_pkt.tcp.window) == channel_info.w_size))
		{
			data = convert_ip_to_string(recv_pkt.ip.ip_src);
			printf("Receiving Data from Forged Src IP: %s\n", data);
			printf("Receiving Data from Src Port: %c\n", ntohs(recv_pkt.tcp.source) / 128);
			
			sprintf(src_port_data, "%c", ntohs(recv_pkt.tcp.source) / 128);

			strcat(data, src_port_data);

			if(server_file_io(data) < 0)
			{
				perror("Cannot write to file\n");
				exit(1);
			}
			free(data);
		}
   	}
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
