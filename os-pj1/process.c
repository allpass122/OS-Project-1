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
#include <sys/syscall.h>
#include <sys/time.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

#define MAX_PROCESS_num 10000
#define MAX_NAME_LENGTH 16
#define TIME_UNIT 1000000UL

void unitime(){
	volatile unsigned long i;
	for(i=0;i<TIME_UNIT;i++);
}
extern int errno;
int main(int argc, char **argv){
	static const long BASE = 1e6;
	printf("%s %d\n", argv[1], getpid());
	int idx, T;
	sscanf(argv[2]," %d",&idx);
	sscanf(argv[3]," %d",&T);
	
	struct timeval t,t1; 
	gettimeofday(&t,NULL);
	//long start = syscall(334);
	//long long start = t.tv_sec*BASE+t.tv_usec;
	//printf("start:%d\n",start);
	for(int i=0;i<T;i++){
		unitime();
	}
	gettimeofday(&t1,NULL);
	//long end = syscall(334);
	//long long end = t.tv_sec*BASE+t.tv_usec;
	//printf("end:%d\n",end);
	//fprintf(stderr,"%s\n",strerror(errno));
	//syscall(333 , getpid(), start, end );
	char string[100];
	sprintf(string, "[Project1] %d %ld.%06ld %ld.%06ld\n", getpid(), t.tv_sec, t.tv_usec, t1.tv_sec, t1.tv_usec);
	int fd = open("/dev/kmsg",O_RDWR);
	write(fd,string,strlen(string)+1);
	//fprintf(stderr, "[Project1] %d %ld.%06ld %ld.%06ld\n", getpid(), start / BASE, start % BASE, end / BASE, end % BASE);
	fprintf(stderr, "[Project1] %d %ld.%06ld %ld.%06ld\n", getpid(),t.tv_sec, t.tv_usec, t1.tv_sec, t1.tv_usec);
	fclose(fd);

}



