covert-ninja-channel
====================

To compile program: run "make"

To cleanup *.o and covert.exe files: run "make clean"

	To use the program:
	
	./covert -dest dest_ip [-dest-port port] [-window-size size] -file filename [-server]
	
	-dest <dest_ip> 	- Host to send data to. 
					  	  In SERVER mode this is the server ip address.
	
	-dest-port <port>   - IP destination port you want data to go to. 
			    		  In SERVER mode this is the port data will be coming inbound on.
			    		  If NOT specified, default is port 8654.
	
	-window-size <size> - Window-size to send/receive (client and server MUST match window-size)
			      		  If NOT specified, default is size 4068.
	
	-file <filename>    - Name of the file to encode and transfer.
	
	-server 			- Server mode to allow receiving of data.
		  				  If NOT specified, client mode will run to allow sending of data.
