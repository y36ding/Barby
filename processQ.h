#ifndef _PROCESSQ_HEADER_
#define _PROCESSQ_HEADER_

#include "rtx.h"

proc_queue * proc_q_create();
void process_q_destroy(proc_queue * queue);

int process_q_is_empty(proc_queue * queue);
pcb * proc_queue_dequeue(proc_queue * queue);
int proc_q_enqueue(proc_queue * queue, pcb * pcb);
pcb * proc_q_remove(proc_queue * queue, pcb *pcb);

#endif
