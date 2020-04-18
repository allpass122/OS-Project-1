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

#define MAX_PROCESS_num 10000
#define MAX_NAME_LENGTH 16
#define TIME_UNIT 1000000UL

#define MAX_QUEUE_SIZE (1000000+7)
#define TIME_QUANTUM 500

void unitime(){
	volatile unsigned long i;
	for(i=0;i<TIME_UNIT;i++);
}

int n_process;
pid_t pid[MAX_PROCESS_num];
int ptr_ready=0;
int ptr_run=0;
int run=0;

char N[MAX_PROCESS_num][MAX_NAME_LENGTH];
int R[MAX_PROCESS_num];
int T[MAX_PROCESS_num];
int idx[MAX_PROCESS_num];
static int cmp(const void *p1, const void *p2){
	int a1 = *(int *)p1;
	int a2 = *(int *)p2;
	if(R[a1] > R[a2]) return 1;
	else if(R[a1] < R[a2]) return -1;
	else return 0;
}

int counting_time = 0, block_flag = 0;
int que[MAX_QUEUE_SIZE], ptrFront=0, ptrBack=0;

void push(int x){
	que[ptrBack++] = x;
}
int front(){
	return que[ptrFront];
}
int size(){
	return ptrBack-ptrFront; 
}
int empty(){
	return ptrFront < ptrBack ? 0:1;
}
void pop(){
	if(ptrFront < ptrBack)
		ptrFront++;
}

void handle(int signum){
	static int nFinish = 0;
	wait(NULL);
	++nFinish;
	pop();
	run = 0;
	block_flag = 0;
	counting_time = 0;
	if(nFinish == n_process) exit(0);
}

void run_next(){
	counting_time = 0;
	int next = que[ptrFront];
	if(T[next] <= TIME_QUANTUM)
		block_flag = 1;
	//set prior
	struct sched_param param;
	param.sched_priority = 80;
	sched_setscheduler(pid[next], SCHED_FIFO, &param);
	run = 1;
}
void increase_next(){
	int next = que[ptrFront+1];
	//set prior
	struct sched_param param;
	param.sched_priority = 50;
	sched_setscheduler(pid[next], SCHED_FIFO, &param);	
}
void change_priority(){
	//have task can run
	if(!run && size() >= 1)
		run_next();
	if(run && size() >= 2)
		increase_next();
}

int main(){
	scanf("%d",&n_process);	
	for(int i=0;i<n_process;i++){
		scanf("%s %d %d",N[i],&R[i],&T[i]);
	}
	for(int i=0;i<n_process;i++){
		idx[i] = i;
	}
	qsort(idx, n_process, sizeof(int), &cmp);
	
	signal(SIGCHLD, handle);
	int next = idx[ptr_ready];
	
	for(int time=0;;time++){
		change_priority();
		while( ptr_ready < n_process && time == R[next] ){
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
			push(next);
			next = idx[++ptr_ready];
			change_priority();
		}
		unitime();
		counting_time++;
		if(!block_flag && run && counting_time == TIME_QUANTUM){
			int id = front();
			pop();
			T[id] -= TIME_QUANTUM;
			
			//set prior
			struct sched_param param;
			param.sched_priority = 20;
			sched_setscheduler(pid[id], SCHED_FIFO, &param);	
			
			push(id);
			run=0;
			change_priority();

		}
	}
}
