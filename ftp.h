#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h> 
#include <string.h> 
#include <time.h>

char *data_ptr;
int file_length;


/*
* @function	: write_to_file()
* @brief	: write data to file at destination
*/
int write_to_file(char *ptr,int length);

/*
* @function	: read_from_file()
* @brief	: Read the source file data
*/
int read_from_file(char *file);

/*
* @function	: user_interface_ftp()
* @brief	: Handles the FTP layer UI
*/
void *user_interface_ftp();

// show the menu
void showmenu();

