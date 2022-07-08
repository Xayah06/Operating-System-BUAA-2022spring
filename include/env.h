/* See COPYRIGHT for copyright information. */

#ifndef _ENV_H_
#define _ENV_H_

#include "types.h"
#include "queue.h"
#include "trap.h"
#include "mmu.h" 

#define LOG2NENV	10
#define LOG2THREAD_NUM_MAX	3
#define NENV		(1<<LOG2NENV)
#define ENVX(envid)	((envid) & (NENV - 1))
#define THREADX(threadid)	((threadid) & (THREAD_NUM_MAX - 1))
#define GET_ENV_ASID(envid) (((envid)>> 11)<<6)

// Values of env_status in struct Env
#define ENV_FREE	0
#define ENV_RUNNABLE		1
#define ENV_NOT_RUNNABLE	2

#define THREAD_NUM_MAX	(1<<LOG2THREAD_NUM_MAX)
#define THREAD_CANCEL_ENABLE 		1
#define THREAD_CANCEL_DISABLE	0
#define SEM_FREE	0
#define SEM_VALID	1


LIST_HEAD(Env_list, Env);
LIST_HEAD(Thread_list, Thread);

struct Thread {
	// run/yield information
	struct Trapframe thread_tf;
	u_int thread_id;
	u_int thread_status;
	u_int thread_pri;
    u_int env_id; 

	// join information
	LIST_ENTRY(Thread) thread_wait_link;
	LIST_ENTRY(Thread) thread_sched_link;
	LIST_ENTRY(Thread) thread_joined_link;
	struct Thread_list thread_joined_list;
	void **thread_join_value_ptr;
	u_int thread_detach;

	// exit information
	int thread_exit_value;
	void *thread_exit_ptr;

	// cancel information
	int thread_canceltype;
	u_int thread_canceled;
};


struct Env {
	LIST_ENTRY(Env) env_link;       // Free list
	u_int env_id;                   // Unique environment identifier
	u_int env_parent_id;            // env_id of this env's parent
	Pde  *env_pgdir;                // Kernel virtual address of page dir
	u_int env_cr3;
    u_int env_pri;
	// Lab 4 IPC
	u_int env_ipc_value;            // data value sent to us 
	u_int env_ipc_from;             // envid of the sender  
	u_int env_ipc_recving;          // env is blocked receiving
	u_int env_ipc_dstva;		// va at which to map received page
	u_int env_ipc_perm;		// perm of page mapping received

	// Lab 4 fault handling
	u_int env_pgfault_handler;      // page fault state
	u_int env_xstacktop;            // top of exception stack

	// Lab 6 scheduler counts
	u_int env_runs;			// number of times been env_run'ed
	u_int env_nop;                  // align to avoid mul instruction

    // POSIX thread
	u_int env_thread_count;
	u_int env_ipc_waiting_thread_no;
	struct Thread env_threads[THREAD_NUM_MAX];
};

struct Semaphore {
	u_int sem_envid;
	int sem_value;
	int sem_status;
	int sem_shared;
	int sem_wait_num;
	struct Thread_list sem_wait_list;
};

extern struct Env *envs;		// All environments
extern struct Env *curenv;	        // the current env
extern struct Env_list env_sched_list[2]; // runnable env list
extern struct Thread *curthread;
extern struct Thread_list thread_sched_list[2];

void env_init(void);
int env_alloc(struct Env **e, u_int parent_id);
void env_free(struct Env *);
void env_create_priority(u_char *binary, int size, int priority);
void env_create(u_char *binary, int size);
void env_destroy(struct Env *e);

int envid2env(u_int envid, struct Env **penv, int checkperm);
void env_run(struct Thread *e);

int thread_alloc(struct Env *e, struct Thread **t);
int threadid2thread(u_int threadid, struct Thread **pthread);

// for the grading script
#define ENV_CREATE2(x, y) \
{ \
	extern u_char x[], y[]; \
	env_create(x, (int)y); \
}
#define ENV_CREATE_PRIORITY(x, y) \
{\
        extern u_char binary_##x##_start[]; \
        extern u_int binary_##x##_size;\
        env_create_priority(binary_##x##_start, \
                (u_int)binary_##x##_size, y);\
}
#define ENV_CREATE(x) \
{ \
	extern u_char binary_##x##_start[];\
	extern u_int binary_##x##_size; \
	env_create(binary_##x##_start, \
		(u_int)binary_##x##_size); \
}

#endif // !_ENV_H_
