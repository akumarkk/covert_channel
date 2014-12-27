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


#define SAMPLING_TIME   1
#define SAMPLING_COUNT 10

/* Let the size of the data to be sufficiently large */
#define READ_DATA_SIZE     (1024)*(1024)

#define SAMPLE_SIZE_FOR_MEAN   5

#define NANO_SECONDS 1000000000


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

/*
 *  get_time_elapsed() : Get the elapsed time in nano seconds
 */
long
get_time_elapsed(
        struct timespec    start,
        struct timespec    end)
{
    long    tmp = end.tv_nsec - start.tv_nsec;
    struct  timespec diff;

    if(tmp < 0)
    {
        /* TIme has elapsed in terms of seconds !!! */
        diff.tv_sec = end.tv_sec - start.tv_sec;
        diff.tv_nsec = NANO_SECONDS + end.tv_nsec - start.tv_nsec;
    }
    else
    {
        diff.tv_sec = end.tv_sec - start.tv_sec;
        diff.tv_nsec = end.tv_nsec - start.tv_nsec;
    }

    if(diff.tv_sec > 0)
        diff.tv_nsec = diff.tv_nsec + (NANO_SECONDS * diff.tv_sec);

    return diff.tv_nsec;
}



long
get_mean_access_time(
        int     disk_fd,
        long    read_addr)
{
    struct timespec    before, after;
    time_t      start;
    long        sum, tmp_avg, avg, diff;
    long        elapsed_time[SAMPLE_SIZE_FOR_MEAN]; // This will have elapsed time in microseconds.
    int         j, ret = -1, i=-1;
    char        read_buff[READ_DATA_SIZE];
    //char        read_buff[1024];

    debug("disk_fd = %d  read_addr = %ld", disk_fd, read_addr);
    start = time(NULL);
    debug("start time   :   %ld", start);
    while(time(NULL) <= (start + SAMPLING_TIME))
    {
        ret = lseek(disk_fd, read_addr, SEEK_SET);
        if(ret == -1)
        {
            perror("lseek");
            return -1;
        }
        
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &before);
        read(disk_fd, read_buff, READ_DATA_SIZE);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &after);
        diff = get_time_elapsed(before, after);
        
        debug("Elapsed time     :   %ld ns", diff);
        if(i == SAMPLE_SIZE_FOR_MEAN)
        {
            for(j=0; j < SAMPLE_SIZE_FOR_MEAN; j++)
                sum = sum + elapsed_time[i];
            tmp_avg = sum/SAMPLE_SIZE_FOR_MEAN;
            avg = (avg + tmp_avg)/2;

            i = -1; //Reset it
            elapsed_time[++i] = diff;
        }
        else
            elapsed_time[++i] = diff;
    }

    return avg;

}


void
get_baseline_results(
        int     fd,
        long    read_addr,
        long    *par_std_dev,
        long    *par_avg_slope)
{
    int     i=0;
    long    prev = -1, avg_time, std_dev;
    long    slope = -1;

    debug("fd = %d  read_addr = %ld", fd, read_addr);

    for(i=0; i < SAMPLING_COUNT; i++)
    {
        avg_time = get_mean_access_time(fd, read_addr);
        std_dev = std_dev + ((double)avg_time/(double)SAMPLING_COUNT);
        
        if(prev == -1)
        {
            prev = avg_time;
            continue;
        }
        if(slope == -1)
            slope = abs(prev - avg_time);
        else if(slope < abs(prev - avg_time))
            slope = abs(prev - avg_time);
    }

    *par_std_dev = sqrt(std_dev);
    *par_avg_slope = slope;
    debug("Baseline std_dev = %ld    slope = %ld", *par_std_dev, *par_avg_slope);

}

        

int
main(int argc, char *argv[])
{
    off_t   drive_size;
    int	    dfd = -1;
    struct  stat stat_buff;
    int	    ret = 0, read_bit;
    long    read_addr = 0, slope, normalized;
    long    base_avg_slope, base_std_dev, curr, prev;


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
    
    printf("Creating baseline parameters for read bit detection ...\n");
    printf("Please wait ...\n");
    get_baseline_results(dfd, read_addr, &base_std_dev, &base_avg_slope);
    
    printf("----------------- BASELINE PARAMETERS ------------------\n");
    printf("std_dev        :       %ld\n", base_std_dev);
    printf("avg_slope      :       %ld\n", base_avg_slope);
    sleep(5);

    while(1)
    {
        curr = get_mean_access_time(dfd, read_addr);
        if(prev == -1)
        {
            prev = curr;
            debug("Exiting from here ...", ""); 
            continue;
        }

        slope = curr - prev;
        normalized = normalized + slope;
        if(normalized > base_avg_slope)
            read_bit = 1;
        else
            read_bit = 0;

        printf("curr = %ld   slope = %ld   normalized = %ld   Bit detected --------> (%d)\n", 
                curr, slope, normalized, read_bit);
    }

    return 0;
}
