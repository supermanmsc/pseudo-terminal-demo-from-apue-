#ifndef _PTY_FORK_H
#define _PTY_FORK_H
#include <termios.h>
#include <sys/ioctl.h>
#include "apue.h"
pid_t pty_fork(int *ptrfdm,char *slave_name,int slave_names,const struct termios *slave_termios,const struct winsize *slave_winsize);
#endif //_PTY_FORK_H
