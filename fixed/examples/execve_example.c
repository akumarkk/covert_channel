#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <signal.h>

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
execute_command(char *ip)
{
    int	    pid, ret;
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
main()
{
    execute_command("192.168.171.129");
    return 0;
}

