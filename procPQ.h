#ifndef _PROCPQ_HEADER_
#define _PROCPQ_HEADER_

#include "rtx.h"

proc_pq * proc_pq_create(int num_priorities);
void proc_pq_destroy(proc_pq *ppq);

pcb * proc_pq_dequeue(proc_pq *ppq);
int proc_pq_enqueue(proc_pq *ppq, pcb * pcb);

int proc_pq_get_num_prorities(proc_pq *ppq);

pcb * proc_pq_remove(proc_pq * ppq, pcb *pcb);

#endif
