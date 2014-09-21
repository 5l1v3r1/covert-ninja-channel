#ifndef SERVER_OPS_H
#define SERVER_OPS_H

#include "covert.h"
#include "socket_ops.h"
#include "util.h"

void decrypt_packet_server();
int server_file_io(char* recv_buffer);

#endif
