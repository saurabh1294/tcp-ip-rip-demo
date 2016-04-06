
/*
* @function : initialize_sock()
* @brief	: initialize the socket here
*/
int initialize_sock();

/*
* @function : udp_client()
* @brief	: A simple UDP client
*/
int udp_client(char *ip_address,char *data,int length,int,int,int sd);


/*
* @function : udp_server()
* @brief	: A simple UDP server
*/
int udp_server(char *msg,int sd,char *his_node);
