#include "rtx_init.h"
#include "rtx.h"
#include "kbcrt.h"


// Initialization Table
InitProc init_table[PROCESS_COUNT] = {
		{"Keyboard I proc\0", KB_I_PROCESS_ID, 0, TRUE},
		{"CRT I proc\0", CRT_I_PROCESS_ID, TRUE},
		{"P Process\0", P_PROCESS_ID, FALSE}
};

int init_globals() {

	numOfTicks = 0;
	displayClock = 0;
	bufsize = BUFFERSIZE;

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	sfilename = "keyboardBuffer";  //the name of the keyboard_memory file
	cfilename = "crtBuffer";  //the name of the crt_memory file
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	return SUCCESS;

}

int init_all_lists()
{
	int i;
	int init_status = SUCCESS;

	free_env_queue = (MsgEnvQ*)MsgEnvQ_create();
	displayQ = (MsgEnvQ*)MsgEnvQ_create();

	for (i = 0; i < PROCESS_COUNT; ++i)
	{
		pcb_list[i] = (pcb*)malloc(sizeof(pcb));
		if (pcb_list[i] == NULL)
		{
			init_status   = OTHER_ERROR;
			break;
		}
		pcb_list[i]->pid = init_table[i].pid;
		pcb_list[i]->priority = init_table[i].priority;
		pcb_list[i]->state = READY;
		pcb_list[i]->name = init_table[i].name;
		pcb_list[i]->rcv_msg_queue = (MsgEnvQ*)MsgEnvQ_create();
		pcb_list[i]->is_i_process = init_table[i].is_i_process;
	}

	for (i = 0; i < MSG_ENV_COUNT; i++)
	{
		msg_list[i] = (MsgEnv*)malloc(sizeof(MsgEnv));
		if (msg_list[i] == NULL)
		{
			init_status   = OTHER_ERROR;
			break;
		}
		msg_list[i]->data = (char*)malloc(sizeof(char)*MSG_ENV_SIZE);
		if (msg_list[i]->data == NULL)
		{
			init_status  = OTHER_ERROR;
			break;
		}
		MsgEnvQ_enqueue(free_env_queue, msg_list[i]);
		// deal with non null later
	}


	current_process = (pcb*)pid_to_pcb(P_PROCESS_ID);
	current_process->state = NEVER_BLK_RCV;
	prev_process = current_process;

	sigset(SIGINT,die);		// catch kill signals
	sigset(SIGBUS,die);		// catch bus errors
	sigset(SIGHUP,die);
	sigset(SIGILL,die);		// illegal instruction
	sigset(SIGQUIT,die);
	sigset(SIGABRT,die);
	sigset(SIGTERM,die);
	sigset(SIGSEGV,die);	// catch segmentation faults

	sigset(SIGUSR1,kbd_i_proc);
	sigset(SIGUSR2,crt_i_proc);


	return init_status ;
}

int init_mmaps() {
	/* Create a new mmap file for read/write access with permissions restricted
		to owner rwx access only */
		fid = open(sfilename, O_RDWR | O_CREAT | O_EXCL, (mode_t) 0755 );
		if (fid < 0){
			printf("Bad Open of mmap file <%s>\n", sfilename);
			exit(0);
		};

		fid2 = open(cfilename, O_RDWR | O_CREAT | O_EXCL, (mode_t) 0755 );
		if (fid2 < 0){
			printf("Bad Open of mmap file <%s>\n", cfilename);
			exit(0);
		};

		// make the file the same size as the buffer
		status = ftruncate(fid, bufsize );
		if (status){
			printf("Failed to ftruncate the file <%s>, status = %d\n", sfilename, status );
			exit(0);
		}

		status = ftruncate(fid2, bufsize );
		if (status){
			printf("Failed to ftruncate the file <%s>, status = %d\n", cfilename, status );
			exit(0);
		}


		// pass parent's process id and the file id to child
		char childarg1[20], childarg2[20]; // arguments to pass to child process(es)
		int mypid = getpid();			// get current process pid

		sprintf(childarg1, "%d", mypid); // convert to string to pass to child
	    sprintf(childarg2, "%d", fid);   // convert the file identifier


		// now start doing whatever work you are supposed to do
		// in this case, do nothing; only the keyboard handler will do work

		printf("\nWelcome to the CCI, the OS is at your disposal: \n");

		// create the keyboard reader process
		// fork() creates a second process identical to the current process,
		// except that the "parent" process has in_pid = new process's ID,
		// while the new (child) process has in_pid = 0.
		// After fork(), we do execl() to start the actual child program.
		// (see the fork and execl man pages for more info)

		in_pid_keyboard = fork();
		if (in_pid_keyboard == 0)	// is this the child process ?
		{
			execl("./keyboard", "keyboard", childarg1, childarg2, (char *)0);
			// should never reach here
			fprintf(stderr,"kernal: can't exec keyboard, errno %d\n",errno);
			cleanup();
			exit(1);
		};

		// pass parent's process id and the file id to child CRT

		sprintf(childarg1, "%d", mypid); // convert to string to pass to child
	    sprintf(childarg2, "%d", fid2);   // convert the file identifier

		in_pid_crt = fork();
		if (in_pid_crt == 0)	// is this the child process ?
		{
			execl("./crt", "crt", childarg1, childarg2, (char *)0);
			// should never reach here
			fprintf(stderr,"kernal: can't exec crt, errno %d\n",errno);
			cleanup();
			exit(1);
		};

		// the parent process continues executing here

		// sleep for a second to give the child process time to start
		sleep(1);

		// allocate a shared memory region using mmap
		// the child process also uses this region

		mmap_ptr_keyboard = mmap((caddr_t) 0,   /* Memory location, 0 lets O/S choose */
			    bufsize,              /* How many bytes to mmap */
			    PROT_READ | PROT_WRITE, /* Read and write permissions */
			    MAP_SHARED,    /* Accessible by another process */
			    fid,           /* the file associated with mmap */
			    (off_t) 0);    /* Offset within a page frame */
		if (mmap_ptr_keyboard == MAP_FAILED){
			printf("Parent's memory map has failed for keyboard, about to quit!\n");
			die(0);  // do cleanup and terminate
		};

		in_mem_p_key = (inputbuf *) mmap_ptr_keyboard;   // pointer to shared memory
		// we can now use 'in_mem_p' as a standard C pointer to access
		// the created shared memory segment

		in_mem_p_key->ok_flag = 0;


		// allocate a shared memory region using mmap
		// the child process also uses this region

		mmap_ptr_crt = mmap((caddr_t) 0,   /* Memory location, 0 lets O/S choose */
			    bufsize,              /* How many bytes to mmap */
			    PROT_READ | PROT_WRITE, /* Read and write permissions */
			    MAP_SHARED,    /* Accessible by another process */
			    fid2,           /* the file associated with mmap */
			    (off_t) 0);    /* Offset within a page frame */
		if (mmap_ptr_crt == MAP_FAILED){
			printf("Parent's memory map has failed for CRT, about to quit!\n");
			die(0);  // do cleanup and terminate
		};

		in_mem_p_crt = (outputbuf *) mmap_ptr_crt;   // pointer to shared memory
		// we can now use 'in_mem_p' as a standard C pointer to access
		// the created shared memory segment

		in_mem_p_crt->ok_flag = 0;

		return SUCCESS;
}

//**************************************************************************
// routine to clean up things before terminating main program
// This stuff must be cleaned up or we have child processes and shared
//	memory hanging around after the main process terminates
void cleanup()
{
	// terminate child process(es)
	kill(in_pid_keyboard,SIGINT);
	kill(in_pid_crt,SIGINT);

	// remove shared memory segment and do some standard error checks
	status = munmap(mmap_ptr_keyboard, bufsize);
    if (status == -1){
      printf("Bad munmap during cleanup\n");
    } else {
      printf("Unmapped shared memory for keyboard...\n");
    }

    status = munmap(mmap_ptr_crt, bufsize);
    if (status == -1){
      printf("Bad munmap during cleanup\n");
    } else {
      printf("Unmapped shared memory for CRT...\n");
    }

	// close the temporary mmap file
    status = close(fid);
    if (status == -1){
      printf("Bad close of temporary mmap file during cleanup\n");
    } else {
      printf("Closed the file used for shared keyboard...\n");
    }

    status = close(fid2);
    if (status == -1){
      printf("Bad close of temporary mmap file during cleanup\n");
    } else {
      printf("Closed the file used for shared CRT...\n");
    }

	// unlink (i.e. delete) the temporary mmap file
    status = unlink(sfilename);
    if (status == -1){
      printf("Bad unlink during claeanup.\n");
    } else {
      printf("Deleted the file used for shared keyboard...\n");
    }

    status = unlink(cfilename);
    if (status == -1){
      printf("Bad unlink during clean up.\n");
    } else {
      printf("Deleted the file used for shared CRT...\n");
    }

    // It is possible allocation failed in initialization
    // We try to ensure we don't try to free memory that was never allocated by always checking whether the pointer
    // is NULL or not
	int i;
	ps("Freeing All Queues");
	MsgEnvQ_destroy(free_env_queue);
	ps("Freeing PCBs\n");
	for (i = 0; i < PROCESS_COUNT; ++i)
	{
#if DEBUG
		printf("Freeing pcb: %i\n", i+1);
#endif
		// deallocate memory until we reach location where allocation may have failed
		if (pcb_list[i] == NULL)
			break;
		else
		{
			MsgEnvQ_destroy(pcb_list[i]->rcv_msg_queue);
			free(pcb_list[i]);
		}
	}


	printf("Freeing Messages and Their Data\n");
	for (i = 0; i < MSG_ENV_COUNT; ++i)
	{
#if DEBUG
		printf("Freeing envelope: %i\n", i+1);
#endif
		// deallocate memory until we reach location where allocation may have failed
		if (msg_list[i] == NULL)
			break;
		else
		{
			if (msg_list[i]->data == NULL)
			{
				free(msg_list[i]);
				break;
			}
			else
				free(msg_list[i]);
		}
	}
	exit(1);
}
