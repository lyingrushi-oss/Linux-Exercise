#define _POSIX_C_SOURCE 200809L

#include <unistd.h>
#include <stdio.h>
#include <signal.h>

static volatile sig_atomic_t stop_request =0;

static void handle_signal(int signo)
{
 stop_request=signo;
}

int main(void)
{
 struct sigaction action={0};
action.sa_handler= handle_signal;
sigemptyset(&action.sa_mask);
action.sa_flags=0;
if(sigaction(SIGINT,&action,NULL)==-1){
perror("sigaction SIGINT");
return 1;
}
if(sigaction(SIGTERM,&action,NULL)==-1){
perror("sigaction SIGTREM");
return 1;

}

printf("pid=%ld\n",(long)getpid());
printf("waiting for SIGTREM or SIGINT\n");

while(!stop_request)
{
printf("working....\n");
sleep(1);
}

printf("received signal:%d\n",stop_request);
printf("program normally exited\n");
return 0;
}
