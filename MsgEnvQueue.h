#ifndef _MSGENVQ_HEADER_
#define _MSGENVQ_HEADER_

#include "rtx.h"

MsgEnvQ * MsgEnvQ_create();
void MsgEnvQ_destroy(MsgEnvQ * queue);
int MsgEnvQ_is_empty(MsgEnvQ* queue);
MsgEnv* MsgEnvQ_dequeue(MsgEnvQ *queue);
int MsgEnvQ_enqueue(MsgEnvQ *queue, MsgEnv* env);
int MsgEnvQ_size(MsgEnvQ *queue);

#endif
