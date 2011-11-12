
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <signal.h>
#include "procPQ.h"
#include <fcntl.h>

#include <sys/mman.h>
#include <sys/wait.h>

proc_pq * proc_pq_create(int num_priorities)
{
    if (num_priorities == 0)
    {
        return NULL;
    }

    proc_pq * ppq = malloc(sizeof(*ppq));
    if (ppq)
    {
        int allocation_error = 0;
        ppq->priority_queues = malloc(sizeof(*ppq->priority_queues) * num_priorities);
        if (ppq->priority_queues)
        {
            int priority;
            for (priority = 0; priority < num_priorities; priority++)
            {
                ppq->priority_queues[priority] = (proc_queue*)proc_q_create();
                allocation_error = allocation_error || !ppq->priority_queues[priority];
            }
        }
        else
        {
            allocation_error = 1;
        }
        ppq->num_priorities = num_priorities;

        if (allocation_error)
        {
            proc_pq_destroy(ppq);
            ppq = NULL;
        }
    }
    return ppq;
}

void proc_pq_destroy(proc_pq * ppq)
{
    if (ppq)
    {
        if (ppq->priority_queues)
        {
            int priority;
            for (priority = 0; priority < ppq->num_priorities; priority++)
            {
                 proc_q_destroy(ppq->priority_queues[priority]);
            }
            free(ppq->priority_queues);
        }
        free(ppq);
    }
}

pcb * proc_pq_dequeue(proc_pq *ppq)
{
    if (ppq != NULL)
    {
        int priority;
        for (priority = 0; priority < ppq->num_priorities; priority++)
        {
            if (!proc_q_is_empty(ppq->priority_queues[priority]))
            {
                return (pcb*)proc_q_dequeue(ppq->priority_queues[priority]);
            }
        }
    }

    return NULL;
}

int proc_pq_enqueue(proc_pq *ppq, pcb * pcb)
{
    if (ppq == NULL || pcb == NULL)
    {
        return NULL_ARGUMENT;
    }

    if (pcb->priority < 0 || pcb->priority >= ppq->num_priorities)
    {
        return ILLEGAL_ARGUMENT;
    }

    assert(!pcb->is_i_process);

    return proc_q_enqueue(ppq->priority_queues[pcb->priority], pcb);
}

int proc_pq_get_num_prorities(proc_pq *ppq)
{
    return ppq ? ppq->num_priorities : -1;
}

pcb * proc_pq_remove(proc_pq * ppq, pcb *pcb2)
{
    if (ppq != NULL && pcb2 != NULL &&
        pcb2->priority >= 0 && pcb2->priority < ppq->num_priorities)
    {
        return (pcb*)proc_q_remove(ppq->priority_queues[pcb2->priority], pcb2);
    }

    return NULL;
}
