#ifndef _IPROCS_INIT_
#define _IPROCS_INIT_

#include "rtx.h"

void kbd_i_proc(int signum);
void crt_i_proc(int signum);
void timer_i_proc(int signum);

//clock process
int clock_get_time();
void clock_inc_time();
void clock_set_time(int time);

#endif
