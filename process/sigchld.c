#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
 
void sys_err(char *str)
{
    perror(str);
    exit(1);
}
 
void do_sig_child(int signo)
{
    int status;
    pid_t pid;
// 不可以将捕捉函数内部的while替换为if。因为在执行捕捉函数期间，发送了多次SIGCHLD信号，未决信号集只是记录了一次// 因此下一次再调用捕捉函数时，if只能完成对一个子进程的回收（即使有多个子进程都发了信号，但是只是调用一次捕捉函数）。// 而while循环则可以对所有结束了的子进程都完成回收。因此对于多个子进程的回收，最好采用循环的方式，不采用if。
//    if ((pid = waitpid(0, &status, WNOHANG)) > 0) {
    while ((pid = waitpid(0, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status))
            printf("------------child %d exit with %d\n", pid, WEXITSTATUS(status));
        else if (WIFSIGNALED(status))
            printf("child %d killed by the %dth signal\n", pid, WTERMSIG(status));
    }
}
 
int main(void)
{
    pid_t pid;
    int i;
    //阻塞SIGCHLD
    for (i = 0; i < 10; i++) {
        if ((pid = fork()) == 0)
            break;
        else if (pid < 0)
            sys_err("fork");
    }
 
    if (pid == 0) {     //10个子进程
        int n = 1;
        while (n--) {
            printf("child ID %d\n", getpid());
            sleep(1);
        }
        return i+1;      //子进程结束状态依次为1、2、••••••、10
    } else if (pid > 0) {   
        struct sigaction act;
 
        act.sa_handler = do_sig_child;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        sigaction(SIGCHLD, &act, NULL);
        //解除对SIGCHLD的阻塞
 
//		signal(SIGCHLD, SIG_IGN);
        while (1) {
            printf("Parent ID %d\n", getpid());
            sleep(1);
        }
    }
 
    return 0;
}
