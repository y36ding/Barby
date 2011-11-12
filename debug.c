#include "debug.h"
#include "MsgEnvQueue.h"

void pm(MsgEnv* env)
{
#if DEBUG
	fflush(stdout);

	printf("\n======== Message Envelope Contents ==========\n");
	if (env==NULL)
		printf("Envelope is NULL\n");
	else
	{
		printf("Message envelope contents:\ndest_pid:%i\nsender_pid:%i\nMsgType:%i\ndata:%s\nNext Pcb Adress:%p\n ",
				env->	dest_pid, env->sender_pid,env->msg_type,env->data,env->next);
	}
	printf("=============================\n");

	fflush(stdout);
#endif
}

void pd(double val)
{
#if DEBUG
	fflush(stdout);
	printf("%f\n", val);
	fflush(stdout);
#endif
}

void pi(int val)
{
#if DEBUG
	fflush(stdout);
	printf("%i\n", val);
	fflush(stdout);
#endif
}

void ps(char* val)
{
#if DEBUG
	fflush(stdout);
	printf("%s\n", val);
	fflush(stdout);
#endif
}

void pp(pcb* pcb)
{
#if DEBUG
	fflush(stdout);

	printf("\n======== PCB Contents ==========\n");
	if (pcb==NULL)
		printf("PCB is NULL\n");
	else
	{
		printf("PCB Contents:\n");
		printf("Name: %s\n", pcb->name);
		printf("pid: %i\n",pcb->pid);
		printf("priority: %i\n",pcb->priority);
		printf("state: %i\n",pcb->state);
		printf("Recieve Envelope Queue Size: %i\n", MsgEnvQ_size(pcb->rcv_msg_queue));
		printf("Atomic Count: %i\n",pcb->a_count);
		printf("is_i_process: %i\n",pcb->is_i_process);
	}
	printf("=============================\n");

	fflush(stdout);
#endif
}
