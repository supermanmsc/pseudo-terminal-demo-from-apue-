#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#include <termios.h>
#include "apue.h"
#ifndef IIOCGWINSZ
#include <sys/ioctl.h>
#endif
//#define TIOCGPTN    _IOR('T',0x30,  unsigned int) /* Get Pty Number (of pty-mux device) */
//#define TIOCSPTLCK    _IOW('T',0x31, int)  /* Lock/unlock Pty */

#ifndef _HAS_OPENPT
int posix_openpt(int oflag)
{
    int     fdm;
    fdm = open("/dev/ptmx",oflag);
    return fdm;
}
#endif

#ifndef _HAS_PTSNAME
char *ptsname(int fdm)
{
    static char     pts_name[16];
    int             sminor;
    if(ioctl(fdm,TIOCGPTN,&sminor) < 0)
        return NULL;
    snprintf(pts_name,sizeof(pts_name),"/dev/pts/%d",sminor);
    return pts_name;
}
#endif

#ifndef _HAS_GRANTPT
int grantpt(int fdm)
{
    char *pts_name;
    pts_name = ptsname(fdm);
    return chmod(pts_name,S_IRUSR | S_IWUSR | S_IWGRP);
}
#endif

#ifndef _HAS_UNLOCKPT
int unlockput(int fdm)
{
    int lock = 0;
    return (ioctl(fdm,TIOCSPTLCK,&lock));
}
#endif
int ptym_open(char *pts_name,int pts_namesz)
{
    char    *ptr;
    int     fdm;
    strncpy(pts_name,"/dev/ptmx",pts_namesz);
    pts_name[pts_namesz-1] = '\0';
    if((fdm = posix_openpt(O_RDWR)) < 0)
        return -1;
    if(grantpt(fdm) < 0)
    {
        close(fdm);
        return -2;
    }
    if(unlockput(fdm) < 0)
    {
        close(fdm);
        return -3;
    }
    if((ptr = ptsname(fdm)) < 0)
    {
        close(fdm);
        return -4;
    }
    strncpy(pts_name,ptr,pts_namesz);
    pts_name[pts_namesz-1] = '\0';
    return fdm;
}
int ptys_open(char *pts_name)
{
    int     fds,setup;
    if((fds = open(pts_name,O_RDWR)) < 0)
        return -5;
    return fds;
}

pid_t    pty_fork(int *ptrfdm,char *slave_name,int slave_namesz,
               const struct termios *slave_termios,
               const struct winsize *slave_winsize)
{
    int     fdm,fds;
    pid_t   pid;
    char    pts_name[20];

    if((fdm=ptym_open(pts_name,sizeof(pts_name))) < 0)
    {
        perror("ptym_open()error");
        exit(-1);
    }
    if(slave_name != NULL)
    {
        strncpy(slave_name,pts_name,slave_namesz);
        slave_name[slave_namesz-1] = '\0';
    }
    if((pid = fork()) < 0)
    {
        perror("fork() error");
        exit(-1);
    }
    else if(pid == 0)
    {
        if(setsid() < 0)
        {
            perror("setsid() error");
            exit(-1);
        }
        if((fds = ptys_open(pts_name)) < 0)
        {
            perror("ptys_open() error");
            exit(-1);
        }
        close(fdm);
    #if defined (TIOCSCTTY)
        if(ioctl(fds,TIOCSCTTY,(char *)0) < 0)
        {
            perror("TIOCSCTTY error");
            exit(-1);
        }
    #endif
        if(slave_termios != NULL)
        {
            if(tcsetattr(fds,TCSANOW,slave_termios) < 0)
            {
                perror("tcsetattr error on slave pty");
                exit(-1);
            }
        }
        if(slave_winsize != NULL)
        {
            if(ioctl(fds,TIOCSWINSZ,slave_winsize) < 0)
            {
                perror("TIOCSWINSZ error on slave pty");
                exit(-1);
            }
        }

        if(dup2(fds,STDIN_FILENO) != STDIN_FILENO)
        {
            perror("dups error to stdin");
            exit(-1);
        }
        if(dup2(fds,STDOUT_FILENO) != STDOUT_FILENO)
        {
            perror("dups error to stdout");
            exit(-1);
        }
        if(dup2(fds,STDERR_FILENO) != STDERR_FILENO)
        {
            perror("dups error to stderr");
            exit(-1);
        }
        if(fds != STDIN_FILENO && fds != STDOUT_FILENO && fds != STDERR_FILENO)
            close(fds);
        return 0;
    }
    else
    {
        *ptrfdm = fdm;
        return pid;
    }
}

