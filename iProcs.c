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
			envTemp = (MsgEnv*)MsgEnvQ_dequeue(displayQ);
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

		//copies into first parameter from second parameter of length+1 bytes
		memcpy(env->data,in_mem_p_key->indata,in_mem_p_key->length + 1);

		if (!strcmp(in_mem_p_key->indata,"t")) {
			die(SIGINT);
		}

		// Send message back to process that called us
		k_send_message(env->sender_pid ,env);

		ps("Keyboard sent message");

		in_mem_p_key->ok_flag = OKAY_TO_WRITE; // okay to write again
		k_return_from_switch();
		return;
	}
	k_return_from_switch();
	return;
}

void timer_i_proc(int signum) {

	int error = k_pseudo_process_switch(TIMER_I_PROCESS_ID);
	if (error != SUCCESS)
	{
		printf("Error! Context Switch failed in keyboard I process");
		cleanup();
	}

	//while(1)
	//  {
	        // Increment the time by recording a tick
			ps("-----------------In Timer-------------------");
	        clock_inc_time();

	        MsgEnv* msg_env = (MsgEnv*)k_receive_message();

	        pm(msg_env);

	        while (msg_env != NULL && msg_env->msg_type==WAKEUP10)
	        {
	        	fflush(stdout);
	            timeout_q_insert(msg_env);
	            msg_env = (MsgEnv*)k_receive_message();
	        }

			msg_env = (MsgEnv*)check_timeout_q();
			if (msg_env==NULL){
				fflush(stdout);
			}

	        if (msg_env != NULL)
	        {
	            // Send the envelope back
	        	fflush(stdout);
	        	msg_env->msg_type = WAKEUP10;
	        	msg_env->data = "Time expired!\0";
	            k_send_message(msg_env->sender_pid, msg_env);
	        }

			//exit i process
	        //k_i_process_exit();
	    //}
	        alarm(1);
	        k_return_from_switch();

}

int clock_get_time() {
    return numOfTicks;
}

void clock_inc_time() {
    numOfTicks++;
}

void clock_set_time(int time) {
    numOfTicks = time;
}

