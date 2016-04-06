

// this file contains the basic data types and Macro definitions



#define MAX_NODES 16
#define IP_ADDR_LEN 20
#define SUCCESS 1
#define FAIL -1
#define CONTINUE 222

#define _REENTRANT


#define FTP_HEADER_SIZE 1


#define TRUE 1
#define FALSE 0
#define BAD_FILE 9
#define BAD_CONFIG_FILE 55

#define PING 2
#define ROUTING 1
#define DATA 0
#define RETRANSMISSION_TIME 2 
#define ROUTING_TABLE_SIZE 255





// Basic data structures
/***********************************************************************************/
struct routing_table_entry
{
	char destination;
	char next_hop;
	int no_of_hops;
	char info_frm;
	int data_port;
	int control_port;
	char ip_addr[IP_ADDR_LEN];
	


};
/***********************************************************************************/

// the node structure
typedef struct {
	int id;
} node;

/***********************************************************************************/

typedef struct routing_table_entry RTABLE;
short int self_node, source_node;
RTABLE routing_table[ROUTING_TABLE_SIZE];

int EXIT;

int start;

int file_start;

int destination_node;

pthread_t thread_send_rt;
pthread_t thread_control[MAX_NODES];
pthread_t thread_data[MAX_NODES];
pthread_t thread_ui;
pthread_t thread_node_detect;


// displays the routing table info at a node
void display_routing_table(RTABLE *routing_table,int l,int node);


// prepares the FTP packet
int prep_ftp_packet(char dnumber,char *data,int size,char *ftp);











