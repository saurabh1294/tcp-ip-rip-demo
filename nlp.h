#define MAX_NODES 16
#define NLP_DATA_ONLY 3
#define NLP_PACKET 6
#define NLP_HEADER_SIZE 4
#define MAX_NLP_DATA_SIZE 1020
#define DEAD_NODE_TIMER 10 
#define PING_DATA_SIZE 1

// new changes

typedef struct ping_packet
{
	char source_node;
	char dest_node;
	int  cont_type : 2;
	char ping_stat : 1;
	int  length;
}PING_TTL;

int PING_ACK;


// data structures used by the NLP layer
/********************************************************************************/
int data_ports[MAX_NODES];
int control_ports[MAX_NODES];
char ip_addrs[MAX_NODES][20];
char neighbours[MAX_NODES];
int data_socket_desc[MAX_NODES];
int control_socket_desc[MAX_NODES];
/********************************************************************************/


/********************************************************************************/
struct sockaddr_in data_servAddr[MAX_NODES];
struct sockaddr_in control_servAddr[MAX_NODES];

struct sockaddr_in data_remoteServAddr[MAX_NODES];
struct sockaddr_in control_remoteServAddr[MAX_NODES];
/********************************************************************************/

// work under progress

/*
* @function	: dead_node_detector()
* @brief	: Detects whether a particular node is dead or not
* @param	: none
* @return	: none
*/
void* dead_node_detector();




/*
* @function : get_next_hop_id()
* @brief	: Returns the next hop ID based on the destination node
* @param	: The ID of the destination node
* @return	: The ID of the next hop
*/
int get_next_hop_id(char dest_node);


/*
* @function	: get_no_of_neighbours()
* @brief	: returns the number of neighbours of a node
*/
int get_no_of_neighbours();

/*
* @function	: update_routing_table()
* @brief	: updates the routing table when new routing info comes
*/
int update_routing_table(char destination,char info_frm,char next_hop,int no_of_hops,RTABLE *routing_table);

/*
* @function	: compute_routing_table()
* @brief	: computes the new routing table as updates come
*/
int compute_routing_table(RTABLE *my_rt,RTABLE *his_rt,char his_node,char my_node,int l);


/*
* @function : prep_nlp_packet()
* @brief	: Prepare the NLP packet
*/
int prep_nlp_packet(char snode,char dnode,char cont_data,char *data,char *nlp,int length);


/*
* @function	: initialize_nlp()
* @brief	: Initializes the NLP layer	
*/
void initialize_nlp();


/*
* @function	: nlp_send_rt()
* @brief	: Broadcasts the routing table
*/
void *nlp_send_rt();

/*
* @function	: nlp_receive_rt()
* @brief	: handle the receiving of the routing table
*/
void *nlp_receive_rt();


/*
* @function	: nlp_send_data()
* @brief	: send the NLP data
*/
void nlp_send_data(char dest_node,char *data,int data_type,int length);

/*
* @function	: nlp_receive_data()
* @brief	: handle the receiving of the NLP data
*/
void *nlp_receive_data(void *port);


/*
* @function	: get_next_hop()
* @brief	: get the next hop
*/
char get_next_hop(char dest_node,int *dest_port,char *addr);

/*
* @function	: get_node_number()
* @brief	: Returns the node ID from the IP
*/
char get_node_number(char *addr);

/*
* @function	: get_neighbours()
* @brief	: Returns the number of neighbours of a node
*/
int get_neighbours(char *n,int ports[],char addr[][20]);


/*
* @function	: get_no_of_links()
* @breif	: Get the number of links
*/
int get_no_of_links(RTABLE *rt);


/*
* @function : initialize_rt()
* @brief	: initialize the routing table
*/
void initialize_rt(RTABLE *rt);

/*
* @function : get_src_node()
* @brief	: Get the source node
* @return	: the ID of the source node
*/
int get_src_node();

/*
* @function : populate_rt()
* @brief	: fills the routing table for the first time from the config file
*/
int populate_rt(char *file);




