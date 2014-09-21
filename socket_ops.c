#include "socket_ops.h"

/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: socket_ops.c - This source file holds functions for socket operations.
-- 
-- FUNCTIONS: 	int create_raw_socket(int sock_proto);
--		int close_socket(int sock_d);
--		void sig_proc();
-- 
-- DATE: 2014/09/20
-- 
-- REVISIONS: (Date and Description)
-- 
-- DESIGNER: Luke Tao
-- 
-- PROGRAMMER: Luke Tao
-- 
-- NOTES: These 3 functions perform socket-related operations, used by the client and/or server.
----------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
-- FUNCTION: create_raw_socket
-- 
-- DATE: 2014/09/20
-- 
-- REVISIONS: (Date and Description)
-- 
-- DESIGNER: Luke Tao
-- 
-- PROGRAMMER: Luke Tao
-- 
-- INTERFACE: int create_raw_socket(int sock_proto)
-- 
-- RETURNS: A sending or receiving file socket descriptor
-- 
-- NOTES: This function simply creates a sending or receiving socket, depending on what the socket protocol is being
--	  passed in as.
----------------------------------------------------------------------------------------------------------------------*/
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

/*--------------------------------------------------------------------------------------------------------------------
-- FUNCTION: close_socket
-- 
-- DATE: 2014/09/20
-- 
-- REVISIONS: (Date and Description)
-- 
-- DESIGNER: Luke Tao
-- 
-- PROGRAMMER: Luke Tao
-- 
-- INTERFACE: int close_socket(int sock_d)
-- 
-- RETURNS: 0 on successful closing socket
-- 
-- NOTES: This wrapper function simply closes the socket passed in. 
----------------------------------------------------------------------------------------------------------------------*/
int close_socket(int sock_d)
{
	close(sock_d);
	return 0;
}

/*--------------------------------------------------------------------------------------------------------------------
-- FUNCTION: sig_proc
-- 
-- DATE: 2014/09/20
-- 
-- REVISIONS: (Date and Description)
-- 
-- DESIGNER: Luke Tao
-- 
-- PROGRAMMER: Luke Tao
-- 
-- INTERFACE: void sig_proc()
-- 
-- RETURNS: void
-- 
-- NOTES: When a signal interrupt (Ctrl-C) is triggered on the server side, this function handles it by closing
--	  the receiving socket. 
----------------------------------------------------------------------------------------------------------------------*/
void sig_proc()
{
	close_socket(recv_sock);
	printf("\nServer Socket closed.\n");
	exit(0);
}
