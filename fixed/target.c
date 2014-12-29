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
#define READ_DATA_SIZE     (1024)*(1024)*(512)
#define SAMPLING_TIME   1
#define SLEEP_TIME 1

long
get_read_addr(int drive_fd)
{

    int     ret = 0;
    long    rand_num;
    off_t   drive_size;

    ret = ioctl(drive_fd, BLKGETSIZE64, &drive_size);
    if(ret == -1)
    {
        perror("ioctl  ");
        return -1;
    }
    debug("Size of disk : %ld", drive_size);


    while(true)
    {
        rand_num = random();
        if(rand_num < drive_size)
            break;
    }

    debug("rand_num = %ld", rand_num);
    return rand_num;
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


char    read_buff[READ_DATA_SIZE];
int
send_bit(int disk_fd, char bit)
{
    long    read_addr;
    time_t  start;
    //char    read_buff[READ_DATA_SIZE];

    printf("Sending bit -------> (%c)\n", bit);

    if(bit == '0')
    {
        sleep(SLEEP_TIME);
    }
    else
    {
        //read_addr = get_read_addr(disk_fd);
        start = time(NULL);
        while(time(NULL) <= (start + SAMPLING_TIME))
        {
            read_addr = get_read_addr(disk_fd);
            lseek(disk_fd, read_addr, SEEK_SET);
            read(disk_fd, read_buff, READ_DATA_SIZE);
        }
    }

    return 0;
}


int
send_data(int disk_fd)
{
    //char    *data = "1010101110101010111010101010100011100001111";
    char    *data = "00010001001000100100";
    int     len, i;

    len = strlen(data);
    for(i=0; i<len; i++)
        send_bit(disk_fd, data[i]);

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
        printf("Usage : target <drive-for-communication>\n");
        return -1;
    }


    dfd = open(argv[1], O_RDWR);
    if(dfd == -1)
    {
        perror("open");
        return -1;
    }
    printf("Waiting ...\n");
    //sleep(15);

    printf("----------------- Sending data over covert channel --------------------\n");
    synchronize();    
    send_data(dfd);

    return 0;
}
