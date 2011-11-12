// kernal.c
#include "rtx.h"
#include "kernal.h"
#include <signal.h>

pcb* pid_to_pcb(int pid)
{
	switch (pid) {

		case 0 : return pcb_list[0];
		case 1 : return pcb_list[1];
		case 2 : return pcb_list[2];
		default: return NULL;

	}
}

MsgEnv* k_request_msg_env()
{
	// the real code will keep on trying to search free env
	// queue for envelope and get blocked otherwise
	if (MsgEnvQ_size(free_env_queue) == 0)
		return NULL;

	MsgEnv* free_env = (MsgEnv*)MsgEnvQ_dequeue(free_env_queue);
	return free_env;
}

int k_release_message_env(MsgEnv* env)
{
	if (env == NULL)
		return NULL_ARGUMENT;
	MsgEnvQ_enqueue(free_env_queue, env);
	// check processes blocked for allocate envelope later
	return SUCCESS;
}

int k_send_message(int dest_process_id, MsgEnv *msg_envelope)
{
	if (DEBUG==1) {
		ps("In send message");

		fflush(stdout);
		printf("Dest process id is %i\n",dest_process_id);
		fflush(stdout);
	}
	pcb* dest_pcb =  pid_to_pcb(dest_process_id);

	if (!dest_pcb || !msg_envelope) {
		printf("The destPCB or MSG_ENV is empty\n");
		return NULL_ARGUMENT;
	}

	msg_envelope->sender_pid = current_process->pid;
	msg_envelope->dest_pid = dest_process_id;

	if (DEBUG==1) {

		fflush(stdout);
		printf("Dest pid is %i\n",dest_pcb->pid);
		fflush(stdout);
	}

	MsgEnvQ_enqueue(dest_pcb->rcv_msg_queue, msg_envelope);
	if (DEBUG==1){
		printf("message SENT on enqueued on PID %i and its size is %i\n",dest_pcb->pid,MsgEnvQ_size(dest_pcb->rcv_msg_queue));
	}
	return SUCCESS;
}

MsgEnv* k_receive_message()
{
	MsgEnv* ret = NULL;
	if (DEBUG==1) {
		fflush(stdout);
		//printf("Current PCB msgQ size is %i for PID %i\n", MsgEnvQ_size(current_process->rcv_msg_queue), current_process->pid );
	}
	if (MsgEnvQ_size(current_process->rcv_msg_queue) > 0){
		ret = (MsgEnv*)MsgEnvQ_dequeue(current_process->rcv_msg_queue);
	}
	else
	{
		if (current_process->is_i_process == TRUE || current_process->state == NEVER_BLK_RCV)
			return ret;
	}

	return ret;
}

int k_send_console_chars(MsgEnv *message_envelope)
{
	if (!message_envelope)
		return NULL_ARGUMENT;


	message_envelope->msg_type = DISPLAY_ACK;
	int retVal = k_send_message(CRT_I_PROCESS_ID, message_envelope);
	crt_i_proc(0);
	return retVal;
}

int k_get_console_chars(MsgEnv *message_envelope)
{
	if (!message_envelope)
		return NULL_ARGUMENT;
	message_envelope->msg_type = CONSOLE_INPUT;
	int retVal = k_send_message( KB_I_PROCESS_ID, message_envelope);

	//current_process = pid_to_pcb(KB_I_PROCESS_ID);
	ps("invoking kbd");
	//kbd_i_proc(0);
	if (DEBUG==1) {
		printf("keyboard process returned to get-console-chars\n");
	}

	return retVal;
}

void atomic(bool state)
{
	if (state != TRUE || state!= FALSE)
		return;

	static sigset_t oldmask;
	sigset_t newmask;
	if (state == TRUE)
	{
		current_process->a_count ++; //every time a primitive is called, increment by 1

		//mask the signals, so atomicity is enforced
		// if first time of atomic on
		if (current_process->a_count == 1)
		{
			sigemptyset(&newmask);
			sigaddset(&newmask, SIGALRM); //the alarm signal
			sigaddset(&newmask, SIGINT); // the CNTRL-C
			sigaddset(&newmask, SIGUSR1); // the KB signal
			sigaddset(&newmask, SIGUSR2); // the CRT signal
			sigprocmask(SIG_BLOCK, &newmask, &oldmask);
		}
	}
	else
	{
		current_process->a_count--; //every time a primitive finishes, decrement by 1
		//if all primitives completes, restore old mask, allow signals
		if (current_process->a_count == 0)
		{
			//restore old mask
			sigprocmask(SIG_SETMASK, &oldmask, NULL);
		}
	}
}

int k_pseudo_process_switch(int pid)
{
	pcb* p = (pcb*)pid_to_pcb(pid);
	if (p == NULL)
		return ILLEGAL_ARGUMENT;
	prev_process = current_process;
	current_process = p;
	return SUCCESS;
}

void k_return_from_switch()
{
	pcb* temp = current_process;
	current_process = prev_process;
	prev_process = current_process;
}

void k_process_switch(ProcessState next_state)
{
	pcb* next_process = PriorityQ_dequeue(rdy_proc_queue);
	// Note this is not checking for null process. It is just for checking th dequeue
	// was successful
	if (next_process != NULL)
	{
		current_process->state = next_state;
		pcb* old_process = current_process;
		current_process = next_process;
		k_context_switch(old_process->buf, next_process->buf);
	}
}

void k_context_switch(jmp_buf* prev, jmp_buf* next)
{
	int val = setjmp(*prev);
	if (val == 0)
	{
		longjmp(*next, 1);
	}
}

int k_release_processor()
{
	PriorityQ_enqueue(current_process);
	k_process_switch(READY);
}

int k_request_process_status(MsgEnv *env)
{
	char* status = (char*)env->data;
	int i;
	for (i = 0; i < PROCESS_COUNT; ++i)
	{
		*status = pcb_list[i]->pid;
		status ++;
		*status = pcb_list[i]->state;
		status++;
		*status = pcb_list[i]->priority;
		status++;
	}
	return SUCCESS;
}

int k_terminate()
{
	cleanup();
}

int k_change_priority(int target_priority, int target_pid)
{
	if (target_priority >= 3 || target_priority < 0)
			return ILLEGAL_ARGUMENT;

	pcb* target_pcb = pid_to_pcb(target_pid);
	if (target_pcb->pid == NULL_PROCESS_ID || target_pcb->is_i_process == TRUE)
		return ILLEGAL_ARGUMENT;

	// if on a ready queue, take if off, change priority, and put it back on
    if(target_pcb->state == READY)
    {
        PriorityQ_dequeue(rdy_proc_queue, target_pcb);
        target_pcb->priority = target_priority;
        PriorityQ_enqueue(ready_pq, target_pcb);
    }
    else
    {
    	target_pcb->priority = target_priority;
    }
    return SUCCESS;
}

/*int k_log_message(MsgEnv* env, TraceBuffer* buff)
{
	if (env == NULL || buff == NULL)
		return NULL_ARGUMENT;

	// copy necessary fields from the message envelope to buffer tail
	buff
}*/


