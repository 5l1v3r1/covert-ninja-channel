#ifndef CLIENT_OPS_H
#define CLIENT_OPS_H

#include "covert.h"
#include "socket_ops.h"
#include "util.h"

int client_file_io();
void forge_packet_client(struct in_addr addr, unsigned int forged_src_port);

#endif
