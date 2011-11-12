#ifndef _USERAPI_HEADER_
#define _USERAPI_HEADER_

#include "rtx.h"

/** 5.1 Interprocess Communication **/
int send_message(int dest_pid, MsgEnv *msg_env);
MsgEnv *receive_message();

/** 5.5 System Console I/O **/
int send_console_chars(MsgEnv *msg_env);
int get_console_chars(MsgEnv *msg_env);

int release_message_env(MsgEnv* env);
MsgEnv* request_msg_env();

#endif
