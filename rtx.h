#ifndef _RTX_HEADER_
#define _RTX_HEADER_

#include <assert.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>

#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include "iProcs.h"
#include "kbcrt.h"
#include <setjmp.h>

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>


// Process IDs
#define KB_I_PROCESS_ID        0
#define CRT_I_PROCESS_ID       1
#define P_PROCESS_ID 		   2
#define NULL_PROCESS_ID 666

// RTX Constants
#define MSG_ENV_SIZE 100
#define MSG_ENV_COUNT 50
#define NUM_PRIORITY_LEVEL 5
#define PROCESS_COUNT 3
#define STACK_SIZE 100

// error codes
#define SUCCESS 0
#define NULL_ARGUMENT -1
#define ILLEGAL_ARGUMENT -2
#define OTHER_ERROR -3

typedef int bool;
#define TRUE 1
#define FALSE 0

// atomic flags
#define ON TRUE
#define OFF FALSE

#define DEBUG 0

void die(int signal);

typedef void (*pc)();

void processP();

typedef enum msg_type {
    CONSOLE_INPUT, DISPLAY_ACK, COUNT_REPORT, WAKEUP10
}MsgType;

typedef struct MsgEnv {
   struct MsgEnv *next;
    int 	dest_pid;
    int 	sender_pid;
    MsgType 	msg_type;
    char    *data;
} MsgEnv;

typedef enum process_states {
    READY, EXECUTING, BLOCKED_ON_ENV_REQUEST, BLOCKED_ON_RCV, NEVER_BLK_RCV,
    SINTERRUPTED
} ProcessState;

typedef struct msg_env_queue {
	MsgEnv* head;
	MsgEnv* tail;
}MsgEnvQ;

typedef struct process_control_block {
	int pid;
	int priority;
	ProcessState state;
	char* name;
	MsgEnvQ*  rcv_msg_queue;
	struct process_control_block* next;
	jmp_buf* buf;
	pc pc_location;
    char * stack;
	int a_count;
	bool is_i_process;
	struct process_control_block* next_pcb;
} pcb;

typedef struct init_proc
{
	char* name;
	int pid;
	int priority;
	bool is_i_process;
	pc pc_location;
}InitProc;

typedef struct trace_log{
	int source_pid; 	//sender‟s process id
	int destination_pid; 	//receiver‟s process id
	MsgType message_type; // message type
	int time_stamp; //the time the message is sent or received
} TraceLog;

typedef struct trace_buffer {
	int head; //head index
	int tail; //tail index
	TraceLog buff[16]; // buffer data of 16 TraceLogs
} TraceBuffer;

// global variables
pcb* current_process;
pcb* prev_process;
MsgEnvQ* free_env_queue;
pcb* pcb_list[PROCESS_COUNT];
MsgEnvQ* displayQ;


// globals
inputbuf * in_mem_p_key;	// pointer to structure that is the shared memory
outputbuf* in_mem_p_crt;	// pointer to structure that is the shared memory
int in_pid_keyboard;		// pid of keyboard child process
int in_pid_crt;				// pid of crt child process
caddr_t mmap_ptr_keyboard;
caddr_t mmap_ptr_crt;
int bufsize;
int fid, fid2, status;		//used to create the shared memory

int numOfTicks;
int displayClock;

TraceBuffer send_trace_buf;
TraceBuffer receive_trace_buf;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
char * sfilename;
char * cfilename;
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif

