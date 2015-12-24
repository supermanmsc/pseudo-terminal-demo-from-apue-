/*************************************************************************
    > File Name: main.c
    > Author:
    > Mail:
    > Created Time: 2015年12月24日 星期四 10时14分22秒
 ************************************************************************/

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#ifndef TIOCGWINSZ
#include <sys/ioctl.h>
#endif

#ifdef LINUX
#define OPTSTR "+d:einv"
#else
#define OPTSTR "d+:einv"
#endif

#include "pty_fork.h"
#include "apue.h"
#include "loop.h"

static void set_noecho(int);
void do_driver(char* driver){
    pid_t child;
    int pipefd[2];
    if(pipe(pipefd) < 0)
        perror("can't craete stream pipe");
    if((child = fork()) < 0)
        perror("fork error");
    else if(child == 0){
        close(pipefd[1]);
        if(dup2(pipefd[0],STDIN_FILENO) != STDIN_FILENO)
            perror("dup2 error to stdin");
        if(dup2(pipefd[0],STDOUT_FILENO) != STDOUT_FILENO)
            close(pipefd[0]);
        execlp(driver,driver,(char*)0);
        err_sys("execlp error for:%s",driver);
    }
    close(pipefd[0]);
    if(dup2(pipefd[1],STDIN_FILENO) != STDIN_FILENO)
        perror("dup2 error to stdin");
    if(pipefd[1] !=STDIN_FILENO && pipefd[1] != STDOUT_FILENO)
        close(pipefd[1]);
}

int main(int argc,char* argv[]){
    int fdm,c,ignoreeof,interactive,noecho,verbose;
    pid_t pid;
    char *driver;
    char slave_name[20];
    struct termios orig_termios;
    struct winsize size;

    interactive = isatty(STDIN_FILENO);
    ignoreeof = 0;
    noecho = 0;
    verbose = 0;
    driver = NULL;

    opterr = 0;
    while((c = getopt(argc,argv,OPTSTR)) != EOF){
        switch(c){
            case 'd':
                driver = optarg;
                break;
            case 'e':
                noecho = 1;
                break;
            case 'i':
                ignoreeof = 1;
                break;
            case 'n':
                interactive = 0;
                break;
            case 'v':
                verbose = 1;
                break;
            case '?':
                printf("can't understand input character\n");
                exit(1);
        }
    }
    if(optind>=argc)
        printf("usage :pty no usage\n");
    if(interactive){
        if(tcgetattr(STDIN_FILENO,&orig_termios) < 0){
            perror("tcgetattr error");
        }
        if(ioctl(STDIN_FILENO,TIOCGWINSZ,(char*)&size) < 0){
            perror("ioctl(stdout_fileno tiocgwinsz) error");
        }
        pid = pty_fork(&fdm,slave_name,sizeof(slave_name),NULL,NULL);
    }else{
        pid = pty_fork(&fdm,slave_name,sizeof(slave_name),NULL,NULL);
    }

    if(pid < 0){
        perror("fork error");
    }else if(pid == 0){
        if(noecho)
            set_noecho(STDIN_FILENO);
        if(execvp(argv[optind],&argv[optind]) < 0)
            err_sys("can't execute : %s\n",argv[optind]);
    }

    if(verbose){
        fprintf(stderr,"slave name = %s\n",slave_name);
        if(driver != NULL)
            fprintf(stderr,"driver = %s\n",driver);
    }

    if(interactive && driver == NULL){
        if(tty_raw(STDIN_FILENO) < 0)
            perror("tty raw error");
            if(atexit(tty_atexit) < 0)
                perror("atexit error");
        if(driver)
            do_driver(driver);
        loop(fdm,ignoreeof);
        exit(0);
    }
}

static void set_noecho(int fd){
    struct termios stermios;
    if(tcgetattr(fd,&stermios) < 0)
        perror("tcgetattr error");
    stermios.c_lflag &= ~(ECHO|ECHOE|ECHOK|ECHONL);
    stermios.c_oflag &= ~(ONLCR);
    if(tcsetattr(fd,TCSANOW,&stermios) < 0)
        perror("tcgetattr error");
}
