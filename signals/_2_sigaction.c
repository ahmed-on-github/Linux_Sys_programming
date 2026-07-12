#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#define RESTART_LOGIC  0

void myhandler(int signum) {

        printf("Signal %s caught, ignoring SIGFPE\n", strsignal(signum));
        sleep(10);/* To test that SIGFPE is ignored here */

        printf("signal handler returning ...\n");
}

int main() {
        struct sigaction action;

        action.sa_handler = myhandler;
        sigemptyset(&action.sa_mask);
        sigaddset(&action.sa_mask, SIGFPE);     /* block  */
        action.sa_flags = SA_RESTART;   // Or 0;

        int fd[2], n;
        char buf[20];
        pipe(fd);

        sigaction(SIGINT, &action, NULL /* old sigaction ignored */ );
        sigaction(SIGHUP, &action, NULL /* old sigaction ignored */ );

        int i = 0;
        sigset_t sigmask;
        /* block and unblock sighup */
        sigemptyset(&sigmask);
        sigaddset(&sigmask, SIGHUP);
        while (1) {
                /* Testing signal blocking */
                if (RESTART_LOGIC == 0) {
                        i++;
                        switch (i % 5) {
                        case 2:
                                /* block sighup */
                                sigprocmask(SIG_BLOCK, &sigmask, NULL /*  */ );
                                break;
                        case 4:
                                /* unblock sighup */
                                sigprocmask(SIG_UNBLOCK, &sigmask, NULL /*  */ );
                                break;
                        default:                        /* nothing */
                                break;
                        }
                        printf("loop iteration%5: %d\n", i % 5);
                        sleep(3);
                }
                else {  /* Testing system calls restart upon signal interrution */
                        /* blocking system call as reading from the read end of pipe will block (not connected to any file/stream) */
                        n = read(fd[0], buf, sizeof(buf));
                        /* Won't be reached if action.sa_flags has SA_RESTART bit set */
                        perror("read: ");
                        printf("read %d bytes, errno = %d\n", n, errno);
                }
        }

        return 0;
}
