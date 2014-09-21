#ifndef UTIL_H
#define UTIL_H

#include "covert.h"

void usage(char *program_name);
unsigned short in_cksum(unsigned short *addr, int len);
unsigned short tcp_in_cksum(unsigned int src, unsigned int dst, unsigned short *addr, int length);
unsigned int host_convert(char *hostname);
char * convert_ip_to_string(struct in_addr addr);

#endif
