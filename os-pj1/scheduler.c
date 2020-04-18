#define _GNU_SOURCE
#define __USE_GNU
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sched.h>
#include <unistd.h>
#include <sys/types.h>

#define MAX_PROCESS_num 10000
#define MAX_NAME_LENGTH 16
#define MAXPROCESS_NAME 40
#define TIME_UNIT 1000000UL
//nice -n 3 command

int main(){
	char policy[10];
	assert( scanf("%s", policy) == 1 );
	//use-cpu
	pid_t pid = getpid();
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(0, &mask);
	if( sched_setaffinity(pid, sizeof(mask), &mask  ) != 0){
		fprintf(stderr, "sched_setaffinity error\n");
	}
	
	//set-priority
	struct sched_param param;
	param.sched_priority = 50;

	fflush(stdin);

	if( sched_setscheduler(pid, SCHED_FIFO, &param) != 0  ){
		fprintf(stderr, "sched_setscheduler error\n");
	}
	
	if(policy[0] == 'F'){
		execl("./FIFO", "./FIFO", (char *)0);
	}
	else if(policy[0] == 'R'){
		execl("./RR", "./RR", (char *)0);
	}
	else if(policy[0] == 'S'){
		execl("./SJF", "./SJF",(char *)0);
	}
	else if(policy[0] == 'P'){
		execl("./PSJF", "./PSJF",(char *)0);
	}
	return 0;
}
