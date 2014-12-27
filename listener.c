#include <stdio.h>
#include <stdbool.h>
#include <string.h>
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

long
get_read_addr(int drive_fd)
{

    int     ret = 0;
    long    rand_num;
    off_t   drive_size;

    //ret = stat(argv[1], &stat_buff);
    //drive_size = lseek(dfd, 0, SEEK_END);
    /* stat() and lseek() are not gonna work here.
     * but ioctl() correctly returns the size of the drive.
     * http://unix.stackexchange.com/questions/52215/determine-the-size-of-a-block-device
     */
    ret = ioctl(drive_fd, BLKGETSIZE64, &drive_size);
    if(ret == -1) 
    {   
        perror("ioctl  ");
        return -1; 
    }   
    debug("Size of disk : %ld", drive_size);


    while(true)
    {
        rand_num = rand();
        if(rand_num < drive_size)
            break;
    }

    return rand_num;
}

void
get_baseline_results(
        int     fd,
        long    read_addr,
        long    *std_dev,
        long    *avg_slope)
{
 
}


int
main(int argc, char *argv[])
{
    off_t   drive_size;
    int	    dfd = -1;
    struct  stat stat_buff;
    int	    ret = 0;
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

    /* This is the read address that is used for entire session */
    read_addr  = get_read_addr(dfd);
    debug("Read addr : %ld\n", read_addr);
    get_baseline_results(dfd, read_addr, &std_dev, &avg_slope);
    return 0;
}
