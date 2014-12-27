#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define DEBUG 1
#define debug(fmt, ...)\
do{\
    if(DEBUG)\
    fprintf(stderr, "%s(%d): " fmt "\n", __FUNCTION__, __LINE__, __VA_ARGS__);\
}while(0)

/* Let the size of the data to be sufficiently large */
#define READ_DATA_SIZE     (1024)*(1024)



int
main(int argc, char *argv[])
{
    off_t   drive_size;
    int     dfd = -1;
    int     ret = 0, read_bit;
    long    read_addr = 0;


    if(argc < 2)
    {
        printf("Usage : listener <drive-for-communication>\n");
        return -1;
    }


    dfd = open(argv[1], O_RDWR);
    if(dfd == -1)
    {
        perror("open");
        return -1;
    }

    send_data(dfd);

    return 0;
}
