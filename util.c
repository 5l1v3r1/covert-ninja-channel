#include "util.h"

/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: util.c - This source file holds utility functions, which are used by the client and/or server.
-- 
-- FUNCTIONS: 	void usage(char *program_name);
--		unsigned short in_cksum(unsigned short *ptr, int nbytes);
--		unsigned short tcp_in_cksum(unsigned int src, unsigned int dst, unsigned short *addr, int length);
--		unsigned int host_convert(char *hostname);
--		char * convert_ip_to_string(struct in_addr addr);
-- 
-- DATE: 2014/09/20
-- 
-- REVISIONS: (Date and Description)
-- 
-- DESIGNER: Luke Tao
-- 
-- PROGRAMMER: Luke Tao
-- 
-- NOTES: Used by the client and/or server, these functions are utilized by other functions.
----------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
-- FUNCTION: usage
-- 
-- DATE: 2014/09/20
-- 
-- REVISIONS: (Date and Description)
-- 
-- DESIGNER: Luke Tao
-- 
-- PROGRAMMER: Luke Tao
-- 
-- INTERFACE: void usage(char *program_name)
-- 
-- RETURNS: void.
-- 
-- NOTES: Prints how to use the covert channel program.
----------------------------------------------------------------------------------------------------------------------*/
void usage(char *program_name)
{
	printf("Usage: %s -dest dest_ip [-dest-port port] [-window-size window_size] -file filename [-server] \n\n", program_name);
	printf("-dest dest_ip 	  - Host to send data to.\n");
	printf(" 		    In SERVER mode this is the server IP address.\n");
	printf("-dest-port port   - IP destination port you want data to go to. In\n");
	printf(" 		    SERVER mode this is the port data will be coming\n");
	printf(" 		    inbound on.\n");
	printf(" 		    If NOT specified, default is port 8654.\n");
	printf("-window-size size - Window-size to send/receive (client and server MUST match window-size)\n");
	printf("		    If NOT specified, default is size 4068.\n");
	printf("-file filename 	  - Name of the file to encode and transfer.\n");
	printf("-server           - Server mode to allow receiving of data.\n");
        printf("                  - If NOT specified, client mode will run to allow sending of data.\n");
	exit(0);
}

/*--------------------------------------------------------------------------------------------------------------------
-- FUNCTION: in_cksum
-- 
-- DATE: 2014/09/20
-- 
-- REVISIONS: (Date and Description)
-- 
-- DESIGNER: Craig H. Rowland (from the TCP Covert Channel source code)
-- 
-- PROGRAMMER: Craig H. Rowland (from the TCP Covert Channel source code)
-- 
-- INTERFACE: unsigned short in_cksum(unsigned short *addr, int len)
-- 
-- RETURNS: The checksum for the IP header.
-- 
-- NOTES: Algorithm that makes the IP header checksum.
----------------------------------------------------------------------------------------------------------------------*/
unsigned short in_cksum(unsigned short *ptr, int nbytes)
{
	register long          sum;            /* assumes long == 32 bits */
        u_short                oddbyte;
        register u_short       answer;         /* assumes u_short == 16 bits */

        /*
         * Our algorithm is simple, using a 32-bit accumulator (sum),
         * we add sequential 16-bit words to it, and at the end, fold back
         * all the carry bits from the top 16 bits into the lower 16 bits.
         */

        sum = 0;
        while (nbytes > 1)  {
                sum += *ptr++;
                nbytes -= 2;
        }

        /* mop up an odd byte, if necessary */
        if (nbytes == 1) {
                oddbyte = 0; /* make sure top half is zero */
                *((u_char *) &oddbyte) = *(u_char *)ptr; /* one byte only */
                sum += oddbyte;
        }

        /*
         * Add back carry outs from top 16 bits to low 16 bits.
         */
        sum  = (sum >> 16) + (sum & 0xffff);    /* add high-16 to low-16 */
        sum += (sum >> 16);                     /* add carry */
        answer = ~sum;          /* ones-complement, then truncate to 16 bits */
        return(answer);
}

/*--------------------------------------------------------------------------------------------------------------------
-- FUNCTION: tcp_in_cksum
-- 
-- DATE: 2014/09/20
-- 
-- REVISIONS: (Date and Description)
-- 
-- DESIGNER: Murat Balaban (from http://www.enderunix.org/docs/en/rawipspoof/)
-- 
-- PROGRAMMER: Murat Balaban (http://www.enderunix.org/docs/en/rawipspoof/)
-- 
-- INTERFACE: unsigned short tcp_in_cksum(unsigned int src, unsigned int dst, unsigned short *addr, int length)
-- 
-- RETURNS: The checksum for the TCP header. 
-- 
-- NOTES: Algorithm that makes the TCP header checksum.
----------------------------------------------------------------------------------------------------------------------*/
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

/*--------------------------------------------------------------------------------------------------------------------
-- FUNCTION: host_convert
-- 
-- DATE: 2014/09/20
-- 
-- REVISIONS: (Date and Description)
-- 
-- DESIGNER: Luke Tao
-- 
-- PROGRAMMER: Luke Tao
-- 
-- INTERFACE: unsigned int host_convert(char *hostname)
-- 
-- RETURNS: The IP address of the given host name.
-- 
-- NOTES: Converts a host name to its IP address.
----------------------------------------------------------------------------------------------------------------------*/
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

/*--------------------------------------------------------------------------------------------------------------------
-- FUNCTION: convert_ip_to_string
-- 
-- DATE: 2014/09/20
-- 
-- REVISIONS: (Date and Description)
-- 
-- DESIGNER: Luke Tao
-- 
-- PROGRAMMER: Luke Tao
-- 
-- INTERFACE: char * convert_ip_to_string(struct in_addr addr)
-- 
-- RETURNS: A pointer to the converted ASCII data.
-- 
-- NOTES: This function takes in an IP address, converts it into a string, splits the string by dividing with the '.',
--	  decrypts it by subtracting 255 to each of the decimal ASCII-equivalent characters and then stores them into a
--	  dynamically-allocated array. Afterwards, it returns a pointer to that array.
----------------------------------------------------------------------------------------------------------------------*/
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
