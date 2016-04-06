#include "pthread.h"
#include "includes.h"
#include "types.h"
#include "udp.h"
#include "nlp.h"
#include "tlp.h"

// new changes


struct node_status
{
	char node_id;
	int status;
	int hopCount;
	time_t t1;
};

typedef struct node_status NODE_STAT;
int INACTIVE_NODE_FLAG = FALSE;
// for routing table
pthread_mutex_t table_lock;

NODE_STAT node_status_table[MAX_NODES];
// new changes

// work under progress
// new changes
/*
* @function	: dead_node_detector()
* @brief	: This is a daemon which detects whether a particular node is dead or not
* @param	: none
* @return	: none
*/
void* dead_node_detector()
{	
	int offset = 0, x = 0;
	pthread_mutex_t lock;
	// check for dead node timer expiry here
	int links = get_no_of_links(routing_table);
	time_t t1, t2, timer2;

	// initialize the first timer
	time(&t1);

	while(1)
	{
		time(&t1);
		time(&t2);
		while((t2 - t1) < 20)
		{
			time(&t2);
		}
		// enter critical section
		//pthread_mutex_lock(&lock);
		for(offset = 0; offset < MAX_NODES; offset++)
		{
			//printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n checking for dead node\n");
			if(node_status_table[offset].node_id != -1)
			{
				time(&timer2);
				if(timer2 - node_status_table[offset].t1 > DEAD_NODE_TIMER)
				{
					int num_links = get_no_of_links(routing_table);
					// now search that node in the routing table and mark it as dead
					for(x = 0; x < num_links; x++)
					{
						//if((routing_table[x].next_hop == node_status_table[offset].node_id) || 
						if((routing_table[x].info_frm == node_status_table[offset].node_id) ||(routing_table[x].next_hop == node_status_table[offset].node_id) || (routing_table[x].destination == node_status_table[offset].node_id) )
						{
							// save the old hopCount first
							int numHops = routing_table[x].no_of_hops;
							if(numHops != 255)
							node_status_table[offset].hopCount = numHops;
							// match is found, now mark the node as dead
							if(numHops != 255)
							{
								routing_table[x].no_of_hops = 255;
							}
							node_status_table[offset].status = FALSE;
							// again reset the timer for the dead node
							time(&timer2);
							node_status_table[offset].t1 = timer2;

						}
						
					}
				}
			}
		}
		//pthread_mutex_unlock(&lock);
	}
}
// new changes


//returns the source node
int get_src_node()
{
	return self_node;
}


// get the ID of the next hop
int get_next_hop_id(char dest_node)
{
	int tot_links, index = 0;
	int i , neighborCount;
	// get the total no of links of a node
	tot_links = get_no_of_links(routing_table);


	
	for(index = 0; index < tot_links; index++)
	{
		
		if(dest_node == routing_table[index].destination)
		{
			neighborCount = get_no_of_neighbours();
			for(i = 0; i < neighborCount; i++)
			{
				
				if(neighbours[i] == routing_table[index].next_hop)
					return i;
			}
		}
	}

	return -1;
}



// this API initializes the data and control ports
void initialize_ports()
{
	int i, index = 0;
	int l=0;
	int tot = get_no_of_links(routing_table);
	pthread_mutex_t lock;
	
	for(i=0;i<tot;i++)
	{
		if(routing_table[i].destination == routing_table[i].next_hop)
		{
			// initialize ports for data only
			data_ports[l] = routing_table[i].data_port;
			// initialize control ports for routing information
			control_ports[l] = routing_table[i].control_port;
			
			memcpy(ip_addrs[l],routing_table[i].ip_addr,strlen(routing_table[i].ip_addr));
			neighbours[l++] = routing_table[i].destination;
			// new changes
			// enter critical section
			//pthread_mutex_lock(&lock);
			node_status_table[index].node_id = routing_table[i].destination;
			node_status_table[index].status = FALSE;
			node_status_table[index].t1 = (time_t)(NULL);
			node_status_table[index].hopCount = 0;
			// leave critical section
			//pthread_mutex_unlock(&lock);
			index++;
			// new changes
		}
	}
	

}



// initializes the NLP layer
void initialize_nlp()
{
	int i;

	node *data_p = (node *)malloc(sizeof(node)*MAX_NODES);
	node *control_p = (node *)malloc(sizeof(node)*MAX_NODES);

	int no_of_neighbours = get_no_of_neighbours();
	
	initialize_ports();
	initialize_sock();

	pthread_create(&thread_send_rt,NULL,&nlp_send_rt,(void *)NULL);

	for(i=0;i<no_of_neighbours;i++)
	{
		data_p[i].id = data_socket_desc[i];
		control_p[i].id = control_socket_desc[i];
		

		
		pthread_create(&thread_data[i],NULL,nlp_receive_data,(void *)(data_p+i));
		pthread_create(&thread_control[i],NULL,&nlp_receive_rt,(void *)(control_p+i));
	}

}


// prepare the NLP packet for sending
int prep_nlp_packet(char snode,char dnode,char cont_data,char *data,char *nlp,int length)
{
	int i=0;
	
	short int no_of_links;
	
	short int data_length;
	
	short int tot_length;
	char checksum;
	char overflow;
	char temp;

	if(cont_data == ROUTING)
	{
		 no_of_links = get_no_of_links((RTABLE *)data);
		 

		 data_length = no_of_links*sizeof(struct routing_table_entry);
		
	}
	else
	{
		data_length = length;		
	}

	tot_length = data_length + NLP_HEADER_SIZE;

	nlp[i++] = (snode << 4) | dnode;
	nlp[i++] = (cont_data << 7) | 0x0 | (tot_length && 0x3ff) >> 8;
	nlp[i++] = (char)(tot_length && 0x00ff);
	nlp[i++] = 0x0; //checksum

	for(i=0;i<data_length;i++)
		nlp[i+NLP_HEADER_SIZE] = data[i];

	checksum = 0x0;

	for(i=0;i<NLP_HEADER_SIZE;i++) // Checksum for header only
	{
		temp = checksum + nlp[i];

		overflow = (temp < checksum) ? 1:0;

		if(!overflow)
			overflow = (temp < nlp[i]) ? 1:0;

		checksum += nlp[i];
		checksum += overflow;
	}

	nlp[3] =(char) (~checksum);
	nlp[tot_length++] = '\0';
	
	return (tot_length);
}



// return the number of neighbours
int get_no_of_neighbours()
{
	int i;
	int num=0;
	int tot = get_no_of_links(routing_table);

	for(i=0;i<tot;i++)
	{
		if(routing_table[i].next_hop == routing_table[i].destination && routing_table[i].next_hop != -1)
			num++;
	}
	
	return num;
}



// sends the NLP data to the destination node
void nlp_send_data(char dest_node,char *data,int data_type,int length)
{
	int i=0;
	char next_hop;
	char dest_addr[16];
	int dest_port;
	int end;
	char *nlp;

	//printf("\n\n\n\n\n\n\n\n\nEntering....nlp_send_data()\n");
	nlp = (char *)malloc(MAX_NLP_DATA_SIZE+NLP_HEADER_SIZE);

	if(data_type == NLP_DATA_ONLY)
	{
		end = prep_nlp_packet(self_node,dest_node,DATA,(char *)data,nlp,length);
		end--;
		//free(nlp);
	}
	else
	{
		nlp = data;
		end = length;
		//i = get_next_hop_id(dest_node);
	
		//udp_client(ip_addrs[i],nlp,end,i,0,data_socket_desc[i]);
	}

	
	

	i = get_next_hop_id(dest_node);
	
	udp_client(ip_addrs[i],nlp,end,i,0,data_socket_desc[i]);
	//printf("\n\n\n\n\n\n\n\n\nExiting....nlp_send_data()\n");


	//free(nlp);
}



// computes NLP header checksum
int nlp_checksum(char *nlp)
{
	int i=0;
	char checksum=0;
	char temp;
	char overflow;

        for(i=0; i<NLP_HEADER_SIZE; i++) // Checksum for header only
        {
                temp = checksum + (nlp[i] & 0xff);
		
                overflow = (temp < checksum) ? 1:0;

                if(!overflow)
                        overflow = (temp < (nlp[i]&0xff)) ? 1:0;

                checksum += (nlp[i]&0xff);

		if(i != NLP_HEADER_SIZE-1)
                	checksum += overflow;

        }
     	
        if((checksum & 0xff) == 0xff)
        	return SUCCESS;
        else
        	return FAIL;
}


// does the processing of the regular NLP data
void *nlp_receive_data(void *p)
{
	int status=0;
	int i=0;
	int end;
	node *f = (node *)p;
	int sd=f->id;

	char neighbours[16];
	char *nlp;

	short int node;

	//printf("\n\n\n\n\n\n\n\n\nEntering....nlp_receive_data()\n");

	while(1)
	{
		char his_node;
		char dest_node;

		nlp = (char *)malloc(MAX_NLP_DATA_SIZE+NLP_HEADER_SIZE);
		
		end = udp_server(nlp,sd,&his_node);
		
		// check for NLP checksum
		if(nlp_checksum(nlp) == FAIL)
		{
			
			continue;
		}
		

		dest_node = (char)(nlp[0] & 0x0f);
		
		// if dest node is reached, TLP layer will take over
		if(self_node == dest_node)
		{
			tlp_receive_packet((nlp[0]>>4),nlp+4,end-4);
		}
		else
		{
			nlp_send_data(dest_node,nlp,NLP_PACKET,end);
		}
		//free(nlp);

		//printf("\n\n\n\n\n\n\n\n\nExiting....nlp_receive_data()\n");
	}

}


// does the processing after new routing info arrives
void *nlp_receive_rt(void *p)
{
	//time_t t1,t2;
	int status=0;
	int i=0, offset = 0;
	int end;
	node *f = (node *)p;
	int sd = f->id;

	char neighbours[16];
	char *nlp;

	short int node;
	static int index = 0, updated = FALSE;
	// the second timer
	time_t timer2;

	RTABLE *recv_rt;
	// new changes
	pthread_mutex_t lock;
	// new changes

	//printf("\n\n\n\n\n\n\n\n\nEntering....nlp_receive_rt()\n");


	nlp = (char *)malloc(NLP_HEADER_SIZE+(MAX_NODES*sizeof(struct routing_table_entry)));

	while(1)
	{
		char his_node;
		memset(nlp,0x0,NLP_HEADER_SIZE+(MAX_NODES*sizeof(struct routing_table_entry)));

		
		end = udp_server(nlp,sd,&his_node);

		
		recv_rt = (RTABLE *)(&nlp[NLP_HEADER_SIZE]);
		end -= 4;
		
		// lock resource for preventing data inconsistency
		pthread_mutex_lock(&table_lock);
		status = compute_routing_table(routing_table,recv_rt,his_node,self_node,(end/sizeof(struct routing_table_entry)));
		pthread_mutex_unlock(&table_lock);
		// unlock resource when done

		// new changes
		
		pthread_mutex_lock(&lock);

		// search for that node in the node_status_table
		for(offset = 0; offset < MAX_NODES; offset++)
		{
			if(node_status_table[offset].node_id == his_node)
			{
				node_status_table[offset].node_id = his_node;
				node_status_table[offset].status  = TRUE;
				// initialize the second timer
				time(&timer2);
				node_status_table[offset].t1 = timer2;
				updated = TRUE;
				//printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n in block 2...match found\n");
				break;
			}
		}
	
		if(updated != TRUE)
		{
			node_status_table[index].node_id = his_node;
			node_status_table[index].status  = TRUE;
			// initialize the second timer
			time(&timer2);
			node_status_table[index].t1 = timer2;
			index++;
			//printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n in block 1\n");
		}
			
		
		// now search the node ID which sent the update into the node status table
		// this will make sure that the old hop count is restored programatically
		
		for(offset = 0; offset < MAX_NODES; offset++)
		{
			if(his_node == node_status_table[offset].node_id)
			{
				int links = get_no_of_links(routing_table);
				int loop = 0;
				for(loop = 0; loop < links; loop++)
				{
					if(routing_table[loop].next_hop == his_node)
					{
						int hops = node_status_table[offset].hopCount;
						if(routing_table[loop].no_of_hops != hops && hops != 0)
						{
							//restore old hop count
							routing_table[loop].no_of_hops = hops;
						}
					}
				}
			}
		}

		
	

		pthread_mutex_unlock(&lock);	// leave critical section
		// new changes
		
		
		if(TRUE == EXIT)
			break;
	}

	//printf("\n\n\n\n\n\n\n\n\nExiting....nlp_receive_rt()\n");

	//free(nlp);
}



void *nlp_send_rt()
{
	time_t t1,t2;
	int i;
	int end;
	char *nlp;

	//printf("\n\n\n\n\n\n\n\n\nEntering....nlp_send_rt()\n");

	int no_of_links = get_no_of_neighbours();
	
	nlp = (char *)malloc(NLP_HEADER_SIZE+(MAX_NODES*sizeof(struct routing_table_entry)));
	//printf("\n\n\n\n\n\n\n\n\nMemory allocated for nlp in....nlp_send_rt()\n");

	while(1)
	{
		time(&t1);
		time(&t2);

		while((t2-t1) < RETRANSMISSION_TIME)
		{
			time(&t2);
		}
		
		
		

		for(i=0;i<no_of_links;i++)
		{	
			memset(nlp,0x0,NLP_HEADER_SIZE+(MAX_NODES*sizeof(struct routing_table_entry)));
			end = prep_nlp_packet(self_node,neighbours[i],ROUTING,(char *)routing_table,nlp,1);
			udp_client(ip_addrs[i],nlp,end,i,1,control_socket_desc[i]);
		}



		if(TRUE == EXIT)
			break;

	}

	//printf("\n\n\n\n\n\n\n\n\nExiting....nlp_send_rt()\n");

	//free(nlp);
}




// create the routing table from the config file
int populate_rt(char *file)
{
	FILE *in_conf;
	char data[MAX_NODES][MAX_NODES];
	int no_of_links;
	int nodes_list[MAX_NODES];
	int	data_port[MAX_NODES];
	int	control_port[MAX_NODES];
	char addresses[MAX_NODES][20];
	int end=0;
	int i;

	//printf("\n\n\n\n\n\n\n\n\n\nEntering....populate_rt()\n");

	initialize_rt(routing_table);

	in_conf=fopen(file,"r");
	if(NULL == in_conf)
	{
		return BAD_FILE;
	}

	end = 0;

	while(fscanf(in_conf,"%s",data[end]) != EOF)
		end++;

	self_node = atoi(data[0]);
	no_of_links = atoi(data[1]);

	i=0;
	while(end > 2)
	{
		data_port[i] = atoi(data[--end]);
		control_port[i] = atoi(data[--end]);
		memcpy(addresses[i],data[end-1],strlen(data[end-1]));
		addresses[i][(strlen(data[end-1]))] = '\0';
		end--;
		nodes_list[i++] = atoi(data[--end]);
	}

	if(i != no_of_links)
	{
		return BAD_CONFIG_FILE;
	}

	//pthread_mutex_lock(&mut_lock);
	for(i=0;i<no_of_links;i++)
	{
		routing_table[i].destination = nodes_list[i];
		routing_table[i].next_hop = nodes_list[i];
		routing_table[i].no_of_hops = 1;
		routing_table[i].info_frm = self_node;
		routing_table[i].data_port = data_port[i];
		routing_table[i].control_port = control_port[i];
		memcpy(routing_table[i].ip_addr,addresses[i],strlen(addresses[i]));
		printf("creating the routing table from the config file....\n");
	}
	//pthread_mutex_unlock(&mut_lock);
	//printf("\n\n\n\n\n\n\n\n\nExiting....populate_rt()\n");

	fclose(in_conf);
}


// this API updates the routing table when new info comes
int update_routing_table(char destination,char info_frm,char next_hop,int no_of_hops,RTABLE *routing_table)
{
	int i, offset = 0, inactiveHop, index = 0;
	int updated = FALSE;
	//pthread_mutex_t mut;

	
	// new changes for dead node
	time_t t;
	// new changes for dead node


	//printf("\n\n\n\n\n\n\n\n\nEntering....update_routing_table()\n");
	//pthread_mutex_lock(&mut);
	int no_of_links = get_no_of_links(routing_table);

	for(i=0;i<no_of_links;i++)
	{
		if((routing_table[i].destination == destination) && (routing_table[i].info_frm == info_frm) && (routing_table[i].next_hop == next_hop) && (routing_table[i].no_of_hops == routing_table[i].no_of_hops))
		{	
			routing_table[i].next_hop = info_frm;
			routing_table[i].no_of_hops = no_of_hops;
			routing_table[i].info_frm = info_frm;
			
			/*
			// new changes for dead node
			time(&t);
			routing_table[i].timer = t;
			// new changes for dead node
			*/


			updated = TRUE;
			break;
		} 
	}

	if(TRUE != updated)
	{
		int found = 0,
		    savedIndex = 0;
		for(i = 0; i < no_of_links; i++)
		{
			if(destination == routing_table[i].destination)
			{
				if(no_of_hops < routing_table[i].no_of_hops)
				{
					found = 1;
					savedIndex = i;
					break;
				}
			}
		}

		if(found == 1)
		{
			routing_table[savedIndex].destination = destination;
			routing_table[savedIndex].next_hop = next_hop;
			routing_table[savedIndex].no_of_hops = no_of_hops;
			routing_table[savedIndex].info_frm = info_frm;
		}
		else
		{
			if(no_of_hops < 255)
			{
				routing_table[no_of_links].destination = destination;
				routing_table[no_of_links].next_hop = next_hop;
				routing_table[no_of_links].no_of_hops = no_of_hops;
				routing_table[no_of_links].info_frm = info_frm;
			}
		}

		/*
		routing_table[no_of_links].destination = destination;
		routing_table[no_of_links].next_hop = next_hop;
		routing_table[no_of_links].no_of_hops = no_of_hops;
		routing_table[no_of_links].info_frm = info_frm;
		*/
		/*
		// new changes for dead node
		time(&t);
		routing_table[i].timer = t;
		// new changes for dead node
		*/

		no_of_links++;
	}
	//pthread_mutex_unlock(&mut);
	//printf("\n\n\n\n\n\n\n\n\nExiting....update_routing_table()\n");
	return no_of_links;
}


// this API returns the ID of the next hop based on dest node
char get_next_hop(char dest_node,int *dest_port,char *addr)
{
	int i=0;
	while(routing_table[i].no_of_hops != -1)
	{
		if(routing_table[i].destination == dest_node)
		{
			*dest_port = routing_table[i].data_port;
			memcpy(addr,routing_table[i].ip_addr,strlen(routing_table[i].ip_addr));;
			return (routing_table[i].next_hop);
		}
		i++;
	}

	return -1;
}


// displays the routing table when needed
void display_routing_table(RTABLE *routing_table,int links,int node)
{
	int i;
	int no_of_links;

	if(links == -1)
		no_of_links = get_no_of_links(routing_table);
	else
		no_of_links = links;

	
	// clear the screen
	fprintf(stdout, "\033[2J");
	fprintf(stdout, "\033[1;1H");

	printf("\nRouting table of NODE %d\n",node);
		
	printf("\nDesti\tNextHop\tHops\tDatFrom\tIPAddress\tdataP\tcontP");
		

	for(i=0;i<no_of_links;i++)
		printf("\n%d\t%d\t%d\t%d\t%s\t%d\t%d",routing_table[i].destination,routing_table[i].next_hop,routing_table[i].no_of_hops,routing_table[i].info_frm,routing_table[i].ip_addr,routing_table[i].data_port,routing_table[i].control_port);

	printf("\n\nThis was the routing table for the current node\n");
	printf("\n\nPlease enter some input to continue\n\n");
	

}



void initialize_rt(RTABLE *rt)
{
	int i = 0, index = 0;
	// new changes
	pthread_mutex_t lock;
	// new changes
	//printf("\n\n\n\n\n\n\n\n\nEntering....initialize_rt()\n");
	for(i=0;i<255;i++)
	{
		rt[i].destination = -1;
		rt[i].next_hop = -1;
		rt[i].no_of_hops = -1;
		rt[i].info_frm = -1;
	}

	// new changes
	//pthread_mutex_lock(&lock);
	for(index = 0; index < MAX_NODES; index++)
	{
		node_status_table[index].node_id = -1;
		node_status_table[index].status = FALSE;	
		node_status_table[index].hopCount = 0;
		node_status_table[index].t1 = (time_t)(NULL);
	}
	//pthread_mutex_unlock(&lock);
	//printf("\n\n\n\n\n\n\n\n\nExiting....initialize_rt()\n");
	// new changes
}





int get_no_of_links(RTABLE *rt)
{
	int i=0;
	char *r;

	while(rt[i].no_of_hops != -1)
	{
		i++;
	}


	return i;
}



// this API completes the routing table when the new routing info arrives
int compute_routing_table(RTABLE *my_rt,RTABLE *his_rt,char his_node,char my_node,int links)
{
	
	int i,j;
	int my_no_of_links;
	//pthread_mutex_t lock, mut_lock;

	my_no_of_links=get_no_of_links(my_rt);
	
	//printf("\n\n\n\n\n\n\n\n\nEntering....compute_routing_table()\n");


	for(i=0;i<links;i++)
	
	{
		j=0;
		while(my_rt[j].no_of_hops != -1)
		{
			if((my_rt[j].destination == his_rt[i].destination) || (his_rt[i].info_frm == my_node) || (his_rt[i].destination == my_node))	
				break;

			j++;
		};

		if(his_rt[i].info_frm == my_node || his_rt[i].destination == my_node)
			continue;


		else if((my_rt[j].no_of_hops == -1) || ((his_rt[i].no_of_hops + 1) < my_rt[j].no_of_hops))
		{
			//pthread_mutex_lock(&lock);
			my_no_of_links = update_routing_table(his_rt[i].destination,his_node,his_node,1+his_rt[i].no_of_hops,my_rt);
			//pthread_mutex_unlock(&lock);
		}		
		
		// new changes for dead node
		if(his_rt[i].no_of_hops >= 255)
		{
			//pthread_mutex_lock(&mut_lock);
			my_no_of_links = update_routing_table(his_rt[i].destination,his_node,his_node,1+his_rt[i].no_of_hops,my_rt);
			//pthread_mutex_unlock(&mut_lock);
		}
		// new changes for dead node

	}

	//printf("\n\n\n\n\n\n\n\n\nExiting....compute_routing_table()\n");

	return(my_no_of_links);
}



// this API simply returns the number of neighbours of a node
int get_neighbours(char *n,int ports[],char addr[][20])
{
	int no_of_links = get_no_of_links(routing_table);
	int i;
	int t=0;
	int length;

	for(i=0;i<no_of_links;i++)
	{

		if(routing_table[i].destination == routing_table[i].next_hop)
		{
			length = strlen(routing_table[i].ip_addr);
			ports[t] = routing_table[i].control_port;
			
			memcpy(addr[t],routing_table[i].ip_addr,strlen(routing_table[i].ip_addr));
			
			n[t++] = routing_table[i].destination;
		}
	}

	return t;
}



// this API returns the node number corresponding to the IP address passed
char get_node_number(char *addr)
{	
	int i;
	int no_of_links = get_no_of_links(routing_table);

	for(i=0;i<no_of_links;i++)
	{
		
		if(strcmp(routing_table[i].ip_addr,addr) == 0)
		{	
			return (routing_table[i].next_hop);
		}
	}

	
	return -1;
}










