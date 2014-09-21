#include "covert.h"
#include "client_ops.h"
#include "server_ops.h"
#include "util.h"
#include "socket_ops.h"

/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: covert_main.c - This source file holds the main functions of the covert program.
-- 
-- FUNCTIONS: 	int main(int argc, char **argv);
--		void start_covert_channel();
-- 
-- DATE: 2014/09/20
-- 
-- REVISIONS: (Date and Description)
-- 
-- DESIGNER: Luke Tao
-- 
-- PROGRAMMER: Luke Tao
-- 
-- NOTES: These 2 functions are the starting functions for the client or the server. 
----------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
-- FUNCTION: main
-- 
-- DATE: 2014/09/20
-- 
-- REVISIONS: (Date and Description)
-- 
-- DESIGNER: Luke Tao
-- 
-- PROGRAMMER: Luke Tao
-- 
-- INTERFACE: int main(int argc, char **argv)
-- 
-- RETURNS: 0 on successful exit
-- 
-- NOTES: This main function masks the process name, parses command-line arguments, and then starts the covert
--	  channel program either by client or server mode.
----------------------------------------------------------------------------------------------------------------------*/
int main(int argc, char **argv)
{
	/* mask the process name */
	memset(argv[0], 0, strlen(argv[0]));	
	strcpy(argv[0], MASK);
	prctl(PR_SET_NAME, MASK, 0, 0);

 	channel_info.dest_host = UNREAD;
	channel_info.w_size = WINDOW_SIZE;
	channel_info.dest_port = DEST_PORT;
	channel_info.server = UNREAD;
	size_t i = 0;

	if (geteuid() != USER_ROOT)
	{
		printf("\nYou need to be root to run this.\n\n");
    		exit(0);
	}

	if (argc < 5 || argc > 10)
	{
		usage(argv[0]);
		exit(0);
	}

	for(i = 1; i < argc; i++)
	{
		if (strcmp(argv[i],"-dest") == 0)
		{
			channel_info.dest_host = host_convert(argv[i+1]);
			strncpy(channel_info.desthost, argv[i+1], BUFFER_SIZE - 1);
		}
		else if (strcmp(argv[i],"-dest-port") == 0)
			channel_info.dest_port = atoi(argv[i+1]);
		else if (strcmp(argv[i],"-window-size") == 0)
			channel_info.w_size = atoi(argv[i+1]);
		else if (strcmp(argv[i],"-file") == 0)
			strncpy(channel_info.filename, argv[i+1], BUFFER_SIZE - 1);
		else if (strcmp(argv[i],"-server") == 0)
			channel_info.server = READ;
	}

	start_covert_channel();

	return 0;
}

/*--------------------------------------------------------------------------------------------------------------------
-- FUNCTION: start_covert_channel
-- 
-- DATE: 2014/09/20
-- 
-- REVISIONS: (Date and Description)
-- 
-- DESIGNER: Luke Tao
-- 
-- PROGRAMMER: Luke Tao
-- 
-- INTERFACE: void start_covert_channel()
-- 
-- RETURNS: void.
-- 
-- NOTES: This function starts the main server or client function, depending on the user's choice.
----------------------------------------------------------------------------------------------------------------------*/
void start_covert_channel()
{	
	if(channel_info.server == READ)
		decrypt_packet_server();
	else
		client_file_io();
}





