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

int release_processor();
int request_process_status(MsgEnv *env);
int terminate();
int change_priority(int new_priority, int target_pid);

int get_trace_buffer(MsgEnv *env);
#endif
