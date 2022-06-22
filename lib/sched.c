#include <env.h>
#include <pmap.h>
#include <printf.h>

/* Overview:
 *  Implement simple round-robin scheduling.
 *
 *
 * Hints:
 *  1. The variable which is for counting should be defined as 'static'.
 *  2. Use variable 'env_sched_list', which is a pointer array.
 *  3. CANNOT use `return` statement!
 */
/*** exercise 3.15 ***/
extern struct Env_list env_sched_list[];
extern struct Env *curenv;

void sched_yield(void)
{
	static int count = 1;	// remaining time slices of current env
	static int point = 0;	// current env_sched_list index
	/*  hint:
	 *  1. if (count==0), insert `e` into `env_sched_list[1-point]`
	 *     using LIST_REMOVE and LIST_INSERT_TAIL.
	 *  2. if (env_sched_list[point] is empty), point = 1 - point;
	 *     then search through `env_sched_list[point]` for a runnable env `e`, 
	 *     and set count = e->env_pri
	 *  3. count--
	 *  4. env_run()
	 *
	 *  functions or macros below may be used (not all):
	 *  LIST_INSERT_TAIL, LIST_REMOVE, LIST_FIRST, LIST_EMPTY
	 */
	struct Env *next_env;
	count--;
	int found;
	if (count == 0 || curenv == NULL || curenv->env_status != ENV_RUNNABLE) {
		found = 0;
		if (curenv != NULL) {
			LIST_REMOVE(curenv, env_sched_link);
			LIST_INSERT_TAIL(&env_sched_list[!point], curenv,
					 env_sched_link);
		}
		while (1) {
			if (LIST_EMPTY(&env_sched_list[point])) {
				point = !point;
				break;
			}
			next_env = LIST_FIRST(&env_sched_list[point]);
			if (next_env->env_status == ENV_RUNNABLE) {
				found = 1;
				break;
			}
			LIST_REMOVE(next_env, env_sched_link);
			LIST_INSERT_HEAD(&env_sched_list[!point], next_env,
					 env_sched_link);
		}
		if (!found) {
			while (1) {
				if (LIST_EMPTY(&env_sched_list[point])) {
					//panic("^^^^^^No env is RUNNABLE!^^^^^^");
				}
				next_env = LIST_FIRST(&env_sched_list[point]);
				if (next_env->env_status == ENV_RUNNABLE) {
					found = 1;
					break;
				}
				LIST_REMOVE(next_env, env_sched_link);
				LIST_INSERT_TAIL(&env_sched_list[!point],
						 next_env, env_sched_link);
			}
		}
		count = next_env->env_pri;
		env_run(next_env);
	}
	env_run(curenv);
	panic("^^^^^^sched yield reached end^^^^^^");
}