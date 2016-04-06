#include "pthread.h"
#include "includes.h"
#include "types.h"
#include "ftp.h"
#include "tlp.h"
#include "nlp.h"

int main(int argc,char *argv[])
{
	int retStatus = 0;

	// check the command line arguments
	if (argc != 2)
	{
		printf("\n Usage : %s <config-file name>\n",argv[0]);
		exit(0);
	}

	retStatus = initialize_tlp(argv[1]);

	if(retStatus == BAD_CONFIG_FILE)
	{
		printf("\n Bad config file. Please retry...");
		exit(1);
	}

	
	file_start = FALSE;

	// create the main UI thread here
	pthread_create(&thread_ui,NULL,&user_interface_ftp,(void *)NULL);


	pthread_create(&thread_node_detect, NULL, &dead_node_detector, (void *)NULL);


	pthread_exit(NULL);

	return 0;
}





