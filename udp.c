#include "includes.h"
#include "types.h"
#include "nlp.h"
#include "udp.h"
#include "errno.h"

#include "garbler.h"




// the UDP server API
int udp_server(char *msg,int sd,char *from_node)
{
	unsigned int numBytes, cliLen;
	struct sockaddr_in cliAddr;

	memset(msg,0x0,MAX_NLP_DATA_SIZE+NLP_HEADER_SIZE);

	/* receive message */
	cliLen = sizeof(cliAddr);
	
	numBytes = recvfrom(sd, msg, MAX_NLP_DATA_SIZE+NLP_HEADER_SIZE, 0,(struct sockaddr *) &cliAddr, &cliLen);

	if(numBytes < 0)
	{
		return FAIL;
	}
	
	*from_node = (char)get_node_number(inet_ntoa(cliAddr.sin_addr));

	return numBytes;

}


// the UDP client API
int udp_client(char *ip_address,char *data,int length,int remote,int type,int sd)
{
	int rBytes = 0;

    // adjust the garbler here
    set_garbler(3,3,3); 
 


	
	if(type == ROUTING)	//for routing info
		rBytes = sendto(sd, data, length, 0,(struct sockaddr *) &control_remoteServAddr[remote],sizeof(control_remoteServAddr[remote]));
	else	//for regular data transfer
	{
		rBytes = sendto(sd, data, length, 0,(struct sockaddr *) &data_remoteServAddr[remote],sizeof(data_remoteServAddr[remote]));
	}

	
	if(rBytes < 0)
	{
		//printf("\nError in sendto() in udp.c");
		//close(sd);
		return FAIL;
	}

	return rBytes;
	
}


// do the socket initialization here
int initialize_sock()
{
	int i,rc;
	struct hostent *h;

	int no_of_neighbours = get_no_of_neighbours();

	for(i=0; i < no_of_neighbours; i++)
	{
		data_socket_desc[i] = socket(AF_INET, SOCK_DGRAM, 0);
		control_socket_desc[i] = socket(AF_INET, SOCK_DGRAM, 0);
		
		if(data_socket_desc[i] <0)
		{
			printf("\nSocket opening error...");
			return FAIL;

		}

		if(control_socket_desc[i] <0)
		{
			printf("\nSocket opening error...");
			return FAIL;
		}

		/* bind local data port */
		data_servAddr[i].sin_family = AF_INET;
		data_servAddr[i].sin_addr.s_addr = htonl(INADDR_ANY);
		data_servAddr[i].sin_port = htons(data_ports[i]);
		
		/* bind local control port */
		control_servAddr[i].sin_family = AF_INET;
		control_servAddr[i].sin_addr.s_addr = htonl(INADDR_ANY);
		control_servAddr[i].sin_port = htons(control_ports[i]);
		
		// bind the socket
		rc = bind (data_socket_desc[i], (struct sockaddr *) &data_servAddr[i],sizeof(data_servAddr[i]));
		if(rc<0)
		{
			printf("\nFailed to bind port number");
			return FAIL;
		}

		rc = bind (control_socket_desc[i], (struct sockaddr *) &control_servAddr[i],sizeof(control_servAddr[i]));
		if(rc<0)
		{
			printf("\nFailed to bind port number");
			return FAIL;
		}

		/* get server IP address (no check if input is IP address or DNS name */
		h = gethostbyname(ip_addrs[i]);
		if(h==NULL)
		{
			printf("unknown host  %s\n",ip_addrs[i]);
			
			return FAIL;
		}

		

		inet_ntoa(*(struct in_addr *)h->h_addr_list[0]);
		data_remoteServAddr[i].sin_family = h->h_addrtype;
		memcpy((char *) &data_remoteServAddr[i].sin_addr.s_addr,h->h_addr_list[0], h->h_length);
		data_remoteServAddr[i].sin_port = htons(data_ports[i]);

		inet_ntoa(*(struct in_addr *)h->h_addr_list[0]);
		control_remoteServAddr[i].sin_family = h->h_addrtype;
		memcpy((char *) &control_remoteServAddr[i].sin_addr.s_addr,h->h_addr_list[0], h->h_length);
		control_remoteServAddr[i].sin_port = htons(control_ports[i]);
	}
}




