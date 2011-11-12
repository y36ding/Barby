#ifndef _TIMERQ_HEADER_
#define _TIMERQ_HEADER_

#include "rtx.h"

int timeout_q_is_empty();
void timeout_q_insert (MsgEnv* new_msg_env);
MsgEnv * check_timeout_q();
MsgEnv * get_timeout_q();

#endif
