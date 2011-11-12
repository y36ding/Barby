#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>

#include "rtx.h"
#include "kbcrt.h"
#include "userAPI.h"
#include "rtx_init.h"


void processP()
{
    ps("ProcessP Started");
    const tWait = 500000;
	MsgEnv* env;
	env = request_msg_env();
    ps("Envelopes Allocated");

    while(1) {
        ps("Asking for Characters");

        // Request keyboard input
		get_console_chars (env);

		ps("Back in Process P. Keyboard has taken input");
		// Check if keyboard i proc sent a confirmation message
		env = receive_message();
		while(env==NULL) {
			usleep(tWait);
			env = (MsgEnv*)k_receive_message();
			if (env != NULL && env->msg_type == CONSOLE_INPUT)
			{
#if DEBUG
				printf("Keyboard Input Acknowledged");
#endif
			}
		}

		// Send the input to CRT
		send_console_chars(env);

		// Check if CRT displayed
		env = receive_message();
		while(env==NULL) {
			usleep(tWait);

			//current_process = (pcb*)pid_to_pcb(P_PROCESS_ID);

			env = receive_message();
			if (env != NULL && env->msg_type == DISPLAY_ACK)
			{
#if DEBUG
				printf("CRT Display Acknowledged");
#endif
			}
		}
	}
	release_msg_env(env);
}


void die(int signal)
{
	cleanup();
	printf( "Ending main process ...\n" );
	exit(0);
}

//**************************************************************************
int main()
{

	if (init_globals() != SUCCESS) {
		printf("Globals failed to initilize!");
		cleanup();
	}

	if (init_all_lists() != SUCCESS) {
		printf("Failed to initialize the process and message envelope list. Exiting the OS.\n");
		cleanup();
	}

    if (init_mmaps() != SUCCESS) {
    	printf("Failed to init mmaps.\n");
    	cleanup();
    }

    processP();

	// should never reach here, but in case we do, clean up after ourselves
	cleanup();
} // main
