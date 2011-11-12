#include "iProcs.h"
#include "rtx.h"
#include "kbcrt.h"

void crt_i_proc(int signum)
{
	int error = k_pseudo_process_switch(CRT_I_PROCESS_ID);

	if (error != SUCCESS)
	{
		printf("Error! Process Switch failed in CRT I process");
		cleanup();
		return;
	}

	ps("Inside CRT I proc");

	if (signum == SIGUSR2)
	{
#if DEBUG
			fflush(stdout);
			printf("Current PCB msgQ size is %i for process 1\n", MsgEnvQ_size(current_process->rcv_msg_queue) );
			ps("Got SIGUSR2");
#endif

			MsgEnv* envTemp = NULL;
			envTemp = MsgEnvQ_dequeue(displayQ);
			if (envTemp == NULL)
			{
				printf("Warning: Recieved a signal in CRT I process but there was no message.");
				return;
			}
			envTemp->msg_type = DISPLAY_ACK;
			k_send_message(P_PROCESS_ID, envTemp);
			ps("Display ACK sent by crt");
			k_return_from_switch();
			return;
	}

	MsgEnv* env = (MsgEnv*)k_receive_message();
	outputbuf command;

	if (env==NULL) {
		env = (MsgEnv*)k_receive_message();
	}

#if DEBUG
		fflush(stdout);
		printf("Message received by crt i proc\n");
		fflush(stdout);
		printf("Current PCB msgQ size is %i for process 1\n", MsgEnvQ_size(current_process->rcv_msg_queue) );
		printf("The message data section holds \"%s\" \n",env->data);
		fflush(stdout);
#endif

	strcpy(in_mem_p_crt->outdata,env->data);

#if DEBUG
		printf("The message data section holds \"%s\" \n",in_mem_p_crt->outdata);
#endif

	MsgEnvQ_enqueue(displayQ,env);
	in_mem_p_crt->ok_flag =  OKAY_DISPLAY;

	k_return_from_switch();
	return;
}

void kbd_i_proc(int signum)
{
	int error = k_pseudo_process_switch(KB_I_PROCESS_ID);
	if (error != SUCCESS)
	{
		printf("Error! Context Switch failed in keyboard I process");
		cleanup();
	}

	ps("Inside keyboard I proc");
	MsgEnv* env = (MsgEnv*)k_receive_message();

	if (env != NULL)
	{
		ps("Envelope recognized by kbd_i_proc");

		// Loop until writing in shared memory is done
		while (in_mem_p_key->ok_flag==OKAY_TO_WRITE);

		memcpy(env->data,in_mem_p_key->indata,in_mem_p_key->length);

		// Send message back to process that called us
		k_send_message(env->sender_pid ,env);

		ps("Keyboard sent message");

		in_mem_p_key->ok_flag = OKAY_TO_WRITE; // okay to write again
	}
	k_return_from_switch();
	return;
}
