
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <signal.h>
#include "processQ.h"
#include <fcntl.h>

#include <sys/mman.h>
#include <sys/wait.h>

proc_queue * proc_q_create()
{
    proc_queue * queue = malloc(sizeof(*queue));
    if (queue)
    {
        queue->head = queue-> tail = NULL;
    }
    return queue;
}

void proc_q_destroy(proc_queue * queue)
{
    free(queue);
}

int proc_q_is_empty(proc_queue* queue)
{
    if (queue == NULL)
    {
        return -1;
    }

    return queue->head == NULL;
}

pcb * proc_q_dequeue(proc_queue* queue)
{
    if (queue == NULL || proc_q_is_empty(queue))
    {
        return NULL;
    }

    pcb * pcb = queue->head;
    if (queue->head == NULL)
    {
        queue->tail = NULL;
    }
    else
    {
        queue->head = queue->head->next;
    }
    pcb->next = NULL;
    return pcb;
}

int proc_q_enqueue(proc_queue * queue, pcb * pcb)
{
    if (queue == NULL || pcb == NULL)
    {
        return NULL_ARGUMENT;
    }

    assert(pcb->next == NULL);
    if (proc_q_is_empty(queue))
    {
        queue->head = pcb;
    }
    else
    {
        queue->tail->next = pcb;
    }
    queue->tail = pcb;
    queue->tail->next = NULL;

    return SUCCESS;
}

pcb * proc_q_remove(proc_queue * queue, pcb *pcb2)
{
    if (queue == NULL || pcb2 == NULL || proc_q_is_empty(queue))
    {
        return NULL;
    }

    if (queue->head == pcb2)
    {
        return proc_q_dequeue(queue);
    }

    pcb * prev_node = queue->head;
    pcb * curr_node = queue->head->next;
    while (curr_node)
    {
        if (curr_node == pcb2)
        {
            prev_node->next = curr_node->next;
            curr_node->next = NULL;
            return curr_node;
        }
        prev_node = curr_node;
        curr_node = curr_node->next;
    }

    return NULL;
}
