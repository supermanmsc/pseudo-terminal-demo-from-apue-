/*************************************************************************
	> File Name: loop.c
	> Author:
	> Mail:
	> Created Time: 2015年12月23日 星期三 20时51分17秒
 ************************************************************************/

#include<stdio.h>
#include <signal.h>
#include <stdlib.h>
#include "apue.h"
#include "loop.h"
#define BUFFSIZE 512
static void sig_term(int);
static volatile sig_atomic_t sigcaught;
void loop(int ptym,int ignoreeof){
    pid_t child;
    int nread;
    char buf[BUFFSIZE];
    if((child = fork()) < 0){
        perror("fork error");
    }else if(child == 0){
        for(;;){
            if((nread = read(STDIN_FILENO,buf,BUFFSIZE)) < 0)
                perror("read error from stdin");
            else if(nread == 0)
                break;
            if(write(ptym,buf,nread) != nread)
                perror("write error to master pty");
        }
        if(ignoreeof == 0)
            kill(getppid(),SIGTERM);
        exit(0);
    }
    //if(signal_intr(SIGTERM,sig_term) == SIG_ERR)
    //    perror("signal_intr error");
    for(;;){
        if((nread = read(ptym,buf,BUFFSIZE)) <= 0)
            break;
        if(write(STDOUT_FILENO,buf,nread)!= nread)
            perror("writen error to stdout");
    }
    if(sigcaught == 0)
        kill(child,SIGTERM);
}
static void sig_term(int signo){
    sigcaught = 1;
}
void tty_atexit(){
    ;
}
int tty_raw(int fd){
    return 0;
}
