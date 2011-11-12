#ifndef _RTX_INIT_
#define _RTX_INIT_

#include "rtx.h"

// A message list to keep track of all messages allocated at run time. Since the messages could be in multiple queues
// at once once the OS starts, this is a convenient way of freeing their memory in the cleanup function
MsgEnv* msg_list[MSG_ENV_COUNT];

int init_globals();
int init_all_lists();
int init_mmaps();
void cleanup();


#endif
