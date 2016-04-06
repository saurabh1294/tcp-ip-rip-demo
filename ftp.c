#include "includes.h"
#include "types.h"
#include "ftp.h"
#include "tlp.h"
#include "nlp.h"




// Prepares the FTP packet
int prep_ftp_packet(char dnumber,char *data,int size,char *ftp)
{
	int i=0;

	// the FTP header with source and destination nodes
	ftp[0] = (self_node << 4) | (0x0f & dnumber);

	for(i=0;i<size;i++)
	{
		ftp[i+1] = data[i];
	}

	return (size+1);
}



// API writes to the file at the destination
int write_to_file(char *p,int length)
{
	FILE *fp = NULL;
	char *ptr;
	
	int i=0;
	

	fp = fopen("sample","a");
	if(fp == NULL)
	{
		return BAD_FILE;
	}

	ptr = p + TLP_HEADER_SIZE;

	
	length -= (TLP_HEADER_SIZE);

	

	printf("\n");
	
	

	while(i<length)
		fputc(ptr[i++],fp);

	// check for the end bit
	if(p[3] & 0x1 == 1)
	{
		printf("\n File transfer complete ..");
		return SUCCESS;
	}
	fclose(fp);
	start = TRUE;
}

int read_from_file(char *file)
{
	int i=0;
	char ch;
	FILE *infp;

	infp = fopen(file,"r");
	if(infp == NULL)
	{
		return BAD_FILE;
	}

	fseek(infp,0,SEEK_END);
	file_length = ftell(infp);
	fseek(infp,0,SEEK_SET);

	data_ptr = (char *)malloc(file_length);

	while((ch = fgetc(infp))!= EOF)
	{
			data_ptr[i++] = ch;
	}

	data_ptr[i] = 0;

	fclose(infp);
	return SUCCESS;

}

void *user_interface_ftp()
{
        char choice;
        int node;
        char file[250];
        int status;

        int i, x = 0, fileNotFoundFlag = 0;
	
        while(file_start == FALSE)
        {
			if(start == FALSE)
			{
				showmenu();
				printf("\n\t\t Please enter your choice : ");
				scanf("%c",&choice);
                switch(choice)
                {
					case '1':	display_routing_table(routing_table,-1,self_node);
								scanf("%d", &i);
								showmenu();
								break;
					case '2':
								// clear the screen
								fprintf(stdout, "\033[2J");
								fprintf(stdout, "\033[1;1H");

								printf("\n Enter the name of the file to be sent : ");
								scanf("%s",file);
								printf("\n Enter the node no where you want to send this file : ");
								scanf("%d",&node);
								source_node = get_src_node();

								status = read_from_file(file);

								if(status == BAD_FILE)
								{
									fileNotFoundFlag = 1;
									printf("\n File not found.. Please retry...");
									for(i = 1; i <= 100000000; i+=2);	//delay loop
									showmenu();
								}


								if(fileNotFoundFlag == 0)
								{
									if(tlp_connect(node,"40") == TRUE)
									{
										if(DISCONNECT == tlp_send_data((char)node,data_ptr,file_length))
										{
											printf("\n Disconnected from the node..");
										}
										
										source_node = self_node;
										destination_node = node;
									}
									getchar();
								}
								break;

					case '3': 	EXIT = TRUE;
								exit(1);
                }
		   }

        }



}

void showmenu()
{
	// clear the screen always
	fprintf(stdout, "\033[2J");
	fprintf(stdout, "\033[1;1H");
                
    printf("\n\t\t What do you want, please enter choice\n");
    printf("\n\t (1)\t Display routing table of this node\n");
    printf("\n\t (2)\t Send a file to a node\n");
    printf("\n\t (3)\t Exit\n");
}

