
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <signal.h>
#include "kbcrt.h"
#include <fcntl.h>

#include <sys/mman.h>
#include <sys/wait.h>

int bufsize = BUFFERSIZE;
int buf_index;

// do any necessary cleanup before exitting
// ( in this case, there is no cleanup to do)
// Basically, we rely on the parent process to cleanup shared memory
void in_die(int signal)
{
	printf("CRT exiting...\n");
	exit(0);
}


int main (int argc, char * argv[])
{
	int parent_pid, fid;

	caddr_t mmap_ptr;
	outputbuf * in_mem_p;
	char c;


	// if parent tells us to terminate, then clean up first
	sigset(SIGINT,in_die);

	// get id of process to signal when we have input
	// and the file id of the memory mapped file
	// i.e. process input arguments 
	sscanf(argv[1], "%d", &parent_pid );
	sscanf(argv[2], "%d", &fid );  // get the file id

	// attach to shared memory so we can pass input to 
	// keyboard interrupt handler
	
	mmap_ptr = mmap((caddr_t) 0,   /* Memory Location, 0 lets O/S choose */
		    bufsize,/* How many bytes to mmap */
		    PROT_READ | PROT_WRITE, /* Read and write permissions */
		    MAP_SHARED,    /* Accessible by another process */
		    fid,           /* which file is associated with mmap */
		    (off_t) 0);    /* Offset in page frame */
    if (mmap_ptr == MAP_FAILED){
      printf("Child memory map has failed, CRT is aborting!\n");
	  in_die(0);
    }
	
	in_mem_p = (outputbuf *) mmap_ptr; // now we have a shared memory pointer

	// read keyboard
	buf_index = 0;
	in_mem_p->ok_flag = WAITING_TO_BE_WRITTEN;
	do
	{
		while(in_mem_p->ok_flag == WAITING_TO_BE_WRITTEN) {
			usleep(100000);
		}
		fflush(stdout);
		printf("====================CRT OUTPUT: %s===================\n",in_mem_p->outdata);
		fflush(stdout);
		in_mem_p->ok_flag = WAITING_TO_BE_WRITTEN;
		kill(parent_pid,SIGUSR2);
		
	}while(1);  //an infinite loop - exit when parent signals us

} // CRT


