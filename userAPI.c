#include "userAPI.h"
#include "kernal.h"

int send_message(int dest_process_id, MsgEnv *msg_envelope)
{
	atomic(ON);
	int ret = k_send_message(dest_process_id, msg_envelope);
	atomic(OFF);
	return ret;
}

MsgEnv *receive_message()
{
	atomic(ON);
	MsgEnv* ret = k_receive_message();
	atomic(OFF);
	return ret;
}

int send_console_chars(MsgEnv *message_envelope)
{
	atomic(ON);
	int ret = k_send_console_chars(message_envelope);
	atomic(OFF);
	return ret;
}

int get_console_chars(MsgEnv *message_envelope)
{
	atomic(ON);
	int ret = k_get_console_chars(message_envelope);
	atomic(OFF);
	return ret;
}


int release_message_env(MsgEnv* env)
{
	atomic(ON);
	int ret = k_release_message_env(env);
	atomic(OFF);
	return ret;

}

MsgEnv* request_msg_env(){
	atomic(ON);
	MsgEnv* ret = k_request_msg_env();
	atomic(OFF);
	return ret;
}

