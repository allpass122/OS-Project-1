#define _GNU_SOURCE
#define __USE_GNU
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sched.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "heap.h"

#define MAX_PROCESS_num 10000
#define MAX_NAME_LENGTH 16
#define TIME_UNIT 1000000UL

void unitime(){
	volatile unsigned long i;
	for(i=0;i<TIME_UNIT;i++);
}


int n_process;
pid_t pid[MAX_PROCESS_num];
int ptr_ready=0;
int ptr_run=0;
int run=0;

int nFinish = 0;
int now_time = 0;
int counting_time = 0;
struct Node *heap_root = NULL;

char N[MAX_PROCESS_num][MAX_NAME_LENGTH];
int R[MAX_PROCESS_num];
int T[MAX_PROCESS_num];
int idx[MAX_PROCESS_num];

# define BLOCK_SIGCHLD() 	\
	sigset_t set, oset;		\
	do{						\
		sigfillset(&set);	\
		sigprocmask(SIG_BLOCK, &set, &oset);\	
	}while(0)

#define UNBLOCK_SIGCHLD() sigprocmask(SIG_SETMASK, &oset, NULL)

#define WAIT_CHILD() do { \
	BLOCK_SIGCHLD(); \
	while(heap_root != NULL && waitpid(pid[heap_root->index],NULL,WNOHANG)!=0){ \
		++nFinish; \
		heap_root = pop_heap(heap_root); \
		run = 0; \
		if(nFinish == n_process) exit(0); \
	} \
	UNBLOCK_SIGCHLD();\
	}while(0)

static int cmp1(const void *p1, const void *p2){
	int a1 = *(int *)p1;
	int a2 = *(int *)p2;
	if(T[a1] > T[a2]) return 1;
	else if(T[a1] < T[a2]) return -1;
	else return 0;
}
static int cmp(const void *p1, const void *p2){
	int a1 = *(int *)p1;
	int a2 = *(int *)p2;
	if(R[a1] > R[a2]) return 1;
	else if(R[a1] < R[a2]) return -1;
	else return 0;
}

void handle(int signum){
	wait(NULL);
	++nFinish;
	heap_root = pop_heap(heap_root);
	run = 0;
	//printf("~~%d %d\n",nFinish,n_process);
	if(nFinish == n_process) exit(0);
}
void decrease(){
	BLOCK_SIGCHLD();
	if(!run || heap_root == NULL)
		return;
	int i1 = heap_root->index;
	heap_root = pop_heap(heap_root);
	T[i1] -= counting_time;
	WAIT_CHILD();
	if(heap_root){
		int i2 = heap_root->index;
		//set prior
		struct sched_param param;
		param.sched_priority = 20;
		sched_setscheduler(pid[i2], SCHED_FIFO, &param);
	}
	//set prior
	struct sched_param param;
	param.sched_priority = 20;
	sched_setscheduler(pid[i1], SCHED_FIFO, &param);
	heap_root = insert_heap(heap_root, T[i1], i1);
	counting_time = 0;
	UNBLOCK_SIGCHLD();
}
void run_next(){
	BLOCK_SIGCHLD();
	int next = heap_root->index;
	//set prior
	struct sched_param param;
	param.sched_priority = 80;
	sched_setscheduler(pid[next], SCHED_FIFO, &param);
	run = 1;
	counting_time = 0;
	UNBLOCK_SIGCHLD();
}
void increase_next(){
	BLOCK_SIGCHLD();
	int i1 = heap_root->index;
	heap_root = pop_heap(heap_root);
	//set prior
	struct sched_param param;
	param.sched_priority = 80;
	sched_setscheduler(pid[i1], SCHED_FIFO, &param);
	WAIT_CHILD();
	if(heap_root){
		int i2 = heap_root->index;
		//set prior
		struct sched_param param;
		param.sched_priority = 50;
		sched_setscheduler(pid[i2], SCHED_FIFO, &param);
	}
	heap_root = insert_heap(heap_root, T[i1], i1);
	UNBLOCK_SIGCHLD();
}
void change_priority(){
	//have task can run
	WAIT_CHILD();
	BLOCK_SIGCHLD();
	if(!run && heap_root != NULL)
		run_next();
	if(run && heap_root != NULL)
		increase_next();
	UNBLOCK_SIGCHLD();
}


int main(){
	scanf("%d",&n_process);	
	for(int i=0;i<n_process;i++){
		scanf("%s %d %d",N[i],&R[i],&T[i]);
	}
	for(int i=0;i<n_process;i++){
		idx[i] = i;
	}
	qsort(idx, n_process, sizeof(int), &cmp1);
	qsort(idx, n_process, sizeof(int), &cmp);

	signal(SIGCHLD, handle);
	int next = idx[ptr_ready];

	for(int time=0;;time++){
		change_priority();
		while( ptr_ready < n_process && time == R[next] ){
			decrease();
			//create process
			pid[next] = fork();
			if(pid[next] == 0){
				char tmp1[20],tmp2[20];
				sprintf(tmp1,"%d",next);
				sprintf(tmp2,"%d",T[next]);
				execl("./process", "./process",N[next],tmp1,tmp2, (char *)0);
			}
			if(pid[next] > 0){
				fflush(stderr);
				//set prior
				struct sched_param param;
				param.sched_priority = 20;
				sched_setscheduler(pid[next], SCHED_FIFO, &param);	
				cpu_set_t mask;
				CPU_ZERO(&mask);
				CPU_SET(1,&mask);
				sched_setaffinity(pid[next], sizeof(mask), &mask);
			}	
			heap_root = insert_heap(heap_root,T[next],next);
			next = idx[++ptr_ready];
			change_priority();
		}
		unitime();
		counting_time++;
		now_time++;
		WAIT_CHILD();
	}
}
