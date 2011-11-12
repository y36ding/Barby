#include <stdio.h>
#include <signal.h>
#include "MsgEnvQueue.h"
#include <fcntl.h>

#include <sys/mman.h>
#include <sys/wait.h>


MsgEnvQ * MsgEnvQ_create()
{
    MsgEnvQ * queue = malloc(sizeof(MsgEnvQ));
    if (queue)
    {
        queue->head = NULL;
        queue-> tail = NULL;
    }
    return queue;
}

void MsgEnvQ_destroy(MsgEnvQ * queue)
{
    free(queue);
}

int MsgEnvQ_is_empty(MsgEnvQ* queue)
{
    if (queue == NULL)
    {
        return -1;
    }

    if (queue->head == NULL)
    {
        assert(queue->tail == NULL);
    }

    return queue->head == NULL;
}

MsgEnv* MsgEnvQ_dequeue(MsgEnvQ *queue)
{
    if (queue == NULL)
    {
        return NULL;
    }

	MsgEnv* returnEnv = queue->head;
	if (queue->head == queue->tail)
	{
		queue->tail = NULL;
	}
	if (queue->head != NULL)
	{
		queue->head = queue->head->next;
        returnEnv->next = NULL;
	}

	return returnEnv;
}


int MsgEnvQ_enqueue(MsgEnvQ *queue, MsgEnv* env)
{
	if (queue == NULL || env == NULL)
	{
		//return ERROR_NULL_ARG;
		return -1;
	}

    if (queue->head == NULL)
    {
        queue->head = env;
    }
    else
    {
        queue->tail->next = env;
    }

    queue->tail = env;
    queue->tail->next = NULL;

    return 0;
}

int MsgEnvQ_size(MsgEnvQ *queue)
{
    if (queue == NULL)
    {
        return -1;
    }

    MsgEnv * env = queue->head;
    int size = 0;
    while (env != NULL)
    {
        env = env->next;
        size++;
    }
    return size;
}
