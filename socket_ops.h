#ifndef SOCKET_OPS_H
#define SOCKET_OPS_H

#include "covert.h"

int create_raw_socket(int sock_proto);
int close_socket(int sock_d);
void sig_proc();

#endif
