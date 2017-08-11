#ifndef __DESim_H__
#define __DESim_H__

typedef char boolean;

#ifdef TIME_64
typedef unsigned long long Time_t;
#else
typedef long long Time_t;
#endif

# if __WORDSIZE == 64
typedef long int __jmp_buf[8];
# elif defined  __x86_64__
__extension__ typedef long long int __jmp_buf[8];
# else
typedef int __jmp_buf[6];
# endif

typedef __jmp_buf jmp_buf;

/*our assembly fucntions (.s) files these store
and load CPU register values. For DESim
the stack pointer and instruction pointer are
all we really care about.*/

#if defined(__linux__) && defined(__i386__)

int setjmp32_2(jmp_buf __env);
void longjmp32_2(jmp_buf __env, int val);

int encode32(int val);
int decode32(int val);

#elif defined(__linux__) && defined(__x86_64)

int setjmp64_2(jmp_buf __env);
void longjmp64_2(jmp_buf __env, int val);

long long encode64(long long val);
long long decode64(long long val);


#else
#error Unsupported machine/OS combination
#endif

typedef Time_t count_t;
typedef struct eventcount_s eventcount;
typedef struct context_t context;
typedef struct task_s task;
typedef struct process_record process;

extern process *current_process;
extern process *terminated_process;

//id for eventcounts
jmp_buf main_context;
extern long long ecid;
extern count_t last_value;


void desim_init(void);
void end_simulate(void);
int desim_end(void);


#endif /*__DESim_H__*/
