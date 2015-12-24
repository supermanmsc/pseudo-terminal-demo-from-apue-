/*************************************************************************
	> File Name: loop.c
	> Author:
	> Mail:
	> Created Time: 2015年12月23日 星期三 20时51分17秒
 ************************************************************************/
#ifndef __LOOP_H
#define __LOOP_H

#include<stdio.h>
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
static void sig_term(int);
void loop(int ptym,int ignoreeof);
void tty_atexit();
int  tty_raw(int);
#endif //__LOOP_H
