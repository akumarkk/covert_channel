#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define DEBUG 1 
#define debug(fmt, ...)\
do{\
    if(DEBUG)\
    fprintf(stderr, "%s(%d): " fmt "\n", __FUNCTION__, __LINE__, __VA_ARGS__);\
}while(0)

#define SLEEP_TIME 1

int
get_file(char *file)
{
    char cmd[1024];
    int  ret = 0;
    snprintf(cmd, sizeof(cmd), "scp akumarkk@192.168.171.128:/home/akumarkk/Desktop/%s .", file); 

    debug("executing command : %s", cmd);

    ret = system(cmd);
    if(ret == -1)
    {
	printf("System failed!!!\n");
	perror("system");
	return -1;
    }
    //debug("scp status - %d", ret);
    return 0;
}
    
void
synchronize()
{
    time_t start = time(NULL);
    debug("Synchronizing %ld...",start); 
        
    while(start % 10 != 5)
    {   
        sleep(1);
        start = time(NULL);
    }   

    debug("covert communication started %ld...", start);
}


int
send_bit(char *file, char bit)
{
    int ret = 0;

    printf("Sending bit -------> (%c)\n", bit);

    if(bit == '0')
    {
        sleep(SLEEP_TIME);
    }
    else
    {
	ret = get_file(file);
	if(ret == -1)
	    printf("Not able to send bit (%d)\n", bit);
    }

    return 0;
}


int
send_data(char *file_name)
{
    //char    *data = "1010101110101010111010101010100011100001111";
    char    *data = "00010101010101010101010101010101";
    int     len, i;

    len = strlen(data);
    for(i=0; i<len; i++)
        send_bit(file_name, data[i]);

    return 0;
}

int
main(int argc, char *argv[])
{
    off_t   drive_size;
    int     dfd = -1;
    int     ret = 0, read_bit;
    long    read_addr = 0;


    if(argc < 2)
    {
        printf("Usage : ssh_target <scp_file_name>\n");
        return -1;
    }


    printf("----------------- Sending data over covert channel --------------------\n");
    send_data(argv[1]);

    return 0;
}
