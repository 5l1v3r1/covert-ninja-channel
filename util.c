#include "util.h"

void usage(char *program_name)
{
	printf("Usage: %s -dest dest_ip -dest_port port -window-size window_size -file filename -server \n\n", program_name);
	printf("-dest dest_ip 	  - Host to send data to.\n");
	printf(" 		    In SERVER mode this is the server ip address\n");
	printf("-dest_port port   - IP source port you want data to go to. In\n");
	printf(" 		    SERVER mode this is the port data will be coming\n");
	printf(" 		    inbound on.\n");
	printf(" 		    If NOT specified, default is port 80.\n");
	printf("-window-size size - Window-size to send/receive (client and server MUST match window-size)\n");
	printf("		    If NOT specified, default is size 4068.\n");
	printf("-file filename 	  - Name of the file to encode and transfer.\n");
	printf("-server           - Server mode to allow receiving of data.\n");
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

	data[0] = MAX_DECIMAL - atoi(strtok(ip_str, "."));

	for(i = 1; i < DATA_SIZE - 1; i++)
		data[i] = MAX_DECIMAL - atoi(strtok(NULL, "."));

	return data;
}
