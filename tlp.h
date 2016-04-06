#define TLP_HEADER_SIZE 6
#define MAX_TLP_DATA_SIZE 1014
#define DISCONNECT 66
#define RECEIVER 333
#define MAX_NO_OF_PACKETS 128


// basic data types
int tlp_dest_node;
int ACK;
unsigned char next_packet_number;
int sender;

#define TIME 5
#define RETRANSMISSION_TIME_REQ 5






/*
* @function : prep_tlp_packet()
* @brief	: make the TLP packet
*
*/
int prep_tlp_packet(unsigned char snumber,unsigned char anumber,char ack,char *data,char *tlp,int data_length);

/*
* @function : tlp_send_data()
* @brief	: send the TLP data
*
*/
int tlp_send_data(char dest_node,char *ftp_packet,int size);


/*
* @function : tlp_receive_packet()
* @brief	: handle the receiving of the TLP data
*
*
*/
void tlp_receive_packet(char source_node,char *tlp_packet,int length);


/*
* @function	: user_interface_tlp()
* @brief	: handle the UI of the TLP layer
*
*/
void *user_interface_tlp();


/*
* @function	: tlp_checksum()
* @brief	: validates TLP checksum
*
*
*/
int tlp_checksum(char *tlp,int length);

/*
* @function : initialize_tlp()
* @brief	: initialize the TLP layer
*/
int initialize_tlp(char *file);


/*
* @function	: tlp_connect()
* @brief	: check for TLP service
*
*/ 
int tlp_connect(char node,char dest_service_number[]);

