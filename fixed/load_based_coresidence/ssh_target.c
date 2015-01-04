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
#include <limits.h>
#include <signal.h>
#include <errno.h>


#define MAX_SCP_PER_SECOND 5

#define DEBUG 1 
#define debug(fmt, ...)\
do{\
    if(DEBUG)\
    fprintf(stderr, "%s(%d): " fmt "\n", __FUNCTION__, __LINE__, __VA_ARGS__);\
}while(0)

#define SLEEP_TIME 1


#define DEST_DIR  "/home/akumarkk/Desktop/ssh_target.data"
int child_proc[1024];
int child_count;


void
cleanup_child_proc()
{
    int i, pid;
    for(i=0; i< child_count; i++)
    {
        pid = waitpid(child_proc[i], NULL, WUNTRACED | WNOHANG);
        if(pid == 0)
        {
            /* This is still running. just clean this */
            kill(child_proc[i], SIGKILL);
        }
        sleep(1);// Allow one sec to finish for next child proc
    }
}



int
scp(char *ip)
{
    int     pid, ret;
    char    *argv[5], cwd[PATH_MAX];
    char    temp_file[256] = "", tmp_dir[256] = "";
    char    tmp[256] = "";

    snprintf(temp_file, sizeof(temp_file), "vm_dataXXXXXX");


    if(getwd(cwd) == NULL)
    {
        perror("getwd");
        return -1;
    }

    snprintf(tmp_dir, sizeof(tmp), "%s/tmp", cwd);
    if(mkdir(tmp_dir, 0777) == -1 && errno != EEXIST)
    {
        perror("mkdir");
        return -1;
    }

    snprintf(temp_file, sizeof(temp_file), "%s/vm_dataXXXXXX", tmp_dir);
    if((ret = mkstemp(temp_file)) == -1)
    {
        perror("mkstemp");
        return -1;
    }
    else
        close(ret);

    snprintf(tmp, sizeof(tmp), "akumarkk@%s:%s", ip,DEST_DIR);


    argv[0] = "/usr/bin/scp";
    argv[1] = tmp;
    argv[2] = temp_file;
    argv[3] = NULL;

    printf("/usr/bin/scp %s %s \n", tmp, temp_file);
    pid = fork();
    if(pid == 0)
    {
        printf("scp process : (%d)  file : %s\n\n", getpid(), temp_file);
        execv(argv[0], argv);

        perror("execv");
        printf("Unexpected error...\n");
        exit(0);
    }
    else
    {
        if(pid == -1)
        {
            perror("fork");
            return -1;
        }

        /* Need this database as we are not waiting here.
         * because stuck child processes can later become 
         * zombie or left uncleaned resulting in system
         * computing resource lockup.
         */
        child_proc[child_count] = pid;
        child_count++;
        /* No need to wait here as we need to spawn multiple processes */
        return 0;
    }
}

int
get_file(char *ip)
{
    long start;
    int  i = 0;
    
    start = time(NULL);
    while(time(NULL) < start+1 && (i < MAX_SCP_PER_SECOND))
    {
	scp(ip);
	i++;
    }

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
send_bit(char *ip_addr, char bit)
{
    int ret = 0;

    printf("Sending bit -------> (%c)\n", bit);

    if(bit == '0')
    {
        sleep(SLEEP_TIME);
    }
    else
    {
	ret = get_file(ip_addr);
	if(ret == -1)
	    printf("Not able to send bit (%d)\n", bit);
    }

    return 0;
}


int
send_data(char *ip_addr)
{
    //char    *data = "1010101110101010111010101010100011100001111";
    char    *data = "00010101010101010101010101010101";
    int     len, i;

    len = strlen(data);
    for(i=0; i<len; i++)
        send_bit(ip_addr, data[i]);

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
        printf("Usage : ssh_target <ip_addr_of_targetVM>\n");
        return -1;
    }


    printf("----------------- Sending data over covert channel --------------------\n");
    send_data(argv[1]);

    return 0;
}
