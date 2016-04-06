#include "includes.h"
#include "types.h"
#include "nlp.h"
#include "tlp.h"
#include "ftp.h"



int ackCount = 0;
int ACK_FLAG = 0;
#define MAX_NO_OF_PACKETS 128


unsigned char mask = 0x7f;
short int lengthMask = 0x00ff;
short int endMask = 0x300;



// check the TLP service
int tlp_connect(char node,char dest_service_number[])
{
	int length;
	int seq_no = 1;
	char *tlp_packet;
	time_t timer1,timer2;

	tlp_packet = (char*)malloc(strlen(dest_service_number)+TLP_HEADER_SIZE);

	length = prep_tlp_packet((unsigned char)seq_no,(unsigned char)0,0,dest_service_number,tlp_packet,strlen(dest_service_number));
	
	nlp_send_data(node,tlp_packet,NLP_DATA_ONLY,length);
	
	time(&timer1);
	while(ACK != 1)
	{
		time(&timer2);
		if(timer2 - timer1 > TIME)
		{
			printf("\n Time out .. transmitting again");
			nlp_send_data(node,tlp_packet,NLP_DATA_ONLY,length);
			//reset the timerv
			time(&timer1);
		}
			
		

	}
	
	printf("\n Service Confirmed ..");
	//free(tlp_packet);
	return TRUE;
}


// make the TLP packet
int prep_tlp_packet(unsigned char snumber,unsigned char anumber,char ack,char *data,char *tlp,int data_length)
{
	int i=0;
	
	short int tot_length = data_length + TLP_HEADER_SIZE;
	
	unsigned short int checksum;
	char overflow;
	unsigned short int temp;


	
	

	tlp[i++] = (mask & snumber) << 1;
	tlp[i++] = ((mask & anumber) << 1) | (0x1 & ack);
	tlp[i++] = (tot_length && lengthMask);

	if(data_length < MAX_TLP_DATA_SIZE)
		// end bit condition
		tlp[i++] = (tot_length && endMask) >> 2 | 0x0 | 0x1; 	
	else
		tlp[i++] = (tot_length && endMask) >> 2 | 0x0 | 0x0;
	for(i=0;i<data_length;i++)
		tlp[i+TLP_HEADER_SIZE] = data[i];

	checksum = 0x0;

	

	for(i=0;i<tot_length;i++)
	{
		if(i == 4 || i == 5)
			continue;

		temp = checksum + tlp[i];

		overflow = (temp < checksum) ? 1:0;

		if(!overflow)
			overflow = (temp < tlp[i]) ? 1:0;

		checksum += tlp[i];
		checksum += overflow;
		
	}

	

	tlp[TLP_HEADER_SIZE-2] = (char)(~checksum>>8);
	tlp[TLP_HEADER_SIZE-1] = (char)(~checksum&(0xff));

	

	return tot_length;
}


int tlp_checksum(char *tlp,int length)
{
	int i=0;
	unsigned short int checksum=0;
	unsigned short int chk;
	unsigned short int temp;
	char overflow;

	

	checksum = 0x0;

	for(i=0;i<length;i++)
	{
		if(i != (TLP_HEADER_SIZE-2) && i!= (TLP_HEADER_SIZE-1))
		{
			temp = checksum + tlp[i];
			overflow = (temp < checksum) ? 1:0;

			if(!overflow)
				overflow = (temp < tlp[i]) ? 1:0;

			checksum += tlp[i];
			checksum += overflow;
		}

		

	}
	unsigned short int offset = tlp[TLP_HEADER_SIZE-2];

	chk = (unsigned short int)((offset << 8 )|((0xff)&tlp[TLP_HEADER_SIZE-1]));

	checksum += chk;

	// validate the checksum
	if(checksum == 0xffff)
		return TRUE;
	else
		return FALSE;
}


// the TLP layer is initialized here
int initialize_tlp(char *file)
{
	int retVal;
	retVal = populate_rt(file);



	if(retVal == BAD_CONFIG_FILE)
		return retVal;
	// initialize the lower layer now
	initialize_nlp();

}


// this API is used to send the regular TLP data to destination
int tlp_send_data(char dest_node,char *ftp_packet,int size)
{
	int i, loop;
	int new_size;
	char seq_number;
	time_t timer1, timer2;
	next_packet_number = next_packet_number % MAX_NO_OF_PACKETS;
	

	char *tlp_packet;
	tlp_packet = (char*)malloc(MAX_TLP_DATA_SIZE+TLP_HEADER_SIZE);

	seq_number = 1;

	

	// increment each time by the MAX_TLP_DATA_SIZE only
	for(i=0;i<size;i+=MAX_TLP_DATA_SIZE)
	{
		if(i+MAX_TLP_DATA_SIZE > size)
			new_size = size - i;
		else
			new_size = MAX_TLP_DATA_SIZE;


		int end;
		end = prep_tlp_packet((unsigned char)seq_number,(unsigned char)0,0,ftp_packet+i,tlp_packet,new_size);


		tlp_dest_node = dest_node;

		nlp_send_data((char)dest_node,tlp_packet,NLP_DATA_ONLY,end);
		seq_number++;
		time(&timer1);	//start timer

		ACK_FLAG = FALSE;	//assume no ACK

		//printf("Seq_number  = %d\n", seq_number);
		

		

		while(ACK != seq_number)
		{
			seq_number = seq_number % MAX_NO_OF_PACKETS;
			//wrap around
			if(seq_number == 0)
			seq_number = 1;

			// end condition
			if(i == size - 1)
				break;
			printf("Value of i = %d\n", i);
			printf("Packet Seq number = %u\nAnd ACK = %d\n", seq_number%128, ACK);

			

			if(timer2 - timer1 > 1)
			{
				printf("\nTime out... I am retransmitting the data packet\n");
				nlp_send_data((char)dest_node,tlp_packet,NLP_DATA_ONLY,end);
				//reset the timer
				time(&timer1);
				ACK_FLAG = FALSE;
			}

			
			if(ACK == seq_number-1 && ACK_FLAG == TRUE)
			{
				nlp_send_data((char)dest_node,tlp_packet,NLP_DATA_ONLY,end);
				ACK_FLAG = FALSE;
				//time(&timer1);
			}
			
			time(&timer2);

			if(ACK == 0)
			{
				printf("\n\nDisconnected\n");
				return DISCONNECT;
			}
		}

		printf("I am exiting from the while loop as I got the packet ACKed\n");

	}

	//free(tlp_packet);

}



void tlp_receive_packet(char source_node,char *tlp_packet,int length)
{
	unsigned char current_packet_number;

	next_packet_number = next_packet_number % MAX_NO_OF_PACKETS;

	if(tlp_checksum(tlp_packet,length) == TRUE)
	{
		printf("\nChecksum pass\n");
		
		// Check for ACK
		if((tlp_packet[1] & 0x1))
		{
			ACK = mask & (tlp_packet[1]>>1);
			printf("\n ACK for packet number %d received\n",ACK);
			ackCount++;
			printf("Ack count = %d\n", ackCount);
			ACK_FLAG = TRUE;
		}
		else
		{
			current_packet_number = (unsigned char)(mask &(tlp_packet[0] >> 1));

			printf("Received packet number = %d\n", current_packet_number);


			if(start == FALSE)
			{
				if(atoi(tlp_packet+TLP_HEADER_SIZE))
				{
					printf("\n checking available service ..");
					int service_requested = atoi(tlp_packet+TLP_HEADER_SIZE);
					switch(service_requested)
					{
						case 40:	start = TRUE;
								sender = RECEIVER;
								break;
					}
					
					char *ack_packet = (char *)malloc(TLP_HEADER_SIZE);
					int end = prep_tlp_packet((unsigned char)0,(unsigned char)1,1,NULL,ack_packet,0);
					nlp_send_data(source_node,ack_packet,NLP_DATA_ONLY,end);
					free(ack_packet);
					next_packet_number = 1;
				}

			}
			else if(start == TRUE && !atoi(tlp_packet+TLP_HEADER_SIZE))
			{
				if(next_packet_number == current_packet_number)
				{
					// write data to file
					int status = write_to_file(tlp_packet,length);

					if(status == SUCCESS)
					{
						printf("\n Please hit return, writing last packet to file\n");
					}

					// Check for the last packet by checking the end bit
					if(tlp_packet[3] & 0x1 == 1)
					{
						// prepare the last ACK here and send it
						start = FALSE;
						char *ack_packet = (char *)malloc(TLP_HEADER_SIZE);
						next_packet_number = (next_packet_number + 1) % MAX_NO_OF_PACKETS;
						if(next_packet_number == 0)
						next_packet_number = 1;

						int end = prep_tlp_packet((unsigned char)(0),(unsigned char)(next_packet_number),1,NULL,ack_packet,0);
						nlp_send_data(source_node,ack_packet,NLP_DATA_ONLY,end);
						
						free(ack_packet);
					}
					else
					{
						char *ack_packet = (char *)malloc(TLP_HEADER_SIZE);
						next_packet_number = (next_packet_number + 1) % MAX_NO_OF_PACKETS;
						if(next_packet_number == 0)
						next_packet_number = 1;

                                    int end = prep_tlp_packet((unsigned char)0,(unsigned char)(next_packet_number),1,NULL,ack_packet,0);
                                    nlp_send_data(source_node,ack_packet,NLP_DATA_ONLY,end);
								
                                    free(ack_packet);
						
					}
						
				}
				else	// to avoid loop if duplicate packets are received again and again
				{
					char *ack_packet = (char*)malloc(TLP_HEADER_SIZE);
					int numBytes = prep_tlp_packet(0,(unsigned char)next_packet_number,1,NULL,ack_packet,0);
					nlp_send_data(source_node, ack_packet, NLP_DATA_ONLY, numBytes);
					free(ack_packet);
				}
			}

		}
	}
	else
	{
		if(sender == RECEIVER)
		{
			if(start == TRUE)
			{
				char *ack_packet = (char *)malloc(TLP_HEADER_SIZE);
				int numBytes = prep_tlp_packet((unsigned char)0, (unsigned char)next_packet_number, 1, NULL, ack_packet, 0);
				nlp_send_data(source_node, ack_packet, NLP_DATA_ONLY, numBytes);
				free(ack_packet);
			}
		}

	}

}





